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

#ifndef LIBSMARTEREYE2_USB_CONTEXT_H
#define LIBSMARTEREYE2_USB_CONTEXT_H

#include <thread>
#include <mutex>
#include <libusb.h>

namespace libsmartereye2 {

namespace platform {

class UsbContext {
 public:
  UsbContext();
  ~UsbContext();

  libusb_context *get() const { return usb_context_; }

  void startEventHandler();
  void stopEventHandler();

  size_t deviceCount() const { return usb_device_count_; }
  libusb_device *getDevice(uint8_t index);

 private:
  std::mutex mutex_;
  libusb_device **usb_device_list_;
  size_t usb_device_count_;
  int handler_requests_ = 0;
  libusb_context *usb_context_;
  libusb_device_handle *dev_handle_;
  int kill_hendler_thread_num_ = 0;
  std::thread event_handler_thread_;
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_CONTEXT_H
