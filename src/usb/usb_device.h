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

#ifndef LIBSMARTEREYE2_USB_DEVICE_H
#define LIBSMARTEREYE2_USB_DEVICE_H

#include "usb_types.h"
#include "usb_interface.h"
#include "usb_messenger.h"
#include "usb_context.h"
#include "libusb.h"

#include <memory>

namespace libsmartereye2 {

namespace platform {

class UsbHandle;

class UsbDevice : public std::enable_shared_from_this<UsbDevice> {
 public:
  UsbDevice(libusb_device *device,
            const libusb_device_descriptor &desc,
            UsbDeviceInfo info,
            std::shared_ptr<UsbContext> context);

  ~UsbDevice();

  UsbDeviceInfo getInfo() const { return info_; }
  std::vector<SeUsbInterface> getInterfaces() const { return interfaces_; }
  SeUsbInterface getInterface(uint8_t interface_number) const;
  SeUsbMessenger open(uint8_t interface_number);
  const std::vector<UsbDescriptor> get_descriptors() const { return descriptors_; }

 private:
  libusb_device *device_;
  libusb_device_descriptor usb_device_descriptor_;
  const UsbDeviceInfo info_{};
  std::vector<std::shared_ptr<UsbInterface>> interfaces_;
  std::vector<UsbDescriptor> descriptors_;
  std::shared_ptr<UsbContext> context_;

  std::shared_ptr<UsbHandle> get_handle(uint8_t interface_number) const;
};

using SeUsbDevice = std::shared_ptr<UsbDevice>;

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_DEVICE_H
