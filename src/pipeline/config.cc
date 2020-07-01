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

#include "config.h"
#include "pipeline_profile.h"
#include "core/streaming.h"
#include "device/device.h"
#include "se_types.h"

namespace libsmartereye2 {

class ConfigPrivate {
 public:
  ConfigPrivate() = default;

  void enableStream(StreamType stream,
                    int index,
                    uint32_t width,
                    uint32_t height,
                    StreamFormat format,
                    int32_t framerate) {
    std::lock_guard<std::mutex> lock()
  }

  void enableAllStreams();
  void enableDevice(const std::string &serial);
  void enableDeviceFromFile(const std::string &file, bool repeat_playback);
  void enableRecordToFile(const std::string &file);
  void disableStream(StreamType stream, int index = -1);
  void disableAllStreams();
  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<PipelinePrivate> pipe,
                                                  const std::chrono::milliseconds &timeout = std::chrono::milliseconds(0));
  bool canResolve(std::shared_ptr<PipelinePrivate> pipe);
  bool getRepeatPlayback();

  //Non top level API
  std::shared_ptr<PipelineProfilePrivate> getCachedResolvedProfile();

  ConfigPrivate(const ConfigPrivate &other) {
    device_request_ = other.device_request_;
    stream_requests_ = other.stream_requests_;
    enable_all_streams_ = other.enable_all_streams_;
    resolved_profile_ = nullptr;
    playback_loop_ = other.playback_loop_;
  }

 private:
  struct device_request {
    std::string serial;
    std::string filename;
    std::string record_output;
  };
  std::shared_ptr<DeviceInterface> get_or_add_playback_device(std::shared_ptr<ContextPrivate> ctx,
                                                              const std::string &file);
  std::shared_ptr<DeviceInterface> resolve_device_requests(std::shared_ptr<PipelinePrivate> pipe,
                                                           const std::chrono::milliseconds &timeout);
  StreamProfiles get_default_configuration(std::shared_ptr<DeviceInterface> dev);
  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<DeviceInterface> dev);

  device_request device_request_;
  std::map<std::pair<StreamType, int>, StreamProfileBase> stream_requests_;
  std::mutex mutex_;
  bool enable_all_streams_ = false;
  std::shared_ptr<PipelineProfilePrivate> resolved_profile_;
  bool playback_loop_;
};

}  // namespace libsmartereye2
