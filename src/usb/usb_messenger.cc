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

#include "usb_messenger.h"

#include "usb_handle.h"
#include "easylogging++.h"

namespace libsmartereye2 {
namespace platform {

UsbMessenger::UsbMessenger(const std::shared_ptr<UsbDevice> &device, std::shared_ptr<UsbHandle> handle)
    : device_(device), handle_(handle) {

}
int UsbMessenger::control_transfer(int request_type, int request, int value, int index,
                                   uint8_t *buffer, uint32_t length, uint32_t &transferred, uint32_t timeout_ms) {
  auto status = libusb_control_transfer(handle_->get(),
                                        request_type, request, value, index, buffer, length, timeout_ms);
  if (status < 0) {
    std::string strerr = libusb_error_name(status);
    LOG(WARNING) << "control_transfer returned error, index: " << index << ", error: " << strerr;
    return status;
  }
  transferred = static_cast<uint32_t>(status);
  return LIBUSB_SUCCESS;
}

int UsbMessenger::bulk_transfer(const SeUsbEndpoint &endpoint, uint8_t *buffer,
                                uint32_t length, uint32_t &transferred, uint32_t timeout_ms) {
  int actual_length = 0;
  int sts = 0;
  if (endpoint->getType() == SE2_USB_ENDPOINT_BULK)
    sts = libusb_bulk_transfer(handle_->get(), endpoint->getAddress(), buffer, length, &actual_length, timeout_ms);
  else if (endpoint->getType() == SE2_USB_ENDPOINT_INTERRUPT)
    sts = libusb_interrupt_transfer(handle_->get(), endpoint->getAddress(), buffer, length, &actual_length, timeout_ms);
  else {
    LOG(ERROR) << "Invalid transfer type " << endpoint->getType() << " on endpoint " << endpoint->getAddress();
    return LIBUSB_ERROR_OTHER;
  }

  if (sts < 0) {
    std::string strerr = strerror(errno);
    LOG(WARNING) << "bulk_transfer returned error, endpoint: 0x" << std::hex << int(endpoint->getAddress()) << std::dec
                 << ", error: " << strerr << ", err. num: " << static_cast<int>(errno);
    return sts;
  }
  transferred = actual_length;
  return LIBUSB_SUCCESS;
}

int UsbMessenger::reset_endpoint(const SeUsbEndpoint &endpoint, uint32_t timeout_ms) {
  int ep = endpoint->getAddress();
  auto sts = libusb_clear_halt(handle_->get(), ep);
  if (sts < 0) {
    std::string strerr = strerror(errno);
    LOG(WARNING) << "reset_endpoint returned error, index: " << ep << ", error: " << strerr << ", number: "
                 << static_cast<int>(errno);
    return sts;
  }
  return LIBUSB_SUCCESS;
}

/*
int UsbMessenger::submit_request(const SeUsbRequest &request) {
  auto nr = reinterpret_cast<libusb_transfer *>(request->get_native_request());
  if (nr->dev_handle == nullptr) {
    return LIBUSB_ERROR_INVALID_PARAM;
  }
  auto req = std::dynamic_pointer_cast<UsbRequest>(request);
  req->set_active(true);
  auto sts = libusb_submit_transfer(nr);
  if (sts < 0) {
    req->set_active(false);
    std::string strerr = strerror(errno);
    LOG(WARNING) << "usb_request_queue returned error, endpoint: " << request->get_endpoint()->getAddress()
                 << " error: " << strerr << ", number: " << static_cast<int>(errno);
    return errno;
  }
  return LIBUSB_SUCCESS;
}

int UsbMessenger::cancel_request(const SeUsbRequest &request) {
  auto nr = reinterpret_cast<libusb_transfer *>(request->get_native_request());
  auto sts = libusb_cancel_transfer(nr);
  if (sts < 0 && sts != LIBUSB_ERROR_NOT_FOUND) {
    std::string strerr = strerror(errno);
    LOG(WARNING) << "usb_request_cancel returned error, endpoint: " << (int) request->get_endpoint()->getAddress()
                 << " error: " << strerr << ", number: " << static_cast<int>(errno);
    return errno;
  }
  return LIBUSB_SUCCESS;
}

SeUsbRequest UsbMessenger::create_request(const SeUsbEndpoint& endpoint) {
  auto rv = std::make_shared<UsbRequest>(handle_->get(), endpoint);
  rv->set_shared(rv);
  return rv;
}
 */

}  // namespace platform
}  // namespace libsmartereye2
