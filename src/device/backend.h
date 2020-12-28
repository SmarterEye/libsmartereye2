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

#ifndef LIBSMARTEREYE2_BACKEND_H
#define LIBSMARTEREYE2_BACKEND_H

#include <memory>
#include <functional>
#include <utility>
#include <vector>
#include <sstream>

#include "usb/usb_types.h"
#include "usb/command_transfer.h"
#include "concurrency/concurrency.h"
#include "se_common.h"
#include "se_callbacks.hpp"

namespace libsmartereye2 {
namespace platform {

enum class BackendType {
  STANDARD,
  PLAYBACK,
  RECORD
};

template<class T>
bool listChanged(const std::vector<T> &list1,
                 const std::vector<T> &list2,
                 std::function<bool(T, T)> equal = [](T first, T second) { return first == second; }) {
  if (list1.size() != list2.size())
    return true;

  for (const auto &dev1 : list1) {
    bool found = false;
    for (const auto &dev2 : list2) {
      if (equal(dev1, dev2)) {
        found = true;
      }
    }

    if (!found) {
      return true;
    }
  }
  return false;
}

struct BackendProfile {
  uint32_t width;
  uint32_t height;
  uint32_t fps;
  uint32_t format;

  using ProfileTuple = std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>;
  explicit operator ProfileTuple() const {
    return std::make_tuple(width, height, fps, format);
  }

  bool operator==(const BackendProfile &rhs) const {
    return (this->width == rhs.width) &&
        (this->height == rhs.height) &&
        (this->fps == rhs.fps) &&
        (this->format == rhs.format);
  }
};

struct ControlRange {
  ControlRange() = default;
  ControlRange(int32_t in_min, int32_t in_max, int32_t in_step, int32_t in_def);
  ControlRange(std::vector<uint8_t> in_min,
               std::vector<uint8_t> in_max, std::vector<uint8_t> in_step,
               std::vector<uint8_t> in_def);

  std::vector<uint8_t> min;
  std::vector<uint8_t> max;
  std::vector<uint8_t> step;
  std::vector<uint8_t> def;

 private:
  void populateRawData(std::vector<uint8_t> &vec, int32_t value);
};

struct PlaybackDeviceInfo {
  std::string file_path;
  operator std::string() { return file_path; }
};

inline bool operator==(const PlaybackDeviceInfo &a, const PlaybackDeviceInfo &b) {
  return (a.file_path == b.file_path);
}

struct BackendDeviceGroup {
  BackendDeviceGroup() = default;

  explicit BackendDeviceGroup(std::vector<UsbDeviceInfo> usb_devices)
      : usb_devices(std::move(usb_devices)) {}

  explicit BackendDeviceGroup(std::vector<PlaybackDeviceInfo> playback_devices)
      : playback_devices(std::move(playback_devices)) {}

  std::vector<UsbDeviceInfo> usb_devices;
  std::vector<PlaybackDeviceInfo> playback_devices;

  bool operator==(const BackendDeviceGroup &other) const {
    return !listChanged(usb_devices, other.usb_devices) &&
        !listChanged(playback_devices, other.playback_devices);
  }

  explicit operator std::string() {
    std::string s;

    s += !usb_devices.empty() ? "usb devices:\n" : "";
    for (const auto &usb : usb_devices) {
      s += usb;
      s += "\n\n";
    }

    s += !playback_devices.empty() ? "playback devices: \n" : "";
    for (auto playback_device : playback_devices) {
      s += playback_device;
      s += "\n\n";
    }

    return s;
  }
};

typedef std::function<void(BackendDeviceGroup old, BackendDeviceGroup curr)> DeviceChangedCallback;

class DeviceWather {
 public:
  virtual void start(DeviceChangedCallback callback) = 0;
  virtual void stop() = 0;
};

class Backend;

class PollingDeviceWatcher : public DeviceWather {
 public:
  explicit PollingDeviceWatcher(const Backend *backend);
  ~PollingDeviceWatcher();

  void polling(Dispatcher::CancellableTimer timer);

  void start(DeviceChangedCallback callback) override;
  void stop() override;

 private:
  RepeatOperation<> repeat_operation_;

  CallbacksHeap callback_inflight_;
  const Backend *backend_;

  BackendDeviceGroup discovered_devices_;
  DeviceChangedCallback callback_;
};

class Backend {
 public:
  virtual std::shared_ptr<CommandTransfer> createUsbDevice(UsbDeviceInfo info) const = 0;

  virtual std::vector<UsbDeviceInfo> queryUsbDevices() const = 0;

  virtual std::shared_ptr<platform::TimeService> createTimeService() const = 0;

  virtual std::shared_ptr<DeviceWather> createDeviceWatcher() const = 0;

  virtual std::string getDeviceSerial(uint16_t device_vid, uint16_t device_pid, const std::string &device_uid) const {
    std::string empty_str;
    return empty_str;
  }
};

class StandardBackend : public Backend, public std::enable_shared_from_this<StandardBackend> {
 public:
  StandardBackend();
  ~StandardBackend();

  std::shared_ptr<CommandTransfer> createUsbDevice(UsbDeviceInfo info) const override;

  std::vector<UsbDeviceInfo> queryUsbDevices() const override;

  std::shared_ptr<platform::TimeService> createTimeService() const override;

  std::shared_ptr<DeviceWather> createDeviceWatcher() const override;

 private:
  std::chrono::high_resolution_clock::time_point start_time_point_;
};

}  // namespace platform

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_BACKEND_H
