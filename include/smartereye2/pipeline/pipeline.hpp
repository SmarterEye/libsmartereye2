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

#ifndef LIBSMARTEREYE2_PIPELINE_HPP
#define LIBSMARTEREYE2_PIPELINE_HPP

#include "pipeline_profile.hpp"
#include "smartereye2/se_types.hpp"
#include "smartereye2/device/context.hpp"

namespace se2 {

class FrameSet;
class PipelineConfig;

class SMARTEREYE2_API Pipeline {
 public:
  explicit Pipeline(const Context& context = Context());

  PipelineProfile start();
  PipelineProfile start(const PipelineConfig &config);

  PipelineProfile start(FrameCallbackPtr callback);
  PipelineProfile start(const PipelineConfig &config, FrameCallbackPtr callback);

  void stop(bool force = false);
  void restart(bool force = false);
  bool isConnected() const;
  FrameSet waitForFrames(uint32_t timeout_ms = 1000) const;
  bool pollForFrames(FrameSet *frame_set) const;
  bool tryWaitForFrames(FrameSet *frame_set, uint32_t timeout_ms = 1000) const;
  PipelineProfile getActiveProfile();
  int64_t registerInternalDeviceCallback(DevicesChangedCallbackPtr callback);
  void unregisterDevicesChangedCallback(int64_t cb_id);

  explicit operator std::shared_ptr<SePipeline>() const { return pipeline_; }
  explicit Pipeline(std::shared_ptr<SePipeline> pipeline) : pipeline_(std::move(pipeline)) {}

 private:
  friend class PipelineConfig;

  std::shared_ptr<SePipeline> pipeline_;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_PIPELINE_HPP
