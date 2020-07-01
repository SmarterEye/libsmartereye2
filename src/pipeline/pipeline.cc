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

#include <device/device_hub.h>
#include "pipeline.h"
#include "core/stream_profile.h"
#include "core/frame_interface.h"
#include "concurrency/concurrency.h"
#include "pipeline_profile.h"
#include "se_callbacks.h"

namespace libsmartereye2 {

class PipelineConfigPrivate;

class PipelinePrivate : public std::enable_shared_from_this<PipelinePrivate> {
 public:
//  explicit PipelinePrivate(std::shared_ptr<ContextPrivate> context) {
//
//  }

  virtual ~PipelinePrivate() {

  }

  std::shared_ptr<PipelineProfilePrivate> start(std::shared_ptr<PipelineConfig> conf,
                                                FrameCallbackPtr callback = nullptr) {
    return nullptr;
  }

  void stop() {}

  std::shared_ptr<PipelineProfilePrivate> getActiveProfile() const;
  FrameHolder waitForFrames(uint32_t timeout_ms);
  bool pollForFrames(FrameHolder *frame_holder);
  bool tryWaitForFrames(FrameHolder *frame_holder, unsigned int timeout_ms);

  std::shared_ptr<DeviceInterface> waitForDevice(const std::chrono::milliseconds &timeout = std::chrono::hours::max(),
                                                 const std::string &serial = "");
  std::shared_ptr<ContextPrivate> getContext() const;

 protected:
  FrameCallbackPtr getCallback(std::vector<int> unique_ids);
  std::vector<int> onStart(std::shared_ptr<PipelineProfilePrivate> profile);

  void unsafeStart(std::shared_ptr<PipelineConfigPrivate> conf);
  void unsafeStop();

  mutable std::mutex mutex_;
  std::shared_ptr<PipelineProfilePrivate> active_profile_;
  std::shared_ptr<PipelineConfigPrivate> prev_profile_;
  DeviceHubPrivate hub_;

 private:
  std::shared_ptr<PipelineProfilePrivate> unsafeGetActiveProfile() const;

  std::shared_ptr<ContextPrivate> context_;
  int playback_stopped_token_ = -1;
  Dispatcher dispatcher_;
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
