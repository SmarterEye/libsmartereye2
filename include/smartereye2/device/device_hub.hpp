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

#ifndef LIBSMARTEREYE2_DEVICE_HUB_HPP
#define LIBSMARTEREYE2_DEVICE_HUB_HPP

#include "se_types.hpp"

namespace se2 {

class Context;
class Device;

class DeviceHub {
 public:
  explicit DeviceHub(const Context &context);

  explicit operator std::shared_ptr<SeDeviceHub>() { return device_hub_; }
  explicit DeviceHub(std::shared_ptr<SeDeviceHub> hub) : device_hub_(std::move(hub)) {}

  Device waitForDevice() const;

  bool isConnected(const Device &dev) const;

 private:
  std::shared_ptr<SeDeviceHub> device_hub_;
};

}  // namespace se2

#endif  // LIBSMARTEREYE2_DEVICE_HUB_HPP
