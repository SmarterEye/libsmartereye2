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

#include "playback.h"

#include "easylogging++.h"

namespace libsmartereye2 {
namespace platform {

PlaybackDeviceWatcher::PlaybackDeviceWatcher(int id)
    : entity_id_(id), alive_(false), dispatcher_(10) {

}

PlaybackDeviceWatcher::~PlaybackDeviceWatcher() {
  stop();
}

void PlaybackDeviceWatcher::start(DeviceChangedCallback callback) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  stop();
  dispatcher_.start();
  callback_ = callback;
  alive_ = true;
}

void PlaybackDeviceWatcher::stop() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (alive_) {
    alive_ = false;
    dispatcher_.stop();
  }
}

void PlaybackDeviceWatcher::raiseCallback(const BackendDeviceGroup &old, const BackendDeviceGroup &curr) {
  dispatcher_.invoke([=](Dispatcher::CancellableTimer timer) {
    callback_(old, curr);
  });
}

PlaybackBackend::PlaybackBackend(const std::string &filename, const std::string &section)
    : device_watcher_(new PlaybackDeviceWatcher(0)) {
  LOG(DEBUG) << "Starting section " << section;
}

std::shared_ptr<CommandTransfer> PlaybackBackend::createUsbDevice(UsbDeviceInfo info) const {
  return nullptr;
}

std::vector<UsbDeviceInfo> PlaybackBackend::queryUsbDevices() const {
  return std::vector<UsbDeviceInfo>();
}
std::shared_ptr<platform::TimeService> PlaybackBackend::createTimeService() const {
  return std::shared_ptr<platform::TimeService>();
}
std::shared_ptr<DeviceWather> PlaybackBackend::createDeviceWatcher() const {
  return std::shared_ptr<DeviceWather>();
}

}  // namespace platform
}  // namespace libsmartereye2
