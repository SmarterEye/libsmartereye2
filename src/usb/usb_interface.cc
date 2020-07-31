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


#include "usb_interface.h"

namespace libsmartereye2 {

namespace platform {

UsbInterface::UsbInterface(const libusb_interface *interface)
    : usb_desc_(*interface->altsetting) {
  for (int i = 0; i < usb_desc_.bNumEndpoints; ++i) {
    auto ep = usb_desc_.endpoint[i];
    endpoints_.push_back(std::make_shared<UsbEndpoint>(ep, usb_desc_.bInterfaceNumber));
  }
}

SeUsbEndpoint UsbInterface::firstEndpoint(EndpointDirection direction, EndpointType type) const {
  for (auto &&ep : endpoints_) {
    if (ep->getType() != type) continue;
    if (ep->getDirection() != direction) continue;

    return ep;
  }
  return nullptr;
}

void UsbInterface::addAssociatedInterface(const SeUsbInterface &interface) {
  if (interface) {
    associated_interfaces_.push_back(interface);
  }
}

}  // namespace platform

}  // namespace libsmartereye2
