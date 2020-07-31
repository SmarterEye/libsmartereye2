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

#ifndef LIBSMARTEREYE2_DEVICE_HPP
#define LIBSMARTEREYE2_DEVICE_HPP

#include <utility>

#include "sensor/sensor.hpp"
#include "se_types.hpp"

namespace se2 {

class SMARTEREYE2_API Device {
 public:
  Device() : device_(nullptr) {}
  explicit Device(std::shared_ptr<SeDevice> dev) : device_(std::move(dev)) {}
  virtual ~Device() = default;

  explicit operator std::shared_ptr<SeDevice>() const { return device_; }
  explicit operator bool() const { return device_ != nullptr; }
  Device &operator=(std::shared_ptr<SeDevice> dev) {
    device_.reset();
    device_ = std::move(dev);
    return *this;
  }
  Device &operator=(const Device &dev) {
    *this = nullptr;
    device_ = dev.device_;
    return *this;
  }

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

}  // namespace se2

#endif  // LIBSMARTEREYE2_DEVICE_H
