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

#include "backend.h"

#include "usb/usb_enumerator.h"

namespace libsmartereye2 {
namespace platform {

ControlRange::ControlRange(int32_t in_min, int32_t in_max, int32_t in_step, int32_t in_def) {

}

ControlRange::ControlRange(std::vector<uint8_t> in_min,
                           std::vector<uint8_t> in_max,
                           std::vector<uint8_t> in_step,
                           std::vector<uint8_t> in_def) {

}

void ControlRange::populateRawData(std::vector<uint8_t> &vec, int32_t value) {

}

StandardBackend::StandardBackend() {

}

StandardBackend::~StandardBackend() {

}

std::shared_ptr<CommandTransfer> StandardBackend::createUsbDevice(UsbDeviceInfo info) const {
  auto dev = UsbEnumerator::createUsbDevcie(info);
  if (dev) {
    return std::make_shared<UsbCommandTransfer>(dev);
  }
  return nullptr;
}

std::vector<UsbDeviceInfo> StandardBackend::queryUsbDevices() const {
  auto device_infos = UsbEnumerator::queryDevicesInfo();
  return device_infos;
}

std::shared_ptr<platform::TimeService> StandardBackend::createTimeService() const {
  return std::shared_ptr<platform::OsTimeService>();
}

std::shared_ptr<DeviceWather> StandardBackend::createDeviceWatcher() const {
  return std::make_shared<PollingDeviceWatcher>(this);
}

PollingDeviceWatcher::PollingDeviceWatcher(const Backend *backend)
    : backend_(backend),
      repeat_operation_([this](Dispatcher::CancellableTimer timer) {
        polling(timer);
      }),
      discovered_devices_() {
}

PollingDeviceWatcher::~PollingDeviceWatcher() {
  stop();
}

void PollingDeviceWatcher::polling(Dispatcher::CancellableTimer timer) {
  if (timer.trySleep(1000)) {
    platform::BackendDeviceGroup curr(backend_->queryUsbDevices());
    if (listChanged(discovered_devices_.usb_devices, curr.usb_devices)) {
      CallbackInvocationHolder callback = {callback_inflight_.allocate(), &callback_inflight_};
      if (callback) {
        callback_(discovered_devices_, curr);
        discovered_devices_ = curr;
      }
    }
  }
}

void PollingDeviceWatcher::start(DeviceChangedCallback callback) {
  stop();
  callback_ = std::move(callback);
  repeat_operation_.start();
}

void PollingDeviceWatcher::stop() {
  repeat_operation_.stop();
  callback_inflight_.waitUntilEmpty();
}

}  // namespace platform
}  // namespace libsmartereye2
