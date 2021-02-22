// Copyright 2020 Smarter Eye Co.,Ltd. All Rights Reserved.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pipeline.h"

#include <memory>
#include <utility>
#include <chrono>
#include <algorithm>
#include <cassert>

#include "pipeline_config.h"
#include "pipeline_profile.h"
#include "streaming/stream_profile.h"
#include "core/frame.h"
#include "easylogging++.h"

#include "pipeline/pipeline.hpp"
#include "pipeline/pipeline_profile.hpp"
#include "pipeline/pipeline_config.hpp"
#include "core/frame.hpp"
#include "core/frame_set.hpp"

namespace libsmartereye2 {

PipelinePrivate::PipelinePrivate(const std::shared_ptr<ContextPrivate> &context)
    : context_(context),
      device_hub_(context),
      synced_streams_({FrameId::CalibLeftCamera, FrameId::Disparity}),
      is_stopping_(false) {

}

PipelinePrivate::~PipelinePrivate() {
  try {
    unsafeStop();
  } catch (...) {}
}

std::shared_ptr<PipelineProfilePrivate> PipelinePrivate::start(std::shared_ptr<PipelineConfigPrivate> conf,
                                                               FrameCallbackPtr callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (active_profile_) {
    LOG(WARNING) << "start() cannot be called before stop()";
    return nullptr;
  }
  streams_callback_ = std::move(callback);
  return unsafeStart(std::move(conf)) ? unsafeGetActiveProfile() : nullptr;
}

void PipelinePrivate::stop(bool force) {
  if (force) {
    is_stopping_ = true;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  if (!active_profile_) {
    if (!force) {
      LOG(WARNING) << "stop() cannot be called before start()";
    }
    return;
  }
  unsafeStop();
}

bool PipelinePrivate::isConnected() const {
  return active_profile_ != nullptr && device_hub_.isConnected(*active_profile_->getDevice());
}

int64_t PipelinePrivate::registerInternalDeviceCallback(DevicesChangedCallbackPtr callback) {
  return context_->registerDevicesChangedCallback(std::move(callback));
}

void PipelinePrivate::unregisterDevicesChangedCallback(int64_t cb_id) {
  context_->unregisterDevicesChangedCallback(cb_id);
}

std::shared_ptr<PipelineProfilePrivate> PipelinePrivate::getActiveProfile() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return unsafeGetActiveProfile();
}

FrameHolder PipelinePrivate::waitForFrames(uint32_t timeout_ms) {
  std::lock_guard<std::mutex> lock(mutex_);
  FrameHolder frame_holder;
  if (!active_profile_) {
    LOG(WARNING) << "wait_for_frames cannot be called before start()";
    return frame_holder;
  }
  if (streams_callback_) {
    LOG(WARNING) << "wait_for_frames cannot be called if a callback was provided";
    return frame_holder;
  }

  if (aggregator_->dequeue(&frame_holder, timeout_ms)) {
    return frame_holder;
  }
  if (!isConnected()) {
    // reconnect
    try {
      auto prev_conf = prev_conf_;
      unsafeStop();
      if (!unsafeStart(prev_conf)) {
        return frame_holder;
      }
      // if timeout_ms < 3000, stream may not resumed
      timeout_ms = timeout_ms < 3000 ? 3000 : timeout_ms;
      if (aggregator_->dequeue(&frame_holder, timeout_ms)) {
        return frame_holder;
      }
    } catch (const std::exception &e) {
      throw std::runtime_error(toString() << "Device disconnected. Failed to recconect: " << e.what() << timeout_ms);
    }
    throw std::runtime_error(toString() << "Frame didn't arrive within " << timeout_ms);
  }
  return frame_holder;
}

bool PipelinePrivate::pollForFrames(FrameHolder *frame_holder) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!active_profile_) {
    LOG(WARNING) << ("poll_for_frames cannot be called before start()");
    return false;
  }
  if (streams_callback_) {
    LOG(WARNING) << "poll_for_frames cannot be called if a callback was provided";
    return false;
  }

  return aggregator_->tryDequeue(frame_holder);
}

bool PipelinePrivate::tryWaitForFrames(FrameHolder *frame_holder, uint32_t timeout_ms) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!active_profile_) {
    LOG(WARNING) << "try_wait_for_frames cannot be called before start()";
    return false;
  }
  if (streams_callback_) {
    LOG(WARNING) << "try_wait_for_frames cannot be called if a callback was provided";
    return false;
  }

  if (aggregator_->dequeue(frame_holder, timeout_ms)) {
    return true;
  }

  //hub returns true even if device already reconnected
  if (!isConnected()) {
    try {
      auto prev_conf = prev_conf_;
      unsafeStop();
      return unsafeStart(prev_conf) ? aggregator_->dequeue(frame_holder, timeout_ms) : false;
    }
    catch (const std::exception &e) {
      LOG(INFO) << e.what();
      return false;
    }
  }
  return false;
}

std::shared_ptr<DeviceInterface> PipelinePrivate::waitForDevice(const std::chrono::milliseconds &timeout,
                                                                const std::string &serial) {
  return device_hub_.waitForDevice(timeout, false, serial);
}

FrameCallbackPtr PipelinePrivate::getCallback() const {
  auto on_frame_func = [this](FrameHolder fref) {
    aggregator_->invoke(std::move(fref));
  };

  FrameCallbackPtr cb(new InternalFrameCallback<decltype(on_frame_func)>(on_frame_func));
  return cb;
}

std::vector<int> PipelinePrivate::onStart(const std::shared_ptr<PipelineProfilePrivate> &profile) {
  std::vector<int> streams_to_aggregate_ids;

  for (auto &&s : profile->getActiveStreams()) {
    streams_to_aggregate_ids.push_back(s->uniqueId());
  }

//  aggregator_ = std::make_unique<FrameAggregator>(streams_to_aggregate_ids);  // c++ 14
  aggregator_ = std::unique_ptr<FrameAggregator>(new FrameAggregator(streams_to_aggregate_ids));
  aggregator_->start();

  if (streams_callback_) {
    aggregator_->setOutputCallback(streams_callback_);
  }

  return streams_to_aggregate_ids;
}

bool PipelinePrivate::unsafeStart(std::shared_ptr<PipelineConfigPrivate> conf) {
  std::shared_ptr<PipelineProfilePrivate> profile = nullptr;
  //first try to get the previously resolved profile (if exists)
  auto cached_profile = conf->getCachedResolvedProfile();
  if (cached_profile) {
    profile = cached_profile;
  } else {
    const int kNumTimesToRetry = 3000;
    for (int i = 1; i <= kNumTimesToRetry; i++) {
      try {
        if (is_stopping_) {
          LOG(WARNING) << "force stop pipeline while it was starting...";
          is_stopping_ = false;
          return false;
        }
        profile = conf->resolve(shared_from_this(), std::chrono::seconds(5));
        break;
      }
      catch (...) {
        if (i == kNumTimesToRetry)
          throw;
      }
    }
  }

  assert(profile);

  auto multi_stream = profile->multi_stream_;
  assert(!multi_stream->getAllProfiles().empty());
  multi_stream->open();
  auto synced_streams_ids = onStart(profile);

  FrameCallbackPtr callbacks = getCallback();
  multi_stream->start(callbacks);

  context_->start();
  active_profile_ = profile;
  prev_conf_ = std::make_shared<PipelineConfigPrivate>(*conf);
  return true;
}

void PipelinePrivate::unsafeStop() {
  if (active_profile_) {
    try {
      aggregator_->stop();
      active_profile_->multi_stream_->stop();
      active_profile_->multi_stream_->close();
    } catch (...) {
    } // Stop will throw if device was disconnected. TODO - refactoring anticipated
  }
  context_->stop();
  active_profile_.reset();
  prev_conf_.reset();
  streams_callback_.reset();
  is_stopping_ = false;
}

std::shared_ptr<PipelineProfilePrivate> PipelinePrivate::unsafeGetActiveProfile() const {
  if (!active_profile_) {
    LOG(WARNING) << "getActiveProfile() can only be called between a start() and a following stop()";
  }
  return active_profile_;
}

}  // namespace libsmartereye2

namespace se2 {

using namespace libsmartereye2;

Pipeline::Pipeline(const Context &context)
    : pipeline_(nullptr) {
  auto context_private = context.context_->context;
  auto pipeline_private = std::make_shared<libsmartereye2::PipelinePrivate>(context_private);
  pipeline_ = std::make_shared<SePipeline>(SePipeline{pipeline_private});
}

PipelineProfile Pipeline::start() {
  auto config = std::make_shared<PipelineConfigPrivate>();
  std::shared_ptr<SePipelineProfile> profile(new SePipelineProfile{pipeline_->pipeline->start(config)});
  return PipelineProfile(profile);
}

PipelineProfile Pipeline::start(const PipelineConfig &config) {
  auto config_private = config.get()->config;
  std::shared_ptr<SePipelineProfile> profile(new SePipelineProfile{pipeline_->pipeline->start(config_private)});
  return PipelineProfile(profile);
}

PipelineProfile Pipeline::start(FrameCallbackPtr callback) {
  auto config = std::make_shared<PipelineConfigPrivate>();
  std::shared_ptr<SePipelineProfile> profile(new SePipelineProfile{pipeline_->pipeline->start(config, std::move(callback))});
  return PipelineProfile(profile);
}

PipelineProfile Pipeline::start(const PipelineConfig &config, FrameCallbackPtr callback) {
  auto config_private = config.get()->config;
  std::shared_ptr<SePipelineProfile>
      profile(new SePipelineProfile{pipeline_->pipeline->start(config_private, std::move(callback))});
  return PipelineProfile(profile);
}

void Pipeline::stop(bool force) {
  pipeline_->pipeline->stop(force);
}

bool Pipeline::isConnected() const {
  return pipeline_->pipeline->isConnected();
}

FrameSet Pipeline::waitForFrames(uint32_t timeout_ms) const {
  auto frame_holder = pipeline_->pipeline->waitForFrames(timeout_ms);
  auto frame = frame_holder.frame;
  frame_holder.frame = nullptr;
  return FrameSet(Frame(frame));
}

bool Pipeline::pollForFrames(FrameSet *frame_set) const {
  if (!frame_set) {
    throw std::invalid_argument("null frameset");
    return false;
  }

  FrameHolder fh;
  if (pipeline_->pipeline->pollForFrames(&fh)) {
    FrameInterface *result = nullptr;
    std::swap(result, fh.frame);
    *frame_set = FrameSet(Frame(result));
    return true;
  }
  return false;
}

bool Pipeline::tryWaitForFrames(FrameSet *frame_set, uint32_t timeout_ms) const {
  if (!frame_set) {
    throw std::invalid_argument("null frameset");
    return false;
  }

  FrameHolder fh;
  if (pipeline_->pipeline->tryWaitForFrames(&fh, timeout_ms)) {
    FrameInterface *result = nullptr;
    std::swap(result, fh.frame);
    *frame_set = FrameSet(Frame(result));
    return true;
  }
  return false;
}

PipelineProfile Pipeline::getActiveProfile() {
  auto profile_private = pipeline_->pipeline->getActiveProfile();
  std::shared_ptr<SePipelineProfile> se_profile(new SePipelineProfile{profile_private});
  return PipelineProfile(se_profile);
}

int64_t Pipeline::registerInternalDeviceCallback(DevicesChangedCallbackPtr callback) {
  return pipeline_->pipeline->registerInternalDeviceCallback(std::move(callback));
}

void Pipeline::unregisterDevicesChangedCallback(int64_t cb_id) {
  pipeline_->pipeline->unregisterDevicesChangedCallback(cb_id);
}

}  // namespace se2
