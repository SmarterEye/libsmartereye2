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

#include "frame_source.h"
#include "frame_data.h"
#include "easylogging++.h"

#include <utility>

namespace libsmartereye2 {

FrameSource::FrameSource(uint32_t max_publish_list_size)
    : frame_callback_(nullptr),
      max_publish_list_size_(max_publish_list_size),
      time_service_(Environment::instance().getTimeService()) {

}

void FrameSource::init(std::shared_ptr<MetadataParserMap> metadata_parsers) {
  std::lock_guard<std::mutex> lock(callback_mutex_);

  // TODO: should move to MetadataParserMap
  std::vector<SeExtension> supported{
      SeExtension::EXTENSION_VIDEO_FRAME,
      SeExtension::EXTENSION_COMPOSITE_FRAME,
      SeExtension::EXTENSION_POINTS,
      SeExtension::EXTENSION_DEPTH_FRAME,
      SeExtension::EXTENSION_DISPARITY_FRAME,
      SeExtension::EXTENSION_MOTION_FRAME,
      SeExtension::EXTENSION_POSE_FRAME,
      SeExtension::EXTENSION_JOURNEY_FRAME,
      SeExtension::EXTENSION_OBSTACLE_FRAME,
      SeExtension::EXTENSION_LANE_FRAME,
      SeExtension::EXTENSION_FREESPACE_FRAME,
      SeExtension::EXTENSION_SMALL_OBS_FRAME
  };

  for (auto type : supported) {
    archives_[type] = makeArchive(type, &max_publish_list_size_, time_service_, metadata_parsers);
  }

  metadata_parsers_ = metadata_parsers;
}

CallbackInvocationHolder FrameSource::begin_callback() {
  return archives_[SeExtension::EXTENSION_VIDEO_FRAME]->begin_callback();
}

void FrameSource::reset() {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  frame_callback_.reset();
  for (auto &&kvp : archives_) {
    kvp.second.reset();
  }
  metadata_parsers_.reset();
}

std::shared_ptr<Option> FrameSource::get_published_size_option() {
  return std::shared_ptr<Option>();
}

FrameInterface *FrameSource::alloc_frame(SeExtension type,
                                         size_t size,
                                         const FrameExtension& additional_data,
                                         bool requires_memory) const {
  auto it = archives_.find(type);
  if (it == archives_.end()) {
    throw std::runtime_error("Requested frame type is not supported!");
  }
  return it->second->alloc_and_track(size, additional_data, requires_memory);
}

void FrameSource::set_callback(FrameCallbackPtr callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  frame_callback_ = std::move(callback);
}

FrameCallbackPtr FrameSource::get_callback() const {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  return frame_callback_;
}

void FrameSource::invoke_callback(FrameHolder frame) const {
  if (frame) {
    auto callback = frame.frame->getOwner()->begin_callback();
    try {
      if (frame_callback_) {
        FrameInterface *ref = nullptr;
        std::swap(frame.frame, ref);
        frame_callback_->onFrame(ref);
      }
    }
    catch (const std::exception &e) {
      LOG(ERROR) << "Exception was thrown during user callback: " + std::string(e.what());
    }
    catch (...) {
      LOG(ERROR) << "Exception was thrown during user callback!";
    }
  }
}

void FrameSource::flush() const {
  for (auto &&kvp : archives_) {
    if (kvp.second)
      kvp.second->flush();
  }
}

void FrameSource::set_sensor(const std::shared_ptr<SensorInterface> &s) {
  for (auto &&a : archives_) {
    a.second->set_sensor(s);
  }
}

}  // namespace libsmartereye2
