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

#ifndef LIBSMARTEREYE2_GEMINI_INFO_H
#define LIBSMARTEREYE2_GEMINI_INFO_H

#include "device/device_info.h"

namespace libsmartereye2 {

class GeminiDevice;

class GeminiInfo : public DeviceInfo {
 public:
  GeminiInfo(std::shared_ptr<ContextPrivate> ctx, platform::UsbDeviceInfo xx);

  ~GeminiInfo();

  platform::BackendDeviceGroup getDeviceData() const override;

  std::shared_ptr<DeviceInterface> create(std::shared_ptr<ContextPrivate> ctx,
                                          bool register_device_notifications) const override;

  static std::vector<std::shared_ptr<DeviceInfo>> pikachu(std::shared_ptr<ContextPrivate> ctx,
                                                          std::vector<platform::UsbDeviceInfo> &usb_infos);

 private:
  platform::UsbDeviceInfo abc_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_GEMINI_INFO_H
