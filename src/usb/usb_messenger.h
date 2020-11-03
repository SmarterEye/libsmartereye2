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

#ifndef LIBSMARTEREYE2_USB_MESSENGER_H
#define LIBSMARTEREYE2_USB_MESSENGER_H

#include "usb_types.h"
#include "usb_endpoint.h"
#include "usb_request.h"

namespace libsmartereye2 {
namespace platform {

class UsbDevice;
class UsbHandle;

class UsbMessenger {
 public:
  UsbMessenger(const std::shared_ptr<UsbDevice> &device, std::shared_ptr<UsbHandle> handle);

  int control_transfer(int request_type, int request, int value, int index,
                       uint8_t *buffer, uint32_t length, uint32_t &transferred, uint32_t timeout_ms);

  int bulk_transfer(const SeUsbEndpoint &endpoint, uint8_t *buffer,
                    uint32_t length, uint32_t &transferred, uint32_t timeout_ms);

  int reset_endpoint(const SeUsbEndpoint &endpoint, uint32_t timeout_ms);

 private:
  const std::shared_ptr<UsbDevice> device_;
  std::mutex mutex_;
  std::shared_ptr<UsbHandle> handle_;
};

using SeUsbMessenger = std::shared_ptr<UsbMessenger>;

}  // namespace platform
}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_USB_MESSENGER_H
