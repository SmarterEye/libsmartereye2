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

#include "update_device.h"

#include "device/update_device.hpp"

namespace se2 {

UpdateDevice::UpdateDevice()
    : Device() {}

UpdateDevice::UpdateDevice(const Device &device)
    : Device(device.get()) {

}

void UpdateDevice::update(const std::vector<uint8_t> &fw_image) const {

}

template<class T>
void UpdateDevice::update(const std::vector<uint8_t> &fw_image, T callback) const {

}

}  // namespace se2
