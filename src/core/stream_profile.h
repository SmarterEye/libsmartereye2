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

#ifndef LIBSMARTEREYE2_STREAM_PROFILE_H
#define LIBSMARTEREYE2_STREAM_PROFILE_H

#include "core/streaming.h"
#include "se_types.h"

namespace libsmartereye2 {

struct Extrinsics;
struct Intrinsics;

class StreamProfile {
 public:
  StreamProfile() : profile_(nullptr) {}
  explicit StreamProfile(StreamProfileBase *profile_base);

  explicit operator std::shared_ptr<StreamProfileBase>();
  explicit operator const StreamProfileBase *();
  explicit operator bool() const;
  bool operator==(const StreamProfile &rhs) const;
  StreamProfile clone(StreamType type, int32_t index, StreamFormat format) const;

  const StreamProfileBase *get() const { return profile_; }
  bool isCloned() const { return clone_ != nullptr; }
  bool isDefault() const { return default_; }

  int32_t index() const { return profile_->index(); }
  int32_t uniqueId() const { return profile_->uniqueId(); }
  StreamType type() const { return profile_->type(); }
  StreamFormat format() const { return profile_->format(); }
  uint32_t fps() const { return profile_->fps(); }

  void setIndex(int32_t index) { profile_->setIndex(index); }
  void setUniqueId(int32_t uid) { profile_->setUniqueId(uid); }
  void setType(StreamType type) { profile_->setType(type); }
  void setFormat(StreamFormat format) { profile_->setFormat(format); }
  void setFrameRate(uint32_t fps) { profile_->setFrameRate(fps); }

  Extrinsics getExtrinsicsTo(const StreamProfile &to) const { return Extrinsics{}; }
  void registerExtrinsicsTo(const StreamProfile &to, Extrinsics extrinsics) {}

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
  friend class VideoStreamProfile;

  StreamProfileBase *profile_;
  std::shared_ptr<StreamProfileBase> clone_;
  bool default_ = false;
};

class VideoStreamProfile : public StreamProfile {
 public:
  explicit VideoStreamProfile(const StreamProfile &sp);
  StreamProfile clone(StreamType type, int32_t index, StreamFormat format,
                      int width, int height, const Intrinsics &intr) const;

  int width() const { return profile_->width(); }
  int height() const { return profile_->height(); }
  Intrinsics getIntrinsics() const { return profile_->getIntrinsics(); }

 private:
  const VideoStreamProfileBase *profile_;
};

class MotionStreamProfile : public StreamProfile {
 public:
  explicit MotionStreamProfile(const StreamProfile &sp);
  MotionDeviceIntrinsics getMotionIntrinsics() const { return profile_->getMotionIntrinsics(); }

 private:
  const MotionStreamProfileBase *profile_;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_STREAM_PROFILE_H
