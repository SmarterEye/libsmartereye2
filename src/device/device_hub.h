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

#ifndef LIBSMARTEREYE2_DEVICE_HUB_H
#define LIBSMARTEREYE2_DEVICE_HUB_H

#include <memory>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "se_types.hpp"

namespace libsmartereye2 {

class DeviceHubPrivate;
class DeviceInterface;
class DeviceInfo;
class ContextPrivate;

class DeviceHubPrivate {
 public:
  explicit DeviceHubPrivate(std::shared_ptr<ContextPrivate> ctx,
                            int mask = ProductCode::SE_PRODUCT_ANY,
                            int vid = 0,
                            bool register_device_notifications = true);

  ~DeviceHubPrivate();

  std::shared_ptr<DeviceInterface> waitForDevice(const std::chrono::milliseconds &timeout = std::chrono::milliseconds(
      std::chrono::hours(1)), bool loop_through_devices = true, const std::string &serial = "");

  bool isConnected(const DeviceInterface &dev) const;

  std::vector<std::shared_ptr<DeviceInfo>> getDeviceList() const { return device_list_; }

 private:
  std::shared_ptr<DeviceInterface> createDevice(const std::string &serial, bool cycle_devices = true);

  std::shared_ptr<ContextPrivate> context_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<std::shared_ptr<DeviceInfo>> device_list_{};
  int camera_index_ = 0;
  int vid_ = 0;
  int64_t device_changes_callback_id_;
  bool register_device_notifications_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_HUB_H
