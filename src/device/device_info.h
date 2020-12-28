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

#ifndef LIBSMARTEREYE2_DEVICE_INFO_H
#define LIBSMARTEREYE2_DEVICE_INFO_H

#include <memory>
#include <utility>
#include <vector>

#include "backend.h"
#include "mock/playback/playback_device.h"
#include "se_util.hpp"

namespace libsmartereye2 {

class DeviceInterface;
class ContextPrivate;

class DeviceInfo : public noncopyable {
 public:
  virtual std::shared_ptr<DeviceInterface> createDevice(bool register_device_notifications = false) const {
    return create(context_, register_device_notifications);
  }

  virtual platform::BackendDeviceGroup getDeviceData() const = 0;

  virtual bool operator==(const DeviceInfo &other) const {
    return getDeviceData() == other.getDeviceData();
  }

 protected:
  explicit DeviceInfo(std::shared_ptr<ContextPrivate> context)
      : context_(move(context)) {}

  virtual std::shared_ptr<DeviceInterface> create(std::shared_ptr<ContextPrivate> ctx,
                                                  bool register_device_notifications) const = 0;

  std::shared_ptr<ContextPrivate> context_;
};

class ReadonlyDeviceInfo : public DeviceInfo {
 public:
  explicit ReadonlyDeviceInfo(const std::shared_ptr<DeviceInterface> &dev)
      : DeviceInfo(dev->getContext()), dev_(dev) {}

  std::shared_ptr<DeviceInterface> createDevice(bool register_device_notifications) const override {
    return dev_;
  }

  platform::BackendDeviceGroup getDeviceData() const override {
    return dev_->getDeviceData();
  }

  std::shared_ptr<DeviceInterface> create(std::shared_ptr<ContextPrivate> ctx,
                                          bool register_device_notifications) const override {
    return dev_;
  }

 private:
  std::shared_ptr<DeviceInterface> dev_;
};

class PlaybackDeviceInfo : public DeviceInfo {
 public:
  explicit PlaybackDeviceInfo(std::shared_ptr<PlaybackDevice> dev)
      : DeviceInfo(nullptr), dev_(std::move(dev)) {

  }

  std::shared_ptr<DeviceInterface> createDevice(bool) const override {
    return dev_;
  }

  platform::BackendDeviceGroup getDeviceData() const override {
    return platform::BackendDeviceGroup({platform::PlaybackDeviceInfo{dev_->getFileName()}});
  }

 protected:
  std::shared_ptr<DeviceInterface> create(std::shared_ptr<ContextPrivate>, bool) const override {
    return dev_;
  }

 private:
  std::shared_ptr<PlaybackDevice> dev_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_INFO_H
