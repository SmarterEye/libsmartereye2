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

#ifndef LIBSMARTEREYE2_STREAM_PROFILE_HPP
#define LIBSMARTEREYE2_STREAM_PROFILE_HPP

#include "stream_types.hpp"
#include "smartereye2/se_global.hpp"
#include "smartereye2/device/device_types.hpp"

namespace libsmartereye2 {
class StreamProfileInterface;
}

namespace se2 {

class SMARTEREYE2_API StreamProfile {
 public:
  StreamProfile() : profile_(nullptr) {}
  explicit StreamProfile(SeStreamProfile *profile);

  explicit operator bool() const { return profile_ != nullptr; }
  bool operator==(const StreamProfile &rhs) const {
    return index() == rhs.index()
        && frameId() == rhs.frameId()
        && format() == rhs.format()
        && fps() == rhs.fps();
  }

  int32_t index() const;
  int32_t uniqueId() const;
  FrameId frameId() const;
  FrameFormat format() const;
  uint32_t fps() const;
  Extrinsics getExtrinsics() const;

  template<typename T>
  bool is() const {
    T extension(*this);
    return extension;
  }

  template<typename T>
  T as() const {
    T extension(*this);
    return extension;
  }

 private:
  friend class Sensor;
  friend class Frame;
  friend class PipelineProfile;
  friend class VideoStreamProfile;
  friend class MotionStreamProfile;

  libsmartereye2::StreamProfileInterface *profile_;
};

class SMARTEREYE2_API VideoStreamProfile : public StreamProfile {
 public:
  explicit VideoStreamProfile(const StreamProfile &sp);

  int width() const;
  int height() const;
  Intrinsics getIntrinsics() const;
};

class SMARTEREYE2_API MotionStreamProfile : public StreamProfile {
 public:
  explicit MotionStreamProfile(const StreamProfile &sp);
  MotionDeviceIntrinsics getMotionIntrinsics() const;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_STREAM_PROFILE_HPP
