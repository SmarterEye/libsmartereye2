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

namespace libsmartereye2 {

class PipelineProfilePrivate;
class StreamProfile;
class Context;
class Device;
enum class StreamType;

class PipelineProfile {
 public:
  PipelineProfile() : pipeline_profile_(nullptr) {}
  explicit PipelineProfile(std::shared_ptr<PipelineProfilePrivate> profile) : pipeline_profile_(std::move(profile)) {}

  explicit operator bool() const { return pipeline_profile_ != nullptr; }
  explicit operator std::shared_ptr<PipelineProfilePrivate>() { return pipeline_profile_; }

  std::vector<StreamProfile> getStreams() const;
  StreamProfile getStream(StreamType stream_type, int index = -1) const;
  Device getDevice() const;

 private:
  friend class PipelineConfig;
  friend class Pipeline;

  std::shared_ptr<PipelineProfilePrivate> pipeline_profile_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PIPELINE_PROFILE_H
