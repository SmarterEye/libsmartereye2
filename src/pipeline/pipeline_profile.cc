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

#include "pipeline_profile.h"
#include "device/device.h"
#include "device/device_info.h"
#include "streaming/stream_profile.h"

#include "pipeline/pipeline_profile.hpp"
#include "device/device.hpp"
#include "streaming/stream_profile.hpp"

#include <utility>

namespace libsmartereye2 {

PipelineProfilePrivate::PipelineProfilePrivate(const std::shared_ptr<DeviceInterface> dev, std::string file)
    : multi_stream_(new MultiStream(dev)),
      device_(dev),
      to_file_(std::move(file)) {

}

std::shared_ptr<DeviceInterface> PipelineProfilePrivate::getDevice() {
  // TODO: handle case where device has disconnected and reconnected
  // TODO: remember to recreate the device as record device in case of to_file.empty() == false
  if (!device_) {
    throw std::runtime_error("Device is unavailable");
  }
  return device_;
}

StreamProfiles PipelineProfilePrivate::getActiveStreams() const {
  StreamProfiles stream_profiles;

  for (auto &&sensor : multi_stream_->getSensors()) {
    auto active_stream = sensor->getActiveStreams();
    stream_profiles.insert(stream_profiles.end(), active_stream.begin(), active_stream.end());
  }

  return stream_profiles;
}

MultiStream::MultiStream(const std::shared_ptr<DeviceInterface> &dev) {
  for (size_t i = 0; i < dev->getSensorCount(); i++) {
    sensors_.push_back(&dev->getSensor(i));
  }

  for (auto &sensor : sensors_) {
    auto profiles = sensor->getStreamProfiles(ProfileTag::PROFILE_TAG_ANY);
    sensor_to_profiles_[sensor] = profiles;
    all_profiles_.insert(all_profiles_.end(), profiles.begin(), profiles.end());
  }
}

void MultiStream::open() {
  for (auto &&kvp : sensor_to_profiles_) {
    kvp.first->open(kvp.second);
  }
}

void MultiStream::close() {
  for (auto &&sensor : sensors_) {
    sensor->close();
  }
}

void MultiStream::stop() {
  for (auto &&sensor : sensors_) {
    sensor->stop();
  }
}

bool MultiStream::isStreaming() const {
  for (const auto &sensor : sensors_) {
    if (sensor->isStreaming()) {
      return true;
    }
  }
  return false;
}

}  // namespace libsmartereye2

namespace se2 {

std::vector<StreamProfile> se2::PipelineProfile::getStreams() const {
  std::vector<StreamProfile> results;
  auto list = pipeline_profile_->profile->getActiveStreams();
  auto size = list.size();

  for (size_t i = 0; i < size; i++) {
    StreamProfile sp(new SeStreamProfile{list.at(i).get()});
    results.push_back(sp);
  }
  return results;
}

StreamProfile PipelineProfile::getStream(FrameId frame_id, int index) const {
  auto stream_list = getStreams();
  for (auto &&stream : stream_list) {
    if (stream.frameId() == frame_id
        && (index == -1 || stream.index() == index))
      return stream;
  }
  throw std::runtime_error("Profile does not contain the requested stream");
}

Device PipelineProfile::getDevice() const {
  auto device_interface = pipeline_profile_->profile->getDevice();
  auto device_info = std::make_shared<libsmartereye2::ReadonlyDeviceInfo>(device_interface);
  std::shared_ptr<SeDevice> dev(new SeDevice{device_interface->getContext(), device_info, device_interface});
  return Device(dev);
}

}  // namespace se2
