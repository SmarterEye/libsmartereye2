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

#include "se_global.hpp"
#include "stream_types.hpp"
#include "device/device_types.hpp"

namespace se2 {

class SMARTEREYE2_API StreamProfile {
 public:
  StreamProfile() : profile_(nullptr) {}
  explicit StreamProfile(SeStreamProfile *profile);

  explicit operator const SeStreamProfile *() { return profile_.get(); }
  explicit operator bool() const { return profile_ != nullptr; }
  bool operator==(const StreamProfile &rhs) const {
    return index() == rhs.index()
        && frameId() == rhs.frameId()
        && format() == rhs.format()
        && fps() == rhs.fps();
  }

  const SeStreamProfile *get() const { return profile_.get(); }

  int32_t index() const { return index_; }
  int32_t uniqueId() const { return uid_; }
  FrameId frameId() const { return frame_id_; }
  FrameFormat format() const { return format_; }
  uint32_t fps() const { return frame_rete_; }

  void setIndex(int32_t index) { index_ = index; }
  void setUniqueId(int32_t uid) { uid_ = uid; }
  void setFrameId(FrameId frame_id) { frame_id_ = frame_id; }
  void setFormat(FrameFormat format) { format_ = format; }
  void setFrameRate(uint32_t fps) { frame_rete_ = fps; }

  Extrinsics getExtrinsicsTo(const StreamProfile &to) const;
  void registerExtrinsicsTo(const StreamProfile &to, Extrinsics extrinsics);

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

  std::shared_ptr<SeStreamProfile> profile_;

  int index_ = 0;
  int uid_ = 0;
  int frame_rete_ = 0;
  FrameFormat format_ = FrameFormat::Any;
  FrameId frame_id_ = FrameId::NotUsed;
};

class SMARTEREYE2_API VideoStreamProfile : public StreamProfile {
 public:
  explicit VideoStreamProfile(const StreamProfile &sp);
  StreamProfile clone(FrameId frame_id, int32_t index, FrameFormat format,
                      int width, int height, const Intrinsics &intr) const;

  int width() const { return width_; }
  int height() const { return height_; }
  Intrinsics getIntrinsics() const;

 private:
  int width_;
  int height_;
};

class SMARTEREYE2_API MotionStreamProfile : public StreamProfile {
 public:
  explicit MotionStreamProfile(const StreamProfile &sp);
  MotionDeviceIntrinsics getMotionIntrinsics() const;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_STREAM_PROFILE_HPP
