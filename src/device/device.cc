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
#include "se_types.h"
#include "easylogging++.h"

#include <map>
#include <mutex>
#include <utility>

namespace libsmartereye2 {


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

DevicePrivate::~DevicePrivate() {
  if (device_changed_notifications_) {
    context_->unregisterInternalDeviceCallback(callback_id_);
  }
  sensors_.clear();
}

SensorInterface &DevicePrivate::getSensor(size_t i) {
  try {
    return *(sensors_.at(i));
  }
  catch (std::out_of_range) {
    LOG(ERROR) << "invalid subdevice value";
    throw std::exception();
  }
}

const SensorInterface &DevicePrivate::getSensor(size_t i) const {
  try {
    return *(sensors_.at(i));
  }
  catch (std::out_of_range) {
    LOG(ERROR) << "invalid subdevice value";
    throw std::exception();
  }
}

size_t DevicePrivate::getSensorCount() const {
  return sensors_.size();
}

void DevicePrivate::hardwareReset() {
// TODO
}

std::string DevicePrivate::getInfo(CameraInfo info) const {
  return ""; // TODO
}

bool DevicePrivate::supportsInfo(CameraInfo info) const {
  return true;  // TODO
}

std::shared_ptr<ContextPrivate> DevicePrivate::getContext() const {
  return
      context_;
}

std::pair<int32_t, Extrinsics> DevicePrivate::getExtrinsics(const StreamProfileBase &stream) const {
// TODO
  return std::pair<int32_t, Extrinsics>();
}

bool DevicePrivate::isValid() const {
  std::lock_guard<std::mutex> lock(device_changed_mtx_);
  return is_valid_;
}

int DevicePrivate::addSensor(const std::shared_ptr<SensorInterface> &sensor_base) { return 0; }
int DevicePrivate::assignSensor(const std::shared_ptr<SensorInterface> &sensor_base, int8_t idx) { return 0; }
void DevicePrivate::registerStreamToExtrinsicGroup(const StreamProfileBase &stream, int32_t groupd_index) {}

Device &Device::operator=(std::shared_ptr<SeDevice> dev) {
  device_.reset();
  device_ = std::move(dev);
  return *this;
}

Device &Device::operator=(const Device &dev) {
  *this = nullptr;
  device_ = dev.device_;
  return *this;
}

std::vector<Sensor> Device::querySensors() const {
  CHECK_PTR_NOT_NULL(device_);
  std::vector<Sensor> results;
  auto device_interface = device_->device;

  for (size_t i = 0; i < device_interface->getSensorCount(); i++) {
    std::shared_ptr<SeSensor> se_sensor(new SeSensor(*device_, &device_interface->getSensor(i)));
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

}  // namespace libsmartereye2