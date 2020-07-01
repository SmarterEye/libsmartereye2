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

#include "context.h"
#include "device_list.h"

namespace libsmartereye2 {

std::vector<std::shared_ptr<DeviceInfo>> ContextPrivate::queryDevices(int mask) const {
  return std::vector<std::shared_ptr<DeviceInfo>>();
}
int64_t ContextPrivate::registerInternalDeviceCallback(DevicesChangedCallbackPtr cb) {
  return 0;
}
void ContextPrivate::unregisterInternalDeviceCallback(int64_t cb_id) {

}
void ContextPrivate::setDevicesChangedCallback(DevicesChangedCallbackPtr cb) {

}
std::vector<std::shared_ptr<DeviceInfo>> ContextPrivate::createDevices(int mask) const {
  return std::vector<std::shared_ptr<DeviceInfo>>();
}

Context::Context()
    : context_(new SeContext) {
}

Context::Context(std::shared_ptr<SeContext> context)
    : context_(context) {
}

Context::operator std::shared_ptr<SeContext>() const {
  return context_;
}

DeviceList Context::queryDevices() const {
  return DeviceList();
}

DeviceList Context::queryDevices(int mask) const {
  return DeviceList();
}

std::vector<Sensor> Context::queryAllSensors() const {
  return std::vector<Sensor>();
}

Device Context::getSensorParent(const Sensor &sensor) const {
  return Device();
}

}  // namespace libsmartereye2
