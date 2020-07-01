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

#ifndef LIBSMARTEREYE2_UPDATE_DEVICE_H
#define LIBSMARTEREYE2_UPDATE_DEVICE_H

#include "device.h"

namespace libsmartereye2 {

class UpdateDevice : public Device {
 public:
  UpdateDevice();
  explicit UpdateDevice(const Device& device);

  void update(const std::vector<uint8_t> &fw_image) const;

  template<class T>
  void update(const std::vector<uint8_t> &fw_image, T callback) const;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_UPDATE_DEVICE_H
