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

#ifndef LIBSMARTEREYE2_PLAYBACK_H
#define LIBSMARTEREYE2_PLAYBACK_H

#include "device/backend.h"
#include "concurrency/dispatcher.h"

namespace libsmartereye2 {
namespace platform {

class PlaybackDeviceWatcher : public DeviceWather {
 public:
  explicit PlaybackDeviceWatcher(int id);
  ~PlaybackDeviceWatcher();

  void start(DeviceChangedCallback callback) override;
  void stop() override;

  void raiseCallback(const BackendDeviceGroup& old, const BackendDeviceGroup& curr);

 private:
  int entity_id_;
  std::atomic<bool> alive_;
  std::recursive_mutex mutex_;
  std::thread callback_thread_;
  Dispatcher dispatcher_;
  DeviceChangedCallback callback_;
};

class Recording;

class PlaybackBackend : public Backend {
 public:
  explicit PlaybackBackend(const std::string &filename, const std::string &section);

  std::shared_ptr<CommandTransfer> createUsbDevice(UsbDeviceInfo info) const override;

  std::vector<UsbDeviceInfo> queryUsbDevices() const override;

  std::shared_ptr<platform::TimeService> createTimeService() const override;

  std::shared_ptr<DeviceWather> createDeviceWatcher() const override;

 private:
  std::shared_ptr<PlaybackDeviceWatcher> device_watcher_;
  std::shared_ptr<Recording> recording_;
};


}  // namespace platform
}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PLAYBACK_H
