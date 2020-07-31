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

#ifndef LIBSMARTEREYE2_COMMAND_TRANSFER_H
#define LIBSMARTEREYE2_COMMAND_TRANSFER_H

#include <utility>
#include <algorithm>

#include "usb_device.h"

namespace libsmartereye2 {
namespace platform {

class CommandTransfer {
 public:
  virtual std::vector<uint8_t> sendReceive(
      const std::vector<uint8_t> &data,
      int timeout_ms = 5000,
      bool require_response = true) = 0;
};

class UsbCommandTransfer : public CommandTransfer {
 public:
  explicit UsbCommandTransfer(SeUsbDevice device) : device_(std::move(device)) {}
  ~UsbCommandTransfer() = default;

  std::vector<uint8_t> sendReceive(
      const std::vector<uint8_t> &data,
      int timeout_ms,
      bool) override {
    auto intfs = device_->getInterfaces();
    auto it = std::find_if(intfs.begin(), intfs.end(),
                           [](const SeUsbInterface &i) { return i->getClass() == SE2_USB_CLASS_VENDOR_SPECIFIC; });
    if (it == intfs.end())
      throw std::runtime_error("can't find VENDOR_SPECIFIC interface of device: " + device_->getInfo().id);

    auto hwm = *it;

    std::vector<uint8_t> output;
    if (const auto &m = device_->open(hwm->getNumber())) {
      uint32_t transfered_count = 0;
      auto sts = m->bulk_transfer(hwm->firstEndpoint(SE2_USB_ENDPOINT_DIRECTION_WRITE),
                                  const_cast<uint8_t *>(data.data()),
                                  static_cast<uint32_t>(data.size()),
                                  transfered_count,
                                  timeout_ms);

      if (sts != LIBUSB_SUCCESS)
        throw std::runtime_error(
            "command transfer failed to execute bulk transfer, error: " + std::to_string(sts));

      output.resize(DEFAULT_BUFFER_SIZE);
      sts = m->bulk_transfer(hwm->firstEndpoint(SE2_USB_ENDPOINT_DIRECTION_READ),
                             output.data(),
                             static_cast<uint32_t>(output.size()),
                             transfered_count,
                             timeout_ms);

      if (sts != LIBUSB_SUCCESS)
        throw std::runtime_error(
            "command transfer failed to execute bulk transfer, error: " + std::to_string(sts));

      output.resize(transfered_count);
    } else {
      std::stringstream s;
      s << "access failed for " << std::hex << device_->getInfo().vid << ":"
        << device_->getInfo().pid << " uid: " << device_->getInfo().id << std::dec;
      throw std::runtime_error(s.str().c_str());
    }

    return output;
  }

 private:
  SeUsbDevice device_;
  static const uint32_t DEFAULT_BUFFER_SIZE = 1024;
};

}  // namespace platform
}  // namespace libsmartereye2


#endif //LIBSMARTEREYE2_COMMAND_TRANSFER_H
