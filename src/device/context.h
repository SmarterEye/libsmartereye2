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
#include <proc/filter.h>

#include "se_types.h"
#include "se_util.h"
#include "se_callbacks.h"

namespace libsmartereye2 {

class DeviceList;
class DeviceInfo;
class StreamProfileBase;

class ContextPrivate : public std::enable_shared_from_this<ContextPrivate> {
 public:
  explicit ContextPrivate(const std::string &filename = "", const std::string &section = "") {}
  virtual ~ContextPrivate() = default;

  void stop() {}
  std::vector<std::shared_ptr<DeviceInfo>> queryDevices(int mask) const;

  int64_t registerInternalDeviceCallback(DevicesChangedCallbackPtr cb);
  void unregisterInternalDeviceCallback(int64_t cb_id);
  void setDevicesChangedCallback(DevicesChangedCallbackPtr cb);

  std::vector<std::shared_ptr<DeviceInfo>> createDevices(int mask) const;

 private:
  DevicesChangedCallbackPtr devices_changed_callback_{};
  std::map<int64_t, DevicesChangedCallbackPtr> devices_changed_callbacks_{};
  std::map<int, std::weak_ptr<const StreamProfileBase>> streams_;
  std::mutex _streams_mutex, _devices_changed_callbacks_mtx;
};

class Device;
class DeviceList;
class Sensor;

class Context : public std::enable_shared_from_this<Context> {
 public:
  Context();
  explicit Context(std::shared_ptr<SeContext> context);
  explicit operator std::shared_ptr<SeContext>() const;

  DeviceList queryDevices() const;
  DeviceList queryDevices(int mask) const;
  std::vector<Sensor> queryAllSensors() const;
  Device getSensorParent(const Sensor &sensor) const;

  template<typename T>
  void setDevicesCahngedCallback(T callback) {}

 protected:
  friend class DeviceHub;

  std::shared_ptr<SeContext> context_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_CONTEXT_H
