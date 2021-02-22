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

#include "syncer_process.h"
#include "proc/processing.hpp"

#include "device/device.h"
#include "core/frame_data.h"
#include "streaming/stream_profile.h"
#include "se_common.h"
#include "easylogging++.h"

#include <utility>

namespace libsmartereye2 {

Matcher::Matcher(std::vector<StreamId> streams_id)
    : streams_id_(std::move(streams_id)) {

}

Matcher::~Matcher() {
  callback_inflight_.stopAllocating();
  auto callbacks_inflight_size = callback_inflight_.size();
  if (callbacks_inflight_size > 0) {
    LOG(WARNING) << callbacks_inflight_size
                 << " callbacks are still running on some other threads. Waiting until all callbacks return...";
  }
  callback_inflight_.waitUntilEmpty();
}

void Matcher::sync(FrameHolder f, SyncronizationEnvironment env) {
  auto cb = beginCallback();
  callback_(std::move(f), env);
}

void Matcher::setCallback(SyncCallback f) {
  callback_ = f;
}

const std::vector<StreamId> &Matcher::getStreams() const {
  return streams_id_;
}

const std::vector<FrameId> &Matcher::getStreamsTypes() const {
  return streams_type_;
}

std::string Matcher::getName() const {
  return std::string();
}

CallbackInvocationHolder Matcher::beginCallback() {
  return {callback_inflight_.allocate(), &callback_inflight_};
}

IdentityMatcher::IdentityMatcher(StreamId stream_id, FrameId frame_id)
    : Matcher({stream_id}) {
  streams_type_ = {frame_id};
  name_ = "I " + std::to_string(static_cast<int>(frame_id));
}

void IdentityMatcher::dispatch(FrameHolder f, SyncronizationEnvironment env) {
  std::stringstream s;
  s << name_ << "--> " << static_cast<int>(f->getStreamProfile()->frameId()) << " " << f->getFrameIndex() << ", " << std::fixed
    << f->getFrameTimestamp() << "\n";
  LOG(DEBUG) << s.str();

  sync(std::move(f), env);
}

CompositeMatcher::CompositeMatcher(const std::vector<std::shared_ptr<Matcher>> &matchers, std::string name) {
  for (auto &&matcher : matchers) {
    for (auto &&stream : matcher->getStreams()) {
      matcher->setCallback([&](FrameHolder f, SyncronizationEnvironment env) {
        sync(std::move(f), env);
      });
      matchers_[stream] = matcher;
      streams_id_.push_back(stream);
    }
    for (auto &&stream : matcher->getStreamsTypes()) {
      streams_type_.push_back(stream);
    }
  }

  name_ = createCompositeName(matchers, name);
}

std::string CompositeMatcher::frameToString(FrameHolder &frame_holder) {
  std::ostringstream s;
  auto composite = dynamic_cast<CompositeFrameData *>(frame_holder.frame);
  if (composite) {
    for (int i = 0; i < composite->getFrameCount(); i++) {
      auto frame = composite->getFrame(i);
      s << static_cast<int>(frame->getStreamProfile()->frameId()) << " " << frame->getFrameIndex() << " " << std::fixed
        << frame->getFrameTimestamp() << " ";
    }
  } else {
    s << static_cast<int>(frame_holder->getStreamProfile()->frameId());
    s << " " << frame_holder->getStreamProfile()->uniqueId();
    s << " " << frame_holder->getFrameIndex();
    s << " " << std::fixed << (double) frame_holder->getFrameTimestamp();
    s << " ";
  }
  return s.str();
}

std::string CompositeMatcher::createCompositeName(const std::vector<std::shared_ptr<Matcher>> &matchers,
                                                  const std::string &name) {
  std::stringstream s;
  s << "(" << name;

  for (auto &&matcher : matchers) {
    s << matcher->getName() << " ";
  }
  s << ")";
  return s.str();
}

void CompositeMatcher::dispatch(FrameHolder frame_holder, SyncronizationEnvironment env) {
  std::stringstream s;
  s << "DISPATCH " << name_ << "--> " << frameToString(frame_holder) << "\n";
  LOG(DEBUG) << s.str();

  cleanInactiveStreams(frame_holder);
  auto matcher = findMatcher(frame_holder);
  updateLastArrived(frame_holder, matcher.get());
  matcher->dispatch(std::move(frame_holder), env);
}

void CompositeMatcher::sync(FrameHolder f, SyncronizationEnvironment env) {
  Matcher::sync(f.clone(), env);
}

std::string CompositeMatcher::framesToString(std::vector<Matcher *> matchers) {
  return std::string();
}

std::shared_ptr<Matcher> CompositeMatcher::findMatcher(const FrameHolder &frame_holder) {
  std::shared_ptr<Matcher> matcher;
  auto stream_id = frame_holder.frame->getStreamProfile()->uniqueId();
  auto stream_type = frame_holder.frame->getStreamProfile()->frameId();

  // TODO: Potential deadlock if getSensor() gets a hold of the last reference of that sensor
  auto sensor = frame_holder.frame->getSensor().get();
  auto dev_exist = false;

  if (sensor) {
    const DeviceInterface *dev = nullptr;
    try {
      dev = sensor->getDevice().shared_from_this().get();
    }
    catch (const std::bad_weak_ptr &) {
      LOG(WARNING) << "Device destroyed";
    }
    if (dev) {
      dev_exist = true;
      matcher = matchers_[stream_id];
      if (!matcher) {
        std::ostringstream ss;
        for (auto const &it : matchers_)
          ss << ' ' << it.first;
        LOG(DEBUG) << "stream id " << stream_id << " was not found; trying to create, existing streams=" << ss.str();
        matcher = dev->createMatcher(frame_holder);

        matcher->setCallback(
            [&](FrameHolder f, SyncronizationEnvironment env) {
              sync(std::move(f), env);
            });

        for (auto stream : matcher->getStreams()) {
          if (matchers_[stream]) {
            frames_queue_.erase(matchers_[stream].get());
          }
          matchers_[stream] = matcher;
          streams_id_.push_back(stream);
        }
        for (auto stream : matcher->getStreamsTypes()) {
          streams_type_.push_back(stream);
        }

        if (std::find(streams_type_.begin(), streams_type_.end(), stream_type) == streams_type_.end()) {
          LOG(ERROR) << "Stream matcher not found! stream=" << static_cast<int>(stream_type);
        }
      } else if (!matcher->getActive()) {
        matcher->setActive(true);
        frames_queue_[matcher.get()].start();
      }
    }
  } else {
    LOG(DEBUG) << "sensor does not exist";
  }

  if (!dev_exist) {
    matcher = matchers_[stream_id];
    // We don't know what device this frame came from, so just store it under device NULL with ID matcher
    if (!matcher) {
      if (matchers_[stream_id]) {
        frames_queue_.erase(matchers_[stream_id].get());
      }
      matchers_[stream_id] = std::make_shared<IdentityMatcher>(stream_id, stream_type);
      streams_id_.push_back(stream_id);
      streams_type_.push_back(stream_type);
      matcher = matchers_[stream_id];

      matcher->setCallback([&](FrameHolder f, SyncronizationEnvironment env) {
        sync(std::move(f), env);
      });
    }
  }
  return matcher;
}

TimestampCompositeMatcher::TimestampCompositeMatcher(std::vector<std::shared_ptr<Matcher>> matchers)
    : CompositeMatcher(matchers, "TS: ") {

}

bool TimestampCompositeMatcher::equal(const FrameHolder &a, const FrameHolder &b) {
  auto a_fps = a.frame->getStreamProfile()->fps();
  auto b_fps = b.frame->getStreamProfile()->fps();
  int min_fps = (a_fps < b_fps ? a_fps : b_fps);

  auto timestamp_pair = extractTimestamps(a, b);
  return equivalent(timestamp_pair.first, timestamp_pair.second, min_fps);
}

bool TimestampCompositeMatcher::lessThan(const FrameHolder &a, const FrameHolder &b) {
  if (!a || !b) {
    return false;
  }

  auto timestamp_pair = extractTimestamps(a, b);
  return timestamp_pair.first < timestamp_pair.second;
}

bool TimestampCompositeMatcher::skipMissingStream(std::vector<Matcher *> synced, Matcher *missing) {
  if (!missing->getActive()) return true;

  FrameHolder *synced_frame;

  frames_queue_[synced[0]].peek(&synced_frame);

  auto next_expected = _next_expected[missing];

  auto it = _next_expected_domain.find(missing);
  if (it != _next_expected_domain.end()) {
    if (it->second != (*synced_frame)->getFrameTimestampDomain()) {
      return false;
    }
  }
  auto gap = 1000.f / (float) fps(*synced_frame);
  //next expected of the missing stream didn't updated yet
  if ((*synced_frame)->getFrameTimestamp() > next_expected
      && abs((*synced_frame)->getFrameTimestamp() - next_expected) < gap * 10) {
    LOG(DEBUG) << "next expected of the missing stream didn't updated yet";
    return false;
  }

  return !equivalent((*synced_frame)->getFrameTimestamp(), next_expected, fps(*synced_frame));
}

void TimestampCompositeMatcher::cleanInactiveStreams(const FrameHolder &frame_holder) {
  if (frame_holder.isBlocking()) return;

  std::vector<StreamId> deadmatcher_s;
  double now = Environment::instance().getTimeService()->getTime();
  for (const auto &matcher: matchers_) {
    auto threshold = fps_[matcher.second.get()] ? (1000 / fps_[matcher.second.get()]) * 5
                                                : 500; //if frame of a specific stream didn't arrive for time equivalence to 5 frames duration
    //this stream will be marked as "not active" in order to not stack the other streams
    if (last_arrived_[matcher.second.get()] && (now - last_arrived_[matcher.second.get()]) > threshold) {
      std::stringstream s;
      s << "clean inactive stream in " << name_;
      for (auto stream : matcher.second->getStreamsTypes()) {
        s << static_cast<int>(stream) << " ";
      }
      LOG(DEBUG) << s.str();

      deadmatcher_s.push_back(matcher.first);
      matcher.second->setActive(false);
    }
  }

  for (auto id: deadmatcher_s) {
    frames_queue_[matchers_[id].get()].clear();
    frames_queue_.erase(matchers_[id].get());
  }
}

void TimestampCompositeMatcher::updateLastArrived(FrameHolder &frame_holder, Matcher *matcher) {
  fps_[matcher] = frame_holder->getStreamProfile()->fps();
  last_arrived_[matcher] = Environment::instance().getTimeService()->getTime();
}

void TimestampCompositeMatcher::updateNextExpected(const FrameHolder &frame_holder) {
  auto matcher = findMatcher(frame_holder);
  _next_expected[matcher.get()] = frame_holder.frame->getFrameIndex() + 1.;
}

uint32_t TimestampCompositeMatcher::fps(const FrameHolder &frame_holder) {
  uint32_t fps = 0;
  LOG(DEBUG) << "fps " << fps << " " << frameToString(const_cast<FrameHolder &>(frame_holder));
  return fps ? fps : frame_holder.frame->getStreamProfile()->fps();
}

std::pair<double, double> TimestampCompositeMatcher::extractTimestamps(const FrameHolder &a, const FrameHolder &b) {
  return {a->getFrameTimestamp(), b->getFrameTimestamp()};
}

bool TimestampCompositeMatcher::equivalent(double a, double b, int fps) {
  auto gap = 1000.f / static_cast<float>(fps);
  return std::abs(a - b) < (gap / 2.f);
}

SyncerProcessUnit::SyncerProcessUnit(std::initializer_list<bool> enable_opts)
    : ProcessingBlock("syncer"),
      matcher_(new TimestampCompositeMatcher({})),
      enable_opts_(enable_opts.begin(), enable_opts.end()) {

  matcher_->setCallback([this](FrameHolder f, SyncronizationEnvironment env) {
    std::stringstream ss;
    ss << "SYNCED: ";
    auto composite = dynamic_cast<CompositeFrameData *>(f.frame);
    for (int i = 0; i < composite->getFrameCount(); i++) {
      auto matched = composite->getFrame(i);
      ss << static_cast<int>(matched->getStreamProfile()->frameId()) << " " << matched->getFrameIndex() << ", " << std::fixed
         << matched->getFrameTimestamp() << " ";
    }

    LOG(DEBUG) << (ss.str());
    env.matches.enqueue(std::move(f));
  });

  auto func = [&](FrameHolder frame, SyntheticSourceInterface *source) {
    // if the syncer is disabled passthrough the frame
    bool enabled = false;
    size_t n_opts = 0;
    for (auto opt : enable_opts_) {
      if (opt) {
        enabled = true;
        break;
      }
    }
    if (!enabled) {
      this->getSource().frameReady(std::move(frame));
      return;
    }

    ConsumerFrameQueue<FrameHolder> matches;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      matcher_->dispatch(std::move(frame), {source, matches});
    }

    FrameHolder holder;
    while (matches.tryDequeue(&holder))
      this->getSource().frameReady(std::move(holder));

  };

//  setProcessingCallback(nullptr);
}

}  // namespace se2

namespace se2 {

}  // namespace se2
