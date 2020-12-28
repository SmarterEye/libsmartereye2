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

#include "device.h"

#include "easylogging++.h"
#include "device/device_info.h"
#include "streaming/stream_profile.h"

#include "device/device.hpp"
#include "device/device_list.hpp"

#include <map>
#include <mutex>
#include <utility>

namespace libsmartereye2 {

DevicePrivate::~DevicePrivate() {
  if (device_changed_notifications_) {
    context_->unregisterDevicesChangedCallback(callback_id_);
  }
  sensors_.clear();
}

SensorInterface &DevicePrivate::getSensor(size_t i) {
  try {
    return *(sensors_.at(i));
  }
  catch (std::out_of_range &e) {
    LOG(ERROR) << "invalid subdevice value";
    throw std::exception();
  }
}

const SensorInterface &DevicePrivate::getSensor(size_t i) const {
  try {
    return *(sensors_.at(i));
  }
  catch (std::out_of_range &e) {
    LOG(ERROR) << "invalid subdevice value";
    throw std::exception();
  }
}

size_t DevicePrivate::getSensorCount() const {
  return sensors_.size();
}

void DevicePrivate::hardwareReset() {
  LOG(WARNING) << __FUNCTION__ << " is not implemented for this device!";
}

std::shared_ptr<ContextPrivate> DevicePrivate::getContext() const {
  return context_;
}

std::pair<int32_t, Extrinsics> DevicePrivate::getExtrinsics(const StreamInterface &stream) const {
// TODO
  return {};
}

platform::BackendDeviceGroup DevicePrivate::getDeviceData() const {
  return group_;
}

bool DevicePrivate::isValid() const {
  std::lock_guard<std::mutex> lock(device_changed_mtx_);
  return is_valid_;
}

void DevicePrivate::tagProfiles(StreamProfiles profiles) const {
  for (const auto &profile : profiles) {
    for (auto tag : profiles_tags_) {
      if (auto vp = dynamic_cast<VideoStreamProfilePrivate *>(profile.get())) {
        if ((tag.frame_id == FrameId::NotUsed || vp->frameId() == tag.frame_id) &&
            (tag.format == FrameFormat::Any || vp->format() == tag.format) &&
            (tag.width == -1 || vp->width() == tag.width) &&
            (tag.height == -1 || vp->height() == tag.height) &&
            (tag.fps == -1 || vp->fps() == tag.fps) &&
            (tag.stream_index == -1 || vp->index() == tag.stream_index))
          profile->tagProfile(tag.tag);
      } else if (auto mp = dynamic_cast<MotionStreamProfilePrivate *>(profile.get())) {
        if ((tag.frame_id == FrameId::NotUsed || mp->frameId() == tag.frame_id) &&
            (tag.format == FrameFormat::Any || mp->format() == tag.format) &&
            (tag.fps == -1 || mp->fps() == tag.fps) &&
            (tag.stream_index == -1 || mp->index() == tag.stream_index))
          profile->tagProfile(tag.tag);
      }
    }
  }
}

void DevicePrivate::setValid(bool is_valid) {
  std::lock_guard<std::mutex> lock(device_changed_mtx_);
  is_valid_ = is_valid;
}

int DevicePrivate::addSensor(const std::shared_ptr<SensorInterface> &sensor_base) {
  sensors_.push_back(sensor_base);
  return static_cast<int>(sensors_.size()) - 1;
}

int DevicePrivate::assignSensor(const std::shared_ptr<SensorInterface> &sensor_base, int8_t idx) {
  try {
    sensors_[idx] = sensor_base;
    return (int) sensors_.size() - 1;
  }
  catch (std::out_of_range &e) {
    throw std::runtime_error(toString() << "Cannot assign sensor - invalid subdevice value" << idx);
  }
}

void DevicePrivate::registerStreamToExtrinsicGroup(const StreamProfileBase &stream, int32_t groupd_index) {
  auto cond = [groupd_index](const std::pair<int, std::pair<uint32_t, std::shared_ptr<const StreamInterface>>> &p) {
    return p.second.first == groupd_index;
  };
  auto iter = std::find_if(extrinsics_.begin(), extrinsics_.end(), cond);

  if (iter == extrinsics_.end()) {
    //First stream to register for this group
    extrinsics_[stream.uniqueId()] = std::make_pair(groupd_index, stream.shared_from_this());
  } else {
    //iter->second holds the group_id and the key stream
    extrinsics_[stream.uniqueId()] = iter->second;
  }
}

DevicePrivate::DevicePrivate(std::shared_ptr<ContextPrivate> context,
                             platform::BackendDeviceGroup group,
                             bool device_changed_notifications)
    : context_(std::move(context)),
      group_(std::move(group)),
      is_valid_(true),
      device_changed_notifications_(device_changed_notifications) {

}

}  // namespace libsmartereye2

namespace se2 {

std::vector<Sensor> Device::querySensors() const {
  std::vector<Sensor> results;
  auto device_interface = device_->device;

  for (size_t i = 0; i < device_interface->getSensorCount(); i++) {
    libsmartereye2::SensorInterface *sensor = &device_->device->getSensor(i);
    std::shared_ptr<SeSensor> se_sensor(new SeSensor(device_.get(), sensor));
    Sensor dev(se_sensor);
    results.push_back(dev);
  }
  return results;
}

bool Device::supports(CameraInfo info) const {
  return device_->device->supportsInfo(info);
}

std::string Device::getInfo(CameraInfo info) const {
  if (supports(info)) {
    return device_->device->getInfo(info);
  } else {
    LOG(ERROR) << "not supported by the device!";
    throw std::exception();
  }
}

void Device::hardwareReset() {
  device_->device->hardwareReset();
}

Device DeviceList::operator[](uint32_t index) const {
  std::shared_ptr<SeDevice> dev(new SeDevice{
      list_->context,
      list_->list[index].info,
      list_->list[index].info->createDevice()
  });
  return Device(dev);
}

bool DeviceList::contains(const Device &dev) const {
  for (const auto &info : list_->list) {
    if (dev.get()->info && dev.get()->info->getDeviceData() == info.info->getDeviceData()) {
      return true;
    }
  }
  return false;
}

int32_t DeviceList::size() const {
  return static_cast<int32_t>(list_->list.size());
}

}
