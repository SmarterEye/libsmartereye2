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
#include "se_util.h"

namespace libsmartereye2 {

class SensorPrivate : public noncopyable {
 public:
  explicit SensorPrivate(std::shared_ptr<SensorPrivate> &other) {}
};

Sensor::Sensor(std::shared_ptr<SeSensor> dev)
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

}  // namespace libsmartereye2
