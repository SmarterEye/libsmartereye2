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

#ifndef LIBSMARTEREYE2_USB_ENDPOINT_H
#define LIBSMARTEREYE2_USB_ENDPOINT_H

#include "usb_types.h"
#include "libusb.h"

#include <memory>

namespace libsmartereye2 {

namespace platform {

class UsbEndpoint {
 public:
  UsbEndpoint(libusb_endpoint_descriptor desc, uint8_t interface_number) :
      endpoint_descriptor_(desc), interface_number_(interface_number) {}

  uint8_t getAddress() const { return endpoint_descriptor_.bEndpointAddress; }
  EndpointType getType() const { return (EndpointType) endpoint_descriptor_.bmAttributes; }
  uint8_t getInterfaceNumber() const { return interface_number_; }

  EndpointDirection getDirection() const {
    return endpoint_descriptor_.bEndpointAddress >= EndpointDirection::SE2_USB_ENDPOINT_DIRECTION_READ ?
           EndpointDirection::SE2_USB_ENDPOINT_DIRECTION_READ : EndpointDirection::SE2_USB_ENDPOINT_DIRECTION_WRITE;
  }

  libusb_endpoint_descriptor getDescriptor() { return endpoint_descriptor_; }

 private:
  libusb_endpoint_descriptor endpoint_descriptor_;
  uint8_t interface_number_;
};

typedef std::shared_ptr<UsbEndpoint> SeUsbEndpoint;

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_ENDPOINT_H
