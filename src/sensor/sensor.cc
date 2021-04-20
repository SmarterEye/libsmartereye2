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

#include "sensor.h"
#include "streaming/stream_profile.h"
#include "device/device.h"

#include "sensor/sensor.hpp"
#include "streaming/stream_profile.hpp"

namespace se2 {

Sensor::Sensor(const std::shared_ptr<SeSensor> &dev)
    : Options((SeOptions *) dev.get()), sensor_(dev) {
}

Sensor::operator std::shared_ptr<SeSensor>() {
  return sensor_;
}

Sensor &Sensor::operator=(const std::shared_ptr<SeSensor> &other) {
  Options::operator=(other);
  sensor_.reset();
  sensor_ = other;
  return *this;
}

Sensor &Sensor::operator=(const Sensor &other) {
  *this = nullptr;
  Options::operator=(other.sensor_);
  sensor_ = other.sensor_;
  return *this;
}

bool Sensor::supports(CameraInfo info) const {
  return sensor_->sensor->supportsInfo(info);
}

std::string Sensor::getInfo(CameraInfo info) const {
  return sensor_->sensor->getInfo(info);
}

void Sensor::open(const StreamProfile &profile) const {
  std::vector<std::shared_ptr<libsmartereye2::StreamProfileInterface>> request;
  auto profile_ptr = profile.profile_->shared_from_this();
  request.push_back(std::dynamic_pointer_cast<libsmartereye2::StreamProfileInterface>(profile_ptr));
  sensor_->sensor->open(request);
}

void Sensor::open(const std::vector<StreamProfile> &profiles) const {
  std::vector<std::shared_ptr<libsmartereye2::StreamProfileInterface>> requests;
  for (const auto &current_profile : profiles) {
    auto profile_ptr = current_profile.profile_->shared_from_this();
    requests.push_back(std::dynamic_pointer_cast<libsmartereye2::StreamProfileInterface>(profile_ptr));
  }
  sensor_->sensor->open(requests);
}

void Sensor::close() const {
  sensor_->sensor->close();
}

std::vector<StreamProfile> Sensor::getStreamProfiles() const {
  std::vector<StreamProfile> results{};
  auto stream_profiles = sensor_->sensor->getStreamProfiles(libsmartereye2::ProfileTag::PROFILE_TAG_ANY);

  for (const auto &profile : stream_profiles) {
    results.emplace_back(new SeStreamProfile{profile.get()});
  }
  return results;
}

std::vector<StreamProfile> Sensor::getActiveStreams() const {
  std::vector<StreamProfile> results{};
  auto stream_profiles = sensor_->sensor->getActiveStreams();

  for (const auto &profile : stream_profiles) {
    results.emplace_back(new SeStreamProfile{profile.get()});
  }
  return results;
}

}  // namespace se2

namespace libsmartereye2 {

libsmartereye2::SensorBase::SensorBase(const std::string &name, libsmartereye2::DevicePrivate *device)
    : is_streaming_(false),
      is_opened_(false),
      metadata_parsers_(std::make_shared<MetadataParserMap>()),
      frame_source_(new FrameSource),
      device_owner_(device) {

}

StreamProfiles SensorBase::getStreamProfiles(int tag) const {
  if (tag == ProfileTag::PROFILE_TAG_ANY)
    return profiles_;

  StreamProfiles results;
  for (const auto &p : profiles_) {
    if (p->tag() & tag)
      results.push_back(p);
  }

  return results;
}

StreamProfiles SensorBase::getActiveStreams() const {
  std::lock_guard<std::mutex> lock(active_profiles_mutex_);
  return active_profiles_;
}

FrameCallbackPtr SensorBase::getFramesCallback() const {
  return frame_source_->get_callback();
}

void SensorBase::setFramesCallback(FrameCallbackPtr cb) {
  frame_source_->set_callback(cb);
}

bool SensorBase::isStreaming() const {
  return is_streaming_;
}

DeviceInterface &SensorBase::getDevice() {
  return *device_owner_;
}

void SensorBase::setActiveStream(const StreamProfiles &requests) {
  std::lock_guard<std::mutex> lock(active_profiles_mutex_);
  active_profiles_ = requests;
}

}  // namespace libsmartereye2

