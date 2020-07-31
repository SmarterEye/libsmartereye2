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
#include "se_types.hpp"
#include "context/context.hpp"

namespace se2 {

class FrameSet;
class PipelineConfig;

class SMARTEREYE2_API Pipeline {
 public:
  explicit Pipeline(const Context& context = Context());

  PipelineProfile start();
  PipelineProfile start(const PipelineConfig &config);

  template<class T>
  PipelineProfile start(T callback);

  template<class T>
  PipelineProfile start(const PipelineConfig &config, T callback);

  void stop();
  FrameSet waitForFrames(uint32_t timeout_ms = 15000) const;
  bool pollForFrames(FrameSet *frame_set) const;
  bool tryWaitForFrames(FrameSet *frame_set, uint32_t timeout_ms = 15000) const;
  PipelineProfile getActiveProfile();

  operator std::shared_ptr<SePipeline>() const { return pipeline_; }
  explicit Pipeline(std::shared_ptr<SePipeline> pipeline) : pipeline_(std::move(pipeline)) {}

 private:
  friend class PipelineConfig;

  std::shared_ptr<SePipeline> pipeline_;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_PIPELINE_HPP
