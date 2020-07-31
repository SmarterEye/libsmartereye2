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

#ifndef LIBSMARTEREYE2_PIPELINE_CONFIG_H
#define LIBSMARTEREYE2_PIPELINE_CONFIG_H

#include "streaming/streaming.h"
#include "pipeline.h"

namespace libsmartereye2 {
class PipelineConfigPrivate;
}

struct SePipelineConfig {
  std::shared_ptr<libsmartereye2::PipelineConfigPrivate> config;
};

namespace libsmartereye2 {

class PipelineConfigPrivate {
 public:
  PipelineConfigPrivate() = default;

  void enableStream(FrameId frame_id,
                    int index,
                    uint32_t width,
                    uint32_t height,
                    FrameFormat format,
                    uint32_t framerate);

  void enableAllStreams();

  void enableDevice(const std::string &serial);

  void enableDeviceFromFile(const std::string &file, bool repeat_playback);

  void enableRecordToFile(const std::string &file);

  void disableStream(FrameId frame_id, int index = -1);

  void disableAllStreams();

  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<PipelinePrivate> pipe,
                                                  const std::chrono::milliseconds &timeout = std::chrono::milliseconds(0));

  bool canResolve(std::shared_ptr<PipelinePrivate> pipe);

  std::shared_ptr<PipelineProfilePrivate> getCachedResolvedProfile();

  bool getRepeatPlayback() const { return playback_loop_; }

  PipelineConfigPrivate(const PipelineConfigPrivate &other);

 private:
  struct DeviceRequest {
    std::string serial;
    std::string filename;
    std::string record_output;
  };
  std::shared_ptr<DeviceInterface> getOrAddPlaybackDevice(std::shared_ptr<ContextPrivate> ctx,
                                                          const std::string &file);

  std::shared_ptr<DeviceInterface> resolveDeviceRequests(std::shared_ptr<PipelinePrivate> pipe,
                                                         const std::chrono::milliseconds &timeout);

  StreamProfiles getDefaultConfiguration(std::shared_ptr<DeviceInterface> dev);

  std::shared_ptr<PipelineProfilePrivate> resolve(std::shared_ptr<DeviceInterface> dev);

  DeviceRequest device_request_;
  std::map<std::pair<FrameId, int>, StreamProfileConfig> stream_requests_;
  std::mutex mutex_;
  bool enable_all_streams_ = false;
  std::shared_ptr<PipelineProfilePrivate> resolved_profile_;
  bool playback_loop_ = false;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PIPELINE_CONFIG_H
