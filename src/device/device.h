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

#ifndef LIBSMARTEREYE2_DEVICE_H
#define LIBSMARTEREYE2_DEVICE_H

#include <utility>

#include "context.h"
#include "backend.h"
#include "core/info.h"
#include "sensor/sensor.h"
#include "streaming/streaming.h"

#include "se_callbacks.hpp"
#include "device/device_types.hpp"

using namespace libsmartereye2;

namespace libsmartereye2 {
class DeviceInterface;
}

struct SeDevice {
  std::shared_ptr<ContextPrivate> context;
  std::shared_ptr<DeviceInfo> info;
  std::shared_ptr<DeviceInterface> device;
};

struct SeDeviceInfo {
  std::shared_ptr<ContextPrivate> context;
  std::shared_ptr<DeviceInfo> info;
};

struct SeDeviceList {
  std::shared_ptr<ContextPrivate> context;
  std::vector<SeDeviceInfo> list;
};

namespace libsmartereye2 {

class Matcher;
class FrameHolder;
class ContextPrivate;

class DeviceInterface : public virtual InfoInterface, public std::enable_shared_from_this<DeviceInterface> {
 public:
  virtual SensorInterface &getSensor(size_t i) = 0;
  virtual const SensorInterface &getSensor(size_t i) const = 0;
  virtual size_t getSensorCount() const = 0;
  virtual void hardwareReset() = 0;

  virtual std::shared_ptr<ContextPrivate> getContext() const = 0;
  virtual std::pair<int32_t, Extrinsics> getExtrinsics(const StreamInterface &stream) const = 0;
  virtual platform::BackendDeviceGroup getDeviceData() const = 0;

  virtual bool isValid() const = 0;

  virtual std::shared_ptr<Matcher> createMatcher(const FrameHolder &frame) const = 0;

  virtual std::vector<TaggedProfile> getProfilesTags() const = 0;
  virtual void tagProfiles(StreamProfiles profiles) const = 0;
};

class DevicePrivate : public virtual DeviceInterface, public InfoContainer {
 public:
  ~DevicePrivate() override;

  SensorInterface &getSensor(size_t i) override;
  const SensorInterface &getSensor(size_t i) const override;
  size_t getSensorCount() const override;
  void hardwareReset() override;
  std::shared_ptr<ContextPrivate> getContext() const override;
  std::pair<int32_t, Extrinsics> getExtrinsics(const StreamInterface &stream) const override;
  platform::BackendDeviceGroup getDeviceData() const override;
  bool isValid() const override;
  void tagProfiles(StreamProfiles profiles) const override;

  std::shared_ptr<Matcher> createMatcher(const FrameHolder &frame) const override {return nullptr;}

 protected:
  int addSensor(const std::shared_ptr<SensorInterface> &sensor_base);
  int assignSensor(const std::shared_ptr<SensorInterface> &sensor_base, int8_t idx);
  void registerStreamToExtrinsicGroup(const StreamProfileBase &stream, int32_t groupd_index);

  DevicePrivate(std::shared_ptr<ContextPrivate> context,
                platform::BackendDeviceGroup group,
                bool device_changed_notifications = false
  );

  std::map<int, std::pair<int32_t, std::shared_ptr<const StreamInterface>>> extrinsics_;

 private:
  std::vector<std::shared_ptr<SensorInterface>> sensors_;
  std::shared_ptr<ContextPrivate> context_;
  const platform::BackendDeviceGroup group_;
  bool is_valid_;
  bool device_changed_notifications_;
  mutable std::mutex device_changed_mtx_;
  int64_t callback_id_{};
  std::vector<TaggedProfile> profiles_tags_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DEVICE_H
