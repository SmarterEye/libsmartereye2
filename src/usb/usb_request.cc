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

#include "usb_request.h"

#include "easylogging++.h"
#include <utility>

namespace libsmartereye2 {
namespace platform {

void internalCallback(const libusb_transfer *transfer) {
  auto urb = reinterpret_cast<UsbRequest *>(transfer->user_data);
  if (urb) {
    urb->set_active(false);
    auto response = urb->get_shared();
    if (response) {
      auto cb = response->get_callback();
      cb->callback(response);
    }
  }
}

UsbRequest::UsbRequest(libusb_device_handle *dev_handle, SeUsbEndpoint endpoint)
    : endpoint_(std::move(endpoint)) {
  transfer_ = std::shared_ptr<libusb_transfer>(libusb_alloc_transfer(0), [this](libusb_transfer *req) {
    if (!active_)
      libusb_free_transfer(req);
    else
      LOG(ERROR) << "active request didn't return on time";
  });

  if (endpoint_->getType() == SE2_USB_ENDPOINT_BULK) {
    libusb_fill_bulk_transfer(transfer_.get(), dev_handle, endpoint_->getAddress(), nullptr, 0,
                              reinterpret_cast<libusb_transfer_cb_fn>(internalCallback), nullptr, 0);
  } else if (endpoint_->getType() == SE2_USB_ENDPOINT_INTERRUPT) {
    libusb_fill_interrupt_transfer(transfer_.get(), dev_handle, endpoint_->getAddress(), nullptr, 0,
                                   reinterpret_cast<libusb_transfer_cb_fn>(internalCallback), nullptr, 0);
  } else {
    LOG(ERROR) << "Unable to fill a usb request for unknown type " << endpoint_->getType();
  }

  transfer_->user_data = this;
}

UsbRequest::~UsbRequest() {
  if (active_) {
    libusb_cancel_transfer(transfer_.get());
  }
  int attempts = 10;
  while (active_ && attempts--) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void UsbRequest::set_buffer(const std::vector<uint8_t> &buffer) {
  buffer_ = buffer;
  set_native_buffer(buffer_.data());
  set_native_buffer_length(buffer_.size());
}

}  // namespace platform
}  // namespace libsmartereye2
