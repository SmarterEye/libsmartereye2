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

#include "playback_device.h"
#include "playback_sensor.h"

namespace libsmartereye2 {

//PlaybackDevice::PlaybackDevice(std::shared_ptr<ContextPrivate> context,
//                               std::shared_ptr<device_serializer::reader> serializer) {

//}

PlaybackDevice::~PlaybackDevice() {

}

SensorInterface &PlaybackDevice::getSensor(size_t i) {
  return *m_sensors.at(i);
}

const SensorInterface &PlaybackDevice::getSensor(size_t i) const {
  return *m_sensors.at(i);
}

size_t PlaybackDevice::getSensorCount() const {
  return 0;
}

void PlaybackDevice::hardwareReset() {

}

std::shared_ptr<ContextPrivate> PlaybackDevice::getContext() const {
  return std::shared_ptr<ContextPrivate>();
}

std::pair<int32_t, Extrinsics> PlaybackDevice::getExtrinsics(const StreamInterface &stream) const {
  return std::pair<int32_t, Extrinsics>();
}

platform::BackendDeviceGroup PlaybackDevice::getDeviceData() const {
  return platform::BackendDeviceGroup();
}

bool PlaybackDevice::isValid() const {
  return false;
}

std::shared_ptr<Matcher> PlaybackDevice::createMatcher(const FrameHolder &frame) const {
  return std::shared_ptr<Matcher>();
}

std::vector<TaggedProfile> PlaybackDevice::getProfilesTags() const {
  return std::vector<TaggedProfile>();
}

void PlaybackDevice::tagProfiles(StreamProfiles profiles) const {

}

std::string PlaybackDevice::getInfo(CameraInfo info) const {
  return InfoContainer::getInfo(info);
}

bool PlaybackDevice::supportsInfo(CameraInfo info) const {
  return InfoContainer::supportsInfo(info);
}

//std::string PlaybackDevice::getInfo(CameraInfo info) const {
//  return InfoContainer::getInfo(info);
//}
//
//bool PlaybackDevice::supportsInfo(CameraInfo info) const {
//  return InfoContainer::supportsInfo(info);
//}

}  // namespace libsmartereye2
