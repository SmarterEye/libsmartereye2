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

#include "device_hub.h"
#include "device/device.h"
#include "device/device_info.h"
#include "easylogging++.h"

#include "device/device_hub.hpp"

#include <memory>
#include "smartereye2/device/context.hpp"

namespace libsmartereye2 {

using HubDevicesChangedCallback = se2::DevicesChangedCallback<std::function<void(se2::EventInfomation &info)>>;

std::vector<std::shared_ptr<DeviceInfo>> filter_by_vid(const std::vector<std::shared_ptr<DeviceInfo>> &devices,
                                                       int vid) {
  std::vector<std::shared_ptr<DeviceInfo>> result;
  for (auto dev : devices) {
    bool filtered = false;
    auto data = dev->getDeviceData();
    for (const auto &usb : data.usb_devices) {
      if (usb.vid == vid || vid == 0) {
        result.push_back(dev);
        filtered = true;
        break;
      }
    }
    for (const auto &uvc : data.usb_devices) {
      if (uvc.vid == vid || vid == 0) {
        result.push_back(dev);
        filtered = true;
        break;
      }
    }
  }
  return result;
}

DeviceHubPrivate::DeviceHubPrivate(std::shared_ptr<ContextPrivate> ctx,
                                   int mask,
                                   int vid,
                                   bool register_device_notifications)
    : context_(std::move(ctx)),
      vid_(vid),
      device_changes_callback_id_(0),
      register_device_notifications_(register_device_notifications) {

//  device_list_ = filter_by_vid(context_->queryDevices(mask), vid_);
  DevicesChangedCallbackPtr cb(new HubDevicesChangedCallback([this, mask](se2::EventInfomation &) {
    std::unique_lock<std::mutex> lock(mutex_);
    device_list_ = filter_by_vid(context_->queryDevices(mask), vid_);
    camera_index_ = 0;
    if (!device_list_.empty()) {
      cv_.notify_all();
    }
  }));
  device_changes_callback_id_ = context_->registerInternalDeviceCallback(cb);
}

DeviceHubPrivate::~DeviceHubPrivate() {
  if (device_changes_callback_id_) {
    context_->unregisterInternalDeviceCallback(device_changes_callback_id_);
  }
  context_->stop();
}

std::shared_ptr<DeviceInterface> DeviceHubPrivate::waitForDevice(const std::chrono::milliseconds &timeout,
                                                                 bool loop_through_devices,
                                                                 const std::string &serial) {
  std::unique_lock<std::mutex> lock(mutex_);
  std::shared_ptr<DeviceInterface> res = nullptr;

  device_list_ = filter_by_vid(context_->queryDevices(ProductCode::SE_PRODUCT_ANY), vid_);

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

bool DeviceHubPrivate::isConnected(const DeviceInterface &dev) {
  std::unique_lock<std::mutex> lock(mutex_);
  return dev.isValid();
}

std::shared_ptr<DeviceInterface> DeviceHubPrivate::createDevice(const std::string &serial, bool cycle_devices) {
  std::shared_ptr<DeviceInterface> res = nullptr;
  for (auto i = 0; ((i < device_list_.size()) && (nullptr == res)); i++) {
    // _camera_index is the curr device that the hub will expose
    auto d = device_list_[(camera_index_ + i) % device_list_.size()];
    try {
      auto dev = d->createDevice(register_device_notifications_);

      if (!serial.empty()) {
        auto new_serial = dev->getInfo(CameraInfo::CAMERA_INFO_SERIAL_NUMBER);

        if (serial == new_serial) {
          res = dev;
          cycle_devices = false;  // Requesting a device by its serial shall not invoke internal cycling
        }
      } else // Use the first selected if "any device" pattern was used
      {
        res = dev;
      }
    }
    catch (const std::exception &ex) {
      LOG(WARNING) << "Could not open device " << ex.what();
    }
  }

  // Advance the internal selection when appropriate
  if (res && cycle_devices)
    camera_index_ = (camera_index_ + 1) % device_list_.size();

  return res;
}

}  // namespace libsmartereye2

namespace se2 {

DeviceHub::DeviceHub(const Context &context) {
  device_hub_ = std::make_shared<SeDeviceHub>(SeDeviceHub{
      std::make_shared<libsmartereye2::DeviceHubPrivate>(context.context_->context)});
}

Device DeviceHub::waitForDevice() const {
  auto dev_interface = device_hub_->hub->waitForDevice();
  std::shared_ptr<SeDevice> device(new SeDevice{
      device_hub_->hub->getContext(),
      std::make_shared<ReadonlyDeviceInfo>(dev_interface),
      dev_interface
  });
  return Device(device);
}

bool DeviceHub::isConnected(const Device &dev) const {
  bool res = device_hub_->hub->isConnected(*dev.device_->device);
  return res;
}

}  // namespace se2

