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

#ifndef LIBSMARTEREYE2_PIPELINE_PROFILE_H
#define LIBSMARTEREYE2_PIPELINE_PROFILE_H

#include <memory>
#include <vector>

#include "streaming/streaming.h"
#include "sensor/sensor.h"

namespace libsmartereye2 {
class PipelineProfilePrivate;
}

struct SePipelineProfile {
  std::shared_ptr<libsmartereye2::PipelineProfilePrivate> profile;
};

namespace libsmartereye2 {

class DeviceInterface;

class MultiStream;

class PipelineProfilePrivate {
 public:
  explicit PipelineProfilePrivate(std::shared_ptr<DeviceInterface> dev, std::string file = "");

  std::shared_ptr<DeviceInterface> getDevice();
  StreamProfiles getActiveStreams() const;

  const std::shared_ptr<MultiStream> multi_stream_;

 private:
  const std::shared_ptr<DeviceInterface> device_;
  std::string to_file_;
};

class MultiStream {
 public:
  MultiStream(const std::shared_ptr<DeviceInterface>& dev);

  void open();
  void close();

  template<class T>
  void start(T callback) {
    for (auto &&sensor : sensors_) {
      sensor->start(callback);
    }
  }
  void stop();

  bool isStreaming() const;
  StreamProfiles getAllProfiles() const { return all_profiles_; }
  std::vector<SensorInterface *> getSensors() const { return sensors_; }

 private:
  StreamProfiles all_profiles_;
  std::vector<SensorInterface *> sensors_;
  std::map<SensorInterface *, StreamProfiles> sensor_to_profiles_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PIPELINE_PROFILE_H
