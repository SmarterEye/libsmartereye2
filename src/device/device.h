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

#ifndef LIBSMARTEREYE2_DEVICE_H
#define LIBSMARTEREYE2_DEVICE_H

#include <utility>

#include "sensor.h"
#include "context.h"

namespace libsmartereye2 {

class EventInfomation;
class DeviceList;

template<class T>
class DevicesChangedCallback : public SeDevicesChangedCallback {
 public:
  explicit DevicesChangedCallback(T callback) : callback_(callback) {}
  void onDevicesChanged(SeDeviceList *removed, SeDeviceList *added) override {
    std::shared_ptr<SeDeviceList> old(removed);
    std::shared_ptr<SeDeviceList> news(added);

    EventInfomation info(DeviceList(old), DeviceList(news));
    callback_(info);
  }
  void release() override { delete this; }

 private:
  T callback_;
};

class DeviceInterface : public std::enable_shared_from_this<DeviceInterface> {
 public:
  virtual SensorInterface &getSensor(size_t i) = 0;
  virtual const SensorInterface &getSensor(size_t i) const = 0;
  virtual size_t getSensorCount() const = 0;
  virtual void hardwareReset() = 0;
  virtual std::string getInfo(CameraInfo info) const = 0;
  virtual bool supportsInfo(CameraInfo info) const = 0;

  virtual std::shared_ptr<ContextPrivate> getContext() const = 0;
  virtual std::pair<int32_t, Extrinsics> getExtrinsics(const StreamProfileBase &stream) const = 0;

  virtual bool isValid() const = 0;
};

class DevicePrivate : public DeviceInterface {
 public:
  virtual ~DevicePrivate();

  SensorInterface &getSensor(size_t i) override;
  const SensorInterface &getSensor(size_t i) const override;
  size_t getSensorCount() const override;
  void hardwareReset() override;
  std::string getInfo(CameraInfo info) const override;
  bool supportsInfo(CameraInfo info) const override;
  std::shared_ptr<ContextPrivate> getContext() const override;
  std::pair<int32_t, Extrinsics> getExtrinsics(const StreamProfileBase &stream) const override;
  bool isValid() const override;

 protected:
  int addSensor(const std::shared_ptr<SensorInterface> &sensor_base);
  int assignSensor(const std::shared_ptr<SensorInterface> &sensor_base, int8_t idx);
  void registerStreamToExtrinsicGroup(const StreamProfileBase &stream, int32_t groupd_index);

  std::map<int, std::pair<int32_t, std::shared_ptr<const StreamProfileBase>>> extrinsics_;

 private:
  std::vector<std::shared_ptr<SensorInterface>> sensors_;
  std::shared_ptr<ContextPrivate> context_;
  bool is_valid_;
  bool device_changed_notifications_;
  mutable std::mutex device_changed_mtx_;
  int64_t callback_id_;
};

class DeviceHub;

class Device {
 public:
  Device() : device_(nullptr) {}
  explicit Device(std::shared_ptr<SeDevice> dev) : device_(std::move(dev)) {}
  virtual ~Device() = default;

  explicit operator std::shared_ptr<SeDevice>() const { return device_; }
  explicit operator bool() const { return device_ != nullptr; }
  Device &operator=(std::shared_ptr<SeDevice> dev);
  Device &operator=(const Device &dev);

  std::vector<Sensor> querySensors() const;
  bool supports(CameraInfo info) const;
  std::string getInfo(CameraInfo info) const;
  void hardwareReset();

  template<typename T>
  T first() const {
    for (auto &&s : querySensors()) {
      if (auto t = s.as<T>()) return t;
    }
  }

  template<typename T>
  bool is() const {
    T extension(*this);
    return extension;
  }

  template<typename T>
  T as() const {
    T extension(*this);
    return extension;
  }

  const std::shared_ptr<SeDevice> &get() const {
    return device_;
  }

 protected:
  friend class DeviceHub;

  std::shared_ptr<SeDevice> device_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_H
