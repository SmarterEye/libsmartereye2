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

#ifndef LIBSMARTEREYE2_USB_HANDLE_H
#define LIBSMARTEREYE2_USB_HANDLE_H

#include <memory>

#include "usb_types.h"
#include "libusb.h"

namespace libsmartereye2 {

namespace platform {

class UsbInterface;
class UsbContext;

class UsbHandle {
 public:
  UsbHandle(std::shared_ptr<UsbContext> context,
            libusb_device *device, const std::shared_ptr<UsbInterface>& interface);

  ~UsbHandle();

  libusb_device_handle *get() const { return _handle; }

 private:
  void claim_interface_or_throw(uint8_t interface);

  int claim_interface(uint8_t interface_number);

  std::shared_ptr<UsbContext> _context;
  std::shared_ptr<UsbInterface> _first_interface;
  libusb_device_handle *_handle;
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_HANDLE_H
