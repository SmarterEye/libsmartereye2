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

#ifndef LIBSMARTEREYE2_EVENT_INFOMATION_H
#define LIBSMARTEREYE2_EVENT_INFOMATION_H

#include "device/device_list.h"

namespace libsmartereye2 {

class EventInfomation {
 public:
  EventInfomation(DeviceList removed, DeviceList added)
      : removed_(std::move(removed)), added_(std::move(added)) {}

  bool wasRemoved(const DevicePrivate &dev) const {
    for (auto info : removed_.getList()->list) {
      // TODO
    }
    return false;
  }

  bool wasAdded(const DevicePrivate &dev) const {
    for (auto info : added_.getList()->list) {
      // TODO
    }
    return false;
  }

  DeviceList getNewDevices() const { return added_; }

 private:
  DeviceList removed_;
  DeviceList added_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_EVENT_INFOMATION_H
