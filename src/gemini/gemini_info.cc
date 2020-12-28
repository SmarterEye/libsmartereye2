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

#include "gemini_info.h"

#include <utility>

#include "gemini_device.h"
#include "easylogging++.h"

#define GENIMI_IDVENDOR  0xF525
#define GEMINI_IDPRODUCT 0xC4a0

namespace libsmartereye2 {

GeminiInfo::GeminiInfo(std::shared_ptr<ContextPrivate> ctx, platform::UsbDeviceInfo info)
    : DeviceInfo(std::move(ctx)), usb_device_info_(std::move(info)) {
}

GeminiInfo::~GeminiInfo() = default;

platform::BackendDeviceGroup GeminiInfo::getDeviceData() const {
  return platform::BackendDeviceGroup({usb_device_info_});
}

std::shared_ptr<DeviceInterface> GeminiInfo::create(std::shared_ptr<ContextPrivate> ctx,
                                                    bool register_device_notifications) const {
  LOG(DEBUG) << "GeminiInfo::created " << this;
  return std::make_shared<GeminiDevice>(ctx, getDeviceData(), register_device_notifications);
}

std::vector<std::shared_ptr<DeviceInfo>> GeminiInfo::pickup(const std::shared_ptr<ContextPrivate>& ctx,
                                                            const std::vector<platform::UsbDeviceInfo> &usb_infos) {
  std::vector<std::shared_ptr<DeviceInfo>> matched_list;
  for (const auto &it : usb_infos) {
      if (it.vid == GENIMI_IDVENDOR && it.pid == GEMINI_IDPRODUCT) {
        std::shared_ptr<DeviceInfo> gemini_device(new GeminiInfo(ctx, it));
        matched_list.push_back(gemini_device);
        break;
      }
    }
  return matched_list;
}

}  // namespace libsmartereye2