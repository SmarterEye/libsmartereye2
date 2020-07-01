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

#include "pipeline.h"
#include "core/stream_profile.h"
#include "core/frame_interface.h"
#include "device/device.h"
#include "pipeline_profile.h"
#include "se_callbacks.h"

namespace libsmartereye2 {

class PipelinePrivate : public std::enable_shared_from_this<PipelinePrivate> {
 public:
  explicit PipelinePrivate(std::shared_ptr<ContextPrivate> context) {

  }
  virtual ~PipelinePrivate() {

  }

  std::shared_ptr<PipelineProfilePrivate> start(std::shared_ptr<Config> conf, FrameCallbackPtr callback = nullptr) {

  }

  void stop() {}
  std::shared_ptr<PipelineProfilePrivate> get_active_profile() const;
  FrameHolder wait_for_frames(unsigned int timeout_ms);
  bool poll_for_frames(FrameHolder *frame_holder);
  bool try_wait_for_frames(FrameHolder *frame_holder, unsigned int timeout_ms);

 private:

};

std::vector<StreamProfile> PipelineProfile::getStreams() const {
  return std::vector<StreamProfile>();
}

StreamProfile PipelineProfile::getStream(StreamType stream_type, int index) const {
  return StreamProfile();
}

Device PipelineProfile::getDevice() const {
  return Device();
}

}  // namespace libsmartereye2
