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

#include "usb_context.h"

#include <cassert>
#include "easylogging++.h"

namespace libsmartereye2 {

namespace platform {

UsbContext::UsbContext()
    : usb_context_(nullptr), usb_device_list_(nullptr), usb_device_count_(0) {
  int res = libusb_init(&usb_context_);
  if (res != LIBUSB_SUCCESS) {
    LOG(ERROR) << "libusb_init failed: " << res;
  }
  usb_device_count_ = libusb_get_device_list(usb_context_, &usb_device_list_);

  libusb_device *foo = nullptr;
  libusb_device *bar = nullptr;
  libusb_device_descriptor desc;
  int ret = 0;
  for (int i = 0; i < usb_device_count_; i++) {
    foo = usb_device_list_[i];
    ret = libusb_get_device_descriptor(foo, &desc);
    if (desc.idVendor==0xF525 && desc.idProduct==0xC4a0) {
      bar = foo;
      break;
    }
  }

//  int xx = libusb_open(bar, &dev_handle_);
  void(0);
}

UsbContext::~UsbContext() {
  libusb_free_device_list(usb_device_list_, true);
  assert(handler_requests_ == 0);
  if (event_handler_thread_.joinable()) {
    event_handler_thread_.join();
  }
//  libusb_close(dev_handle_);
  libusb_exit(usb_context_);
}

void UsbContext::startEventHandler() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!handler_requests_) {
    if (event_handler_thread_.joinable()) {
      event_handler_thread_.join();
      kill_hendler_thread_num_ = 0;
    }
    event_handler_thread_ = std::thread([this]() {
      while (kill_hendler_thread_num_ != 0) {
        libusb_handle_events_completed(usb_context_, &kill_hendler_thread_num_);
      }
    });
  }
  handler_requests_++;
}

void UsbContext::stopEventHandler() {
  std::lock_guard<std::mutex> lock(mutex_);
  handler_requests_--;
  if (!handler_requests_) {
    kill_hendler_thread_num_ = 1;
  }
}
libusb_device *UsbContext::getDevice(uint8_t index) {
  return index < usb_device_count_ ? usb_device_list_[index] : nullptr;
}

}  // namespace platform

}  // namespace libsmartereye2
