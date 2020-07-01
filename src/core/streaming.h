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

#ifndef LIBSMARTEREYE2_STREAMING_H
#define LIBSMARTEREYE2_STREAMING_H

#include <memory>
#include <tuple>

#include "device/motion.h"
#include "se_types.h"
#include "se_util.h"

namespace libsmartereye2 {

enum class StreamType {
  STREAM_ANY,
  STREAM_DEPTH,
  STREAM_COLOR,
  STREAM_INFRARED,
  STREAM_COUNT
  // TODO
};

enum class StreamFormat {
  FORMAT_CUSTOM,
  FORMAT_GRAY,
  FORMAT_RGB8,
  FORMAT_BGR8,
  FORMAT_YUV422,
  FORMAT_YUV422P,
  FORMAT_DISPARITY16,
  FORMAT_DISPARITY32,
  FORMAT_MOTION_RAW,
  FORMAT_MOTION_XYZ32F
  // TODO
};

struct StreamBackendProfile {
  uint32_t width;
  uint32_t height;
  uint32_t fps;
  uint32_t format;

  using ProfileTuple = std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>;
  explicit operator ProfileTuple() const {
    return std::make_tuple(width, height, fps, format);
  }

  bool operator==(const StreamBackendProfile &rhs) const {
    return (this->width == rhs.width) &&
        (this->height == rhs.height) &&
        (this->fps == rhs.fps) &&
        (this->format == rhs.format);
  }
};

class StreamInterface {
 public:
  virtual int index() const = 0;
  virtual void setIndex(int index) = 0;

  virtual int uniqueId() const = 0;
  virtual void setUniqueId(int uid) = 0;

  virtual StreamType type() const = 0;
  virtual void setType(StreamType type) = 0;
};

class StreamProfileBase : public StreamInterface, public std::enable_shared_from_this<StreamProfileBase> {
 public:
  explicit StreamProfileBase(StreamBackendProfile backend_profile);

  int32_t index() const override { return index_; }
  int32_t uniqueId() const override { return uid_; }
  StreamType type() const override { return type_; }
  StreamFormat format() const { return format_; }
  uint32_t fps() const { return framerate_; }

  void setIndex(int32_t index) override { index_ = index; }
  void setUniqueId(int32_t uid) override { uid_ = uid; }
  void setType(StreamType type) override { type_ = type; }
  void setFormat(StreamFormat format) { format_ = format; }
  void setFrameRate(uint32_t fps) { framerate_ = fps; }

  virtual std::shared_ptr<StreamProfileBase> clone() const;

 private:
  int32_t index_ = 1;
  int32_t uid_ = 0;
  uint32_t framerate_ = 0;
  StreamType type_ = StreamType::STREAM_ANY;
  StreamFormat format_ = StreamFormat::FORMAT_CUSTOM;
  StreamBackendProfile backend_profile_;
};

using StreamProfiles = std::vector<std::shared_ptr<StreamProfileBase>>;

class VideoStreamProfileBase : public StreamProfileBase {
 public:
  explicit VideoStreamProfileBase(StreamBackendProfile backend_profile);

  int32_t width() const { return width_; }
  int32_t height() const { return height_; }

  void setDims(int32_t width, int32_t height) {
    width_ = width;
    height_ = height;
  }

  Intrinsics getIntrinsics() const { return intrinsics_; }
  void setIntrinsics(const Intrinsics &intrinsics) { intrinsics_ = intrinsics; }

  std::shared_ptr<StreamProfileBase> clone() const override;

 private:
  int width_ = 0;
  int height_ = 0;
  Intrinsics intrinsics_{};
};

class MotionStreamProfileBase : public StreamProfileBase {
 public:
  explicit MotionStreamProfileBase(StreamBackendProfile backend_profile);

  MotionDeviceIntrinsics getMotionIntrinsics() const { return intrinsics_; }
  void setMotionIntrinsics(const MotionDeviceIntrinsics &intrinsics) { intrinsics_ = intrinsics; }

  std::shared_ptr<StreamProfileBase> clone() const override;

 private:
  MotionDeviceIntrinsics intrinsics_{};
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_STREAMING_H
