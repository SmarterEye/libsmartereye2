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

#ifndef LIBSMARTEREYE2_GEMINI_DEVICE_H
#define LIBSMARTEREYE2_GEMINI_DEVICE_H

#include "device/device.h"

namespace libsmartereye2 {

class GeminiSensor;

class GeminiDevice : public DevicePrivate {
 public:
  GeminiDevice(std::shared_ptr<ContextPrivate> ctx, const platform::BackendDeviceGroup &group,
               bool register_device_notifications);

  ~GeminiDevice() override;

  void hardwareReset() override;

  std::vector<TaggedProfile> getProfilesTags() const override { return std::vector<TaggedProfile>(); }
  void tagProfiles(StreamProfiles profiles) const override {}

  std::shared_ptr<GeminiSensor> getGeminiSensor() const { return sensor_; }

 private:
  std::shared_ptr<GeminiSensor> sensor_;

  platform::UsbDeviceInfo usb_info_;
  platform::SeUsbDevice usb_device_;
  platform::SeUsbMessenger usb_messenger_;

  platform::SeUsbEndpoint endpoint_bulk_out_, endpoint_bulk_in_;

  // for sync get
  platform::UsbStatus control_transfer_in(int cmd, int index, platform::UsbCommonPackHead *response, int buffer_size);
  platform::UsbStatus control_transfer_out(int cmd, int index, const platform::UsbCommonPackHead *request, int buffer_size);

  std::mutex bulk_mutex_;
  platform::UsbStatus bulk_request_response(const platform::UsbCommonPackHead &request,
                                            platform::UsbCommonPackHead &response,
                                            size_t max_response_size,
                                            bool assert_success);
  platform::UsbStatus stream_read(platform::UsbCommonPackHead &response);
  platform::UsbStatus stream_write(const platform::UsbCommonPackHead &request);
  platform::UsbStatus reset_usb_endpoint();

  friend class GeminiSensor;
  friend class GeminiSerialPort;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_GEMINI_DEVICE_H
