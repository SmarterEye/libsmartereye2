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

#include "usb_handle.h"

#include <utility>
#include "usb_interface.h"
#include "usb_context.h"
#include "easylogging++.h"

namespace libsmartereye2 {

namespace platform {

UsbHandle::UsbHandle(std::shared_ptr<UsbContext> context,
                     libusb_device *device,
                     const std::shared_ptr<UsbInterface> &interface)
    : _first_interface(interface),
      _context(std::move(context)),
      _handle(nullptr) {
  auto sts = libusb_open(device, &_handle);
  if (sts != LIBUSB_SUCCESS) {
    std::stringstream msg;
    msg << "failed to open usb interface: " << (int) interface->getNumber() << ", error: " << sts;
    LOG(ERROR) << msg.str();
    throw std::runtime_error(msg.str());
  }

  claim_interface_or_throw(interface->getNumber());
  for (auto &&i : interface->getAssociatedInterfaces()) {
    claim_interface_or_throw(i->getNumber());
  }

  _context->startEventHandler();
}

UsbHandle::~UsbHandle() {
  _context->stopEventHandler();
  for (auto &&i : _first_interface->getAssociatedInterfaces()) {
    libusb_release_interface(_handle, i->getNumber());
  }
  libusb_close(_handle);
}

void UsbHandle::claim_interface_or_throw(uint8_t interface) {
  auto sts = claim_interface(interface);
  if (sts != SE2_USB_STATUS_SUCCESS) {
    std::stringstream error_string;
    error_string <<"Unable to claim interface " << (int) interface << ", error: " << sts;
    throw std::runtime_error(error_string.str());
  }
}

int UsbHandle::claim_interface(uint8_t interface_number) {
  if (libusb_kernel_driver_active(_handle, interface_number) == 1)//find out if kernel driver is attached
    if (libusb_detach_kernel_driver(_handle, interface_number) == 0)// detach driver from device if attached.
      LOG(DEBUG) << "handle_libusb - detach kernel driver";

  auto sts = libusb_claim_interface(_handle, interface_number);
  if (sts != LIBUSB_SUCCESS) {
    LOG(ERROR) << "failed to claim usb interface: " << interface_number << ", error: " << sts;
    return sts;
  }

  return LIBUSB_SUCCESS;
}

}  // namespace platform

}  // namespace libsmartereye2
