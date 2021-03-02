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

#ifndef LIBSMARTEREYE2_CONTEXT_H
#define LIBSMARTEREYE2_CONTEXT_H

#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <mutex>

#include "backend.h"
#include "proc/filter.hpp"
#include "se_callbacks.hpp"

namespace libsmartereye2 {

class DeviceInfo;
class StreamProfileBase;
class PlaybackDeviceInfo;

class ContextPrivate : public std::enable_shared_from_this<ContextPrivate> {
 public:
  explicit ContextPrivate(platform::BackendType type,
                          const std::string &filename = "",
                          const std::string &section = "");
  virtual ~ContextPrivate();

  void start();
  void stop();
  std::vector<std::shared_ptr<DeviceInfo>> queryDevices(int mask) const;

  int64_t registerDevicesChangedCallback(DevicesChangedCallbackPtr callback);
  void unregisterDevicesChangedCallback(int64_t cb_id);

  std::vector<std::shared_ptr<DeviceInfo>>
  createDevices(platform::BackendDeviceGroup devices,
                const std::map<std::string, std::weak_ptr<DeviceInfo>> &playback_devices,
                int mask) const;

  std::shared_ptr<PlaybackDeviceInfo> addPlaybackDevice(const std::string &file);

 protected:
  void onDeviceChanged(platform::BackendDeviceGroup old, platform::BackendDeviceGroup current,
                       const std::map<std::string, std::weak_ptr<DeviceInfo>> &old_playback_devices,
                       const std::map<std::string, std::weak_ptr<DeviceInfo>> &new_playback_devices);

  void raiseDevicesChanged(const std::vector<SeDeviceInfo> &removed, const std::vector<SeDeviceInfo> &added);

 private:
  std::shared_ptr<platform::Backend> backend_;
  std::shared_ptr<platform::DeviceWather> device_watcher_;
  std::map<std::string, std::weak_ptr<DeviceInfo>> playback_devices_;
  std::map<int64_t, DevicesChangedCallbackPtr, std::greater<>> devices_changed_callbacks_{};
  std::map<int, std::weak_ptr<const StreamProfileBase>> streams_;
  mutable std::mutex streams_mtx_;
  mutable std::mutex devices_changed_callbacks_mtx_;
};

}  // namespace libsmartereye2

struct SeContext {
  std::shared_ptr<libsmartereye2::ContextPrivate> context;
};

#endif  // LIBSMARTEREYE2_CONTEXT_H
