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

#ifndef LIBSMARTEREYE2_PIPELINE_CONFIG_HPP
#define LIBSMARTEREYE2_PIPELINE_CONFIG_HPP

namespace se2 {

class PipelineProfile;

class SMARTEREYE2_API PipelineConfig {
 public:
  PipelineConfig();

  void enableStream(FrameId frame_id,
                    int index,
                    uint32_t width,
                    uint32_t height,
                    FrameFormat format,
                    uint32_t framerate);

  void enableStream(FrameId frame_id,
                    int width,
                    int height,
                    FrameFormat format = FrameFormat::Any,
                    int framerate = 0);

  void enableStream(FrameId frame_id, int stream_index = -1);
  void enableStream(FrameId frame_id, FrameFormat format, int framerate = 0);
  void enableStream(FrameId frame_id, int stream_index, FrameFormat format, int framerate = 0);
  void enableAllStreams();

  void enableDevice(const std::string &serial);
  void enableDeviceFromFile(const std::string &file, bool repeat_playback);

  void disableStream(FrameId frame_id, int index = -1);
  void disableAllStreams();

  PipelineProfile resolve(std::shared_ptr<SePipeline> pipeline) const;
  bool canResolve(std::shared_ptr<SePipeline> pipeline) const;

  std::shared_ptr<SePipelineConfig> get() const { return config_; }
  explicit operator std::shared_ptr<SePipelineConfig>() const {
    return config_;
  }

 private:
  std::shared_ptr<SePipelineConfig> config_;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_PIPELINE_CONFIG_HPP
