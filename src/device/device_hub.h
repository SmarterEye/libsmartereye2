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

#include "device/context.h"
#include "device/device.h"
#include "device/device_info.h"
#include "device/device_list.h"
#include "se_types.h"

namespace libsmartereye2 {

class DeviceInterface;
class EventInfomation;

using HubDevicesChangedCallback = DevicesChangedCallback<std::function<void(EventInfomation &info)>>;

class DeviceHubPrivate {
 public:
  explicit DeviceHubPrivate(std::shared_ptr<ContextPrivate> ctx,
                            int mask = 0,
                            bool register_device_notifications = true)
      : context_(std::move(ctx)),
        device_changes_callback_id_(0),
        register_device_notifications_(register_device_notifications) {
    device_list_ = context_->queryDevices(mask);
    DevicesChangedCallbackPtr cb(new HubDevicesChangedCallback([mask](EventInfomation &) {}));
    context_->registerInternalDeviceCallback(cb);
  }

  ~DeviceHubPrivate() {
    if (device_changes_callback_id_) {
      context_->unregisterInternalDeviceCallback(device_changes_callback_id_);
    }
    context_->stop();
  }

  std::shared_ptr<DeviceInterface> waitForDevice(const std::chrono::milliseconds &timeout = std::chrono::milliseconds(
      std::chrono::hours(1)), bool loop_through_devices = true, const std::string &serial = "") {
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<DeviceInterface> res = nullptr;

    device_list_ = context_->queryDevices(0xff);

    auto device_create_func = [&]() -> bool {
      if (!device_list_.empty()) {
        res = createDevice(serial, loop_through_devices);
      }
      return res != nullptr;
    };

    if (device_create_func()) return res;

    if (!cv_.wait_for(lock, timeout, device_create_func)) {
      throw std::runtime_error("No device connected");
    }
    return res;
  }

  bool isConnected(const DeviceInterface &dev) {
    std::unique_lock<std::mutex> lock(mutex_);
    return dev.isValid();
  }

  std::shared_ptr<ContextPrivate> getContext() { return context_; }

 private:
  std::shared_ptr<DeviceInterface> createDevice(const std::string &serial, bool cycle_devices = true) {
    return nullptr; // TODO
  }

  std::shared_ptr<ContextPrivate> context_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<std::shared_ptr<DeviceInfo>> device_list_;
  int camera_index_ = 0;
//  int vid_ = 0;
  int64_t device_changes_callback_id_;
  bool register_device_notifications_;
};

class DeviceHub {
 public:
  explicit DeviceHub(const Context &context) {
    device_hub_ =
        std::make_shared<SeDeviceHub>(SeDeviceHub{
            std::make_shared<DeviceHubPrivate>(context.context_->ctx)
        });
  }

  Device waitForDevice() const {
    auto dev_interface = device_hub_->hub->waitForDevice();
    std::shared_ptr<SeDevice> device(new SeDevice{
        device_hub_->hub->getContext(),
        nullptr,
        dev_interface
    });
    return Device(device);
  }

  bool isConnected(const Device &dev) const {
    bool res = device_hub_->hub->isConnected(*dev.device_->device);
    return res;
  }

 private:
  std::shared_ptr<SeDeviceHub> device_hub_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_HUB_H
