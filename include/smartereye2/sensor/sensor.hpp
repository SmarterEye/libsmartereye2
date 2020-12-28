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

#ifndef LIBSMARTEREYE2_SENSOR_HPP
#define LIBSMARTEREYE2_SENSOR_HPP

#include <memory>
#include <utility>
#include <vector>

#include "smartereye2/se_global.hpp"
#include "smartereye2/se_types.hpp"
#include "smartereye2/core/options.hpp"

namespace se2 {

class StreamProfile;

class SMARTEREYE2_API Sensor : public Options {
 public:
  using Options::supports;

  Sensor() : sensor_(nullptr) {}
  explicit Sensor(const std::shared_ptr<SeSensor> &dev);
  explicit operator std::shared_ptr<SeSensor>();
  Sensor &operator=(const std::shared_ptr<SeSensor> &other);
  Sensor &operator=(const Sensor &other);

  explicit operator bool() const {
    return sensor_ != nullptr;
  }

  const std::shared_ptr<SeSensor> &get() const {
    return sensor_;
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

  bool supports(CameraInfo info) const;
  std::string getInfo(CameraInfo info) const;

  void open(const StreamProfile &profile) const;
  void open(const std::vector<StreamProfile> &profiles) const;
  void close() const;

  template<typename T>
  void setNotificationCallback(T callback) const {}

  template<typename T>
  void start(T callback) const {}
  void stop() const {}

  std::vector<StreamProfile> getStreamProfiles() const;
  std::vector<StreamProfile> getActiveStreams() const;

 private:
  std::shared_ptr<SeSensor> sensor_;
};

class SMARTEREYE2_API ColorSensor : public Sensor {

};

class SMARTEREYE2_API MotionSensor : public Sensor {

};

class SMARTEREYE2_API RoiSensor : public Sensor {

};

class SMARTEREYE2_API DepthSensor : public Sensor {

};

class SMARTEREYE2_API DepthStereoSensor : public DepthSensor {

};

}  // namespace se2

#endif  // LIBSMARTEREYE2_SENSOR_HPP
