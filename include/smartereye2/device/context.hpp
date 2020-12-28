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

#ifndef LIBSMARTEREYE2_CONTEXT_HPP
#define LIBSMARTEREYE2_CONTEXT_HPP

#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include <map>
#include <mutex>

#include "smartereye2/se_types.hpp"
#include "smartereye2/se_global.hpp"
#include "smartereye2/proc/filter.hpp"
#include "smartereye2/device/device_list.hpp"

namespace se2 {

class Device;
class DeviceList;
class Sensor;

class SMARTEREYE2_API DeviceChangedEvent {
 public:
  DeviceChangedEvent(DeviceList removed, DeviceList added)
      : removed_(std::move(removed)), added_(std::move(added)) {}

  bool wasRemoved(const Device &dev) const;
  bool wasAdded(const Device &dev) const;

  DeviceList getNewDevices() const { return added_; }

 private:
  DeviceList removed_;
  DeviceList added_;
};

using DeviceChangeFunction = std::function<void(se2::DeviceChangedEvent &info)>;
class SMARTEREYE2_API DevicesChangedCallback : public SeDevicesChangedCallback {
 public:
  explicit DevicesChangedCallback(DeviceChangeFunction callback);
  void onDevicesChanged(SeDeviceList *removed, SeDeviceList *added) override;
  void release() override { delete this; }

 private:
  DeviceChangeFunction callback_;
};

class SMARTEREYE2_API Context : public std::enable_shared_from_this<Context> {
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
  friend class Pipeline;
  friend class DeviceHub;

  std::shared_ptr<SeContext> context_;
};

}  // namespace se2

#endif  // LIBSMARTEREYE2_CONTEXT_HPP
