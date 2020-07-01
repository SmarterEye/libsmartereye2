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

#ifndef LIBSMARTEREYE2_CONFIG_H
#define LIBSMARTEREYE2_CONFIG_H

#include "se_types.h"
#include "core/streaming.h"

namespace libsmartereye2 {

class PipelineProfile;
class PipelineConfigPrivate;

class PipelineConfig {
 public:
  PipelineConfig();

  void enableStream(StreamType stream_type,
                    int index,
                    uint32_t width,
                    uint32_t height,
                    StreamFormat format,
                    uint32_t framerate);

  void enableStream(StreamType stream_type,
                    int width,
                    int height,
                    StreamFormat format = StreamFormat::FORMAT_CUSTOM,
                    int framerate = 0);

  void enableStream(StreamType stream_type, int stream_index = -1);
  void enableStream(StreamType stream_type, StreamFormat format, int framerate = 0);
  void enableStream(StreamType stream_type, int stream_index, StreamFormat format, int framerate = 0);
  void enableAllStreams();

  void enableDevice(const std::string &serial);
  void enableDeviceFromFile(const std::string &file, bool repeat_playback);

  void disableStream(StreamType stream, int index = -1);
  void disableAllStreams();

  PipelineProfile resolve(std::shared_ptr<PipelinePrivate> pipeline) const;
  bool canResolve(std::shared_ptr<PipelinePrivate> pipeline) const;

  std::shared_ptr<PipelineConfigPrivate> get() const { return config_; }
  explicit operator std::shared_ptr<PipelineConfigPrivate>() const {
    return config_;
  }

 private:
  std::shared_ptr<PipelineConfigPrivate> config_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_CONFIG_H
