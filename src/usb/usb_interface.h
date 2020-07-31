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

#ifndef LIBSMARTEREYE2_USB_INTERFACE_H
#define LIBSMARTEREYE2_USB_INTERFACE_H

#include "usb_types.h"
#include "usb_endpoint.h"
#include "libusb.h"

namespace libsmartereye2 {

namespace platform {

class UsbInterface;

typedef std::shared_ptr<UsbInterface> SeUsbInterface;

class UsbInterface {
 public:
  explicit UsbInterface(const libusb_interface *interface);
  ~UsbInterface() = default;

  uint8_t getNumber() const { return usb_desc_.bInterfaceNumber; }
  uint8_t getClass() const { return usb_desc_.bInterfaceClass; }
  uint8_t getSubclass() const { return usb_desc_.bInterfaceSubClass; }

  std::vector<SeUsbEndpoint> getEndpoints() const { return endpoints_; }

  SeUsbEndpoint firstEndpoint(EndpointDirection direction,
                              EndpointType type = EndpointType::SE2_USB_ENDPOINT_BULK) const;

  std::vector<SeUsbInterface> getAssociatedInterfaces() const { return associated_interfaces_; }

  void addAssociatedInterface(const SeUsbInterface &interface);

 private:
  libusb_interface_descriptor usb_desc_;
  std::vector<std::shared_ptr<UsbEndpoint>> endpoints_;
  std::vector<SeUsbInterface> associated_interfaces_;
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_INTERFACE_H
