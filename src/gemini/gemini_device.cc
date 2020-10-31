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

#include "gemini_device.h"
#include "gemini_sensor.h"
#include "usb/usb_foo.h"
#include "easylogging++.h"

#include <utility>

namespace libsmartereye2 {

#define USB_STEREOCAM_CONFIG_VLAUE 1

#define PACK_MAGIC 0xAA55AA55
#define ENDP0_REQUEST_IN     0xA0
#define ENDP0_REQUEST_OUT    0xC0

static const int kUsbTimeout(1000);
static const int kUsbRequestTypeIn(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN);
static const int kUsbRequestTypeOut(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT);

GeminiDevice::GeminiDevice(std::shared_ptr<ContextPrivate> ctx,
                           const platform::BackendDeviceGroup &group,
                           bool register_device_notifications)
    : DevicePrivate(std::move(ctx), group, register_device_notifications) {
  LOG(DEBUG) << "Creating a Gemini device";

  usb_device_ = platform::UsbFoo::createUsbDevcie(group.usb_devices[0]);
  if (!usb_device_) {
    LOG(ERROR) << "Unable to create USB device";
  }

  usb_messenger_ = usb_device_->open(0);
  if (!usb_messenger_) {
    throw std::runtime_error("Unable to open device interface");
  }

  usb_info_ = usb_device_->getInfo();

  const std::vector<platform::SeUsbInterface> interfaces = usb_device_->getInterfaces();
  for (auto &it : interfaces) {
    if (it->getClass() != LIBUSB_CLASS_VENDOR_SPEC) {
      continue;
    }

    auto endpoints = it->getEndpoints();
    for (const auto &endpoint : endpoints) {
      int addr = endpoint->getAddress();
      int type = endpoint->getType();

      if ((type & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
        if (addr & 0x80) {
          endpoint_bulk_in_ = endpoint;
        } else {
          endpoint_bulk_out_ = endpoint;
        }
      }
    }
  }

  if (!endpoint_bulk_in_ || !endpoint_bulk_out_) {
    throw std::runtime_error("Missing usb camera endpoint");
  }

  LOG(DEBUG) << "Successfully opened and claimed interface 0";

  sensor_ = std::make_shared<GeminiSensor>(this);
  addSensor(sensor_);
}

GeminiDevice::~GeminiDevice() {
  LOG(DEBUG) << "Stopping sensor";
  sensor_->dispose();
  LOG(DEBUG) << "Destroying Gemini Device";
}

void GeminiDevice::hardwareReset() {
  LOG(INFO) << "Sending hardware reset";
  control_transfer_out(platform::UsbCommand::RESET_USB_EDP, 0, nullptr, 0);
}

platform::UsbStatus GeminiDevice::control_transfer_in(int cmd,
                                                      int index,
                                                      platform::UsbCommonPackHead *response,
                                                      int buffer_size) {
  if (!response) {
    LOG(ERROR) << "response buffer is empty!";
    return platform::UsbStatus::SE2_USB_STATUS_OTHER;
  }

  uint32_t transferred = 0;
  int ret = usb_messenger_->control_transfer(kUsbRequestTypeIn, ENDP0_REQUEST_IN, cmd, index,
                                             (uint8_t *) response, buffer_size, transferred, kUsbTimeout);
  return static_cast<platform::UsbStatus>(ret);
}

platform::UsbStatus GeminiDevice::control_transfer_out(int cmd,
                                                       int index,
                                                       const platform::UsbCommonPackHead *request,
                                                       int buffer_size) {
  uint32_t transferred = 0;
  int ret = usb_messenger_->control_transfer(kUsbRequestTypeOut, ENDP0_REQUEST_OUT, cmd, index,
                                             (uint8_t *) request, buffer_size, transferred, kUsbTimeout);
  return static_cast<platform::UsbStatus>(ret);
}

platform::UsbStatus GeminiDevice::bulk_request_response(const platform::UsbCommonPackHead &request,
                                                        platform::UsbCommonPackHead &response,
                                                        size_t max_response_size,
                                                        bool assert_success) {
  std::lock_guard<std::mutex> lock(bulk_mutex_);

  // request
  uint32_t length = request.pack_length;
  uint16_t cmd = request.cmd;
  LOG(DEBUG) << "Sending cmd " << cmd << " length " << length;

  uint32_t transferred = 0;
  int e;
  e = usb_messenger_->bulk_transfer(endpoint_bulk_out_, (uint8_t *) &request, length, transferred, kUsbTimeout);
  if (e != platform::SE2_USB_STATUS_SUCCESS) {
    LOG(ERROR) << "Bulk request error " << platform::kUsbStatus2String.at(e);
    return static_cast<platform::UsbStatus>(e);
  }
  if (transferred != length) {
    LOG(ERROR) << "error: sent " << transferred << " not " << length;
    return platform::SE2_USB_STATUS_OTHER;
  }

  // response
  if (max_response_size == 0) {
    max_response_size = sizeof(response);
  }
  LOG(DEBUG) << "Receiving cmd with max_response_size " << max_response_size;

  transferred = 0;
  e = usb_messenger_->bulk_transfer(endpoint_bulk_in_,
                                    (uint8_t *) &response,
                                    int(max_response_size),
                                    transferred,
                                    kUsbTimeout);
  if (e != platform::SE2_USB_STATUS_SUCCESS) {
    LOG(ERROR) << "Bulk response error " << platform::kUsbStatus2String.at(e);
    return static_cast<platform::UsbStatus>(e);
  }
  if (transferred != response.pack_length) {
    LOG(ERROR) <<
               "Received " << transferred << " but header was " << response.pack_length
               << " bytes (max_response_size was "
               << max_response_size << ")";
    return platform::SE2_USB_STATUS_OTHER;
  }
  if (assert_success && response.state != platform::UsbMessageError::MESSAGE_SUCCESS) {
    LOG(ERROR) << "Received " << (response.cmd) << " with length " << response.pack_length
               << " but got non-zero status of " << response.state;
  }
  LOG(DEBUG) << "Received " << (response.cmd) << " with length " << response.pack_length;
  return static_cast<platform::UsbStatus>(e);
}

platform::UsbStatus GeminiDevice::stream_write(const platform::UsbCommonPackHead &request) {
  std::lock_guard<std::mutex> lock(bulk_mutex_);

  uint32_t length = request.pack_length;
  uint16_t cmd = request.cmd;
  LOG(DEBUG) << "Sending stream cmd " << cmd << " length " << length;
  uint32_t transferred = 0;
  int e;
  e = usb_messenger_->bulk_transfer(endpoint_bulk_out_, (uint8_t *) &request, length, transferred, kUsbTimeout);
  if (e != platform::SE2_USB_STATUS_SUCCESS) {
    LOG(ERROR) << "Stream write error " << platform::kUsbStatus2String.at(e);
    return static_cast<platform::UsbStatus>(e);
  }
  if (transferred != length) {
    LOG(ERROR) << "error: sent " << transferred << " not " << length;
    return platform::SE2_USB_STATUS_OTHER;
  }
  return static_cast<platform::UsbStatus>(e);
}

platform::UsbStatus GeminiDevice::stream_read(platform::UsbCommonPackHead &response) {
  std::lock_guard<std::mutex> lock(bulk_mutex_);

  int e = -1;
  uint32_t transferred = 0;
  int read_length = 0;
  auto *img_buf = (uint8_t *) &response;

  // control out for reading frame
  e = control_transfer_out(platform::UsbCommand::GET_FRAME, 0, nullptr, 0);
  if (e != platform::SE2_USB_STATUS_SUCCESS) {
    LOG(ERROR) << "Stream read 0 error " << platform::kUsbStatus2String.at(e);
    return static_cast<platform::UsbStatus>(e);
  }

  // drop when blocking over 80ms
  e = usb_messenger_->bulk_transfer(endpoint_bulk_in_, img_buf, 1024, transferred, 80);
  if (e != platform::SE2_USB_STATUS_SUCCESS) {
    LOG(ERROR) << "Stream read 1 error " << platform::kUsbStatus2String.at(e);
    return static_cast<platform::UsbStatus>(e);
  }

  // head size: UsbCommonPackHead + uint64_t(timestamp)
  if (transferred < sizeof(platform::UsbCommonPackHead) + sizeof(uint64_t)) {
    e = reset_usb_endpoint();
    if (e != platform::SE2_USB_STATUS_SUCCESS) {
      return static_cast<platform::UsbStatus>(e);
    }
  }

  if (response.magic != PACK_MAGIC) {
    e = reset_usb_endpoint();
    if (e != platform::SE2_USB_STATUS_SUCCESS) {
      return static_cast<platform::UsbStatus>(e);
    }
  }

  auto read_size = response.pack_length - transferred;
  img_buf += transferred;

  e = usb_messenger_->bulk_transfer(endpoint_bulk_in_, img_buf, read_size, transferred, kUsbTimeout);
  if (e == LIBUSB_SUCCESS) {
    while (transferred < read_size) {
      read_length += transferred;
      read_size -= transferred;
      img_buf += transferred;

      e = usb_messenger_->bulk_transfer(endpoint_bulk_in_, img_buf, read_size, transferred, kUsbTimeout);
      if (e < 0) {
        LOG(ERROR) << "Stream read 2 error " << platform::kUsbStatus2String.at(e);
        break;
      }
    }
  } else {
    LOG(ERROR) << "Stream read 2 error " << platform::kUsbStatus2String.at(e);
  }

  return static_cast<platform::UsbStatus>(e);
}

platform::UsbStatus GeminiDevice::reset_usb_endpoint() {
  int ret;
  unsigned char buf[16];
  auto *comm_pack = (platform::UsbCommonPackHead *) buf;
  ret = control_transfer_in(platform::UsbCommand::RESET_USB_EDP, 0, comm_pack, sizeof(buf));
  if (ret < 0) {
    return platform::UsbStatus::SE2_USB_STATUS_OTHER;
  }
  return static_cast<platform::UsbStatus>(comm_pack->state);
}

}  // namespace libsmartereye2