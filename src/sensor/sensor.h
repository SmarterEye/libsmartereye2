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

#ifndef LIBSMARTEREYE2_SENSOR_H
#define LIBSMARTEREYE2_SENSOR_H
#pragma warning (disable:4250)

#include <memory>
#include <utility>
#include <vector>
#include <atomic>

#include "se_util.hpp"
#include "se_callbacks.hpp"
#include "core/options.h"
#include "core/info.h"
#include "core/frame_source.h"
#include "device/backend.h"

namespace libsmartereye2 {
class SensorInterface;
}

struct SeSensor : public SeOptions, public noncopyable {
  SeSensor(SeDevice *parent, libsmartereye2::SensorInterface *sensor)
      : SeOptions((libsmartereye2::OptionsInterface *) sensor), parent(parent), sensor(sensor) {}

  SeDevice *parent;
  libsmartereye2::SensorInterface *sensor;
};

namespace libsmartereye2 {

class DevicePrivate;
class DeviceInterface;
class StreamProfileInterface;

using StreamProfiles = std::vector<std::shared_ptr<StreamProfileInterface>>;

class SensorInterface : public virtual OptionsInterface, public virtual InfoInterface {
 public:
  virtual StreamProfiles getStreamProfiles(int tag) const = 0;
  virtual StreamProfiles getActiveStreams() const = 0;

  virtual void open(const StreamProfiles &requests) = 0;
  virtual void close() = 0;
  virtual void start(FrameCallbackPtr callback) = 0;
  virtual void stop() = 0;

  virtual FrameCallbackPtr getFramesCallback() const = 0;
  virtual void setFramesCallback(FrameCallbackPtr cb) = 0;
  virtual bool isStreaming() const = 0;

  virtual DeviceInterface &getDevice() = 0;
};

class SensorBase : public virtual SensorInterface, public virtual OptionsContainer, public virtual InfoContainer,
                   public std::enable_shared_from_this<SensorBase> {
 public:
  explicit SensorBase(const std::string &name, DevicePrivate *device);
  ~SensorBase() override { frame_source_->flush(); }

  virtual StreamProfiles initStreamProfiles() = 0;

  StreamProfiles getStreamProfiles(int tag) const override;
  StreamProfiles getActiveStreams() const override;

  FrameCallbackPtr getFramesCallback() const override;
  void setFramesCallback(FrameCallbackPtr cb) override;
  bool isStreaming() const override;

  DeviceInterface &getDevice() override;

  virtual bool isOpened() const { return is_opened_; }

 protected:
  void setActiveStream(const StreamProfiles &requests);

  std::atomic<bool> is_streaming_;
  std::atomic<bool> is_opened_;
  std::shared_ptr<MetadataParserMap> metadata_parsers_ = nullptr;

  std::shared_ptr<FrameSource> frame_source_;
  DevicePrivate *device_owner_;
  StreamProfiles profiles_;

 private:
  StreamProfiles active_profiles_;
  mutable std::mutex active_profiles_mutex_;
};

class VideoSensorInterface {
 public:
  virtual Intrinsics getIntrinsics() const = 0; // request Intrinsics without params
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_SENSOR_H
