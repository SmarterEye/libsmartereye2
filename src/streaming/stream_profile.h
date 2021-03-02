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

#include "device/device_types.hpp"
#include "alg/calibrationparams.h"
#include "streaming.h"

namespace libsmartereye2 {
class StreamProfileInterface;
}

struct SeStreamProfile {
  libsmartereye2::StreamProfileInterface *profile;
};

using namespace se2;

namespace libsmartereye2 {

class StreamProfileInterface : public StreamInterface {
 public:
  virtual FrameFormat format() const = 0;
  virtual uint32_t fps() const = 0;
  virtual int tag() const = 0;

  virtual void setFormat(FrameFormat format) = 0;
  virtual void setFrameRate(uint32_t fps) = 0;
  virtual void tagProfile(int tag) = 0;

  virtual std::shared_ptr<StreamProfileInterface> clone() const = 0;
};

class StreamProfileBase : public virtual StreamProfileInterface {
 public:
  int32_t index() const override { return index_; }
  int32_t uniqueId() const override { return uid_; }
  FrameId frameId() const override { return frame_id_; }
  FrameFormat format() const override { return format_; }
  uint32_t fps() const override { return framerate_; }
  int tag() const override { return tag_; }

  void setIndex(int32_t index) override { index_ = index; }
  void setUniqueId(int32_t uid) override { uid_ = uid; }
  void setFrameId(FrameId frame_id) override { frame_id_ = frame_id; }
  void setFormat(FrameFormat format) override { format_ = format; }
  void setFrameRate(uint32_t fps) override { framerate_ = fps; }
  void tagProfile(int tag) override { tag_ = tag; }

  std::shared_ptr<StreamProfileInterface> clone() const override;

 private:
  int32_t index_ = 1;
  int32_t uid_ = 0;
  uint32_t framerate_ = 0;
  FrameId frame_id_ = FrameId::NotUsed;
  FrameFormat format_ = FrameFormat::Any;
  int tag_ = 0;
};

class VideoStreamProfileInterface : public virtual StreamProfileInterface {
 public:
  virtual int32_t width() const = 0;
  virtual int32_t height() const = 0;
  virtual void setDims(int32_t width, int32_t height) = 0;

  virtual StereoCalibrationParameters getStereoCalibParams() const = 0;
  virtual void setStereoCalibParams(const StereoCalibrationParameters &params) = 0;
  virtual Intrinsics getIntrinsics() const = 0;
  virtual void setIntrinsics(const Intrinsics &intrinsics) = 0;
};

class VideoStreamProfilePrivate : public StreamProfileBase, public virtual VideoStreamProfileInterface {
 public:
  int32_t width() const override { return width_; }
  int32_t height() const override { return height_; }

  void setDims(int32_t width, int32_t height) override {
    width_ = width;
    height_ = height;
  }

  StereoCalibrationParameters getStereoCalibParams() const override { return stereo_calib_params_; }
  void setStereoCalibParams(const StereoCalibrationParameters &params) override { stereo_calib_params_ = params; }
  Intrinsics getIntrinsics() const override { return intrinsics_; }
  void setIntrinsics(const Intrinsics &intrinsics) override { intrinsics_ = intrinsics; }

  std::shared_ptr<StreamProfileInterface> clone() const override;

 private:
  int width_ = 0;
  int height_ = 0;
  Intrinsics intrinsics_{};
  StereoCalibrationParameters stereo_calib_params_{};
};

class MotionStreamProfileInterface : public virtual StreamProfileInterface {
 public:
  virtual MotionDeviceIntrinsics getMotionIntrinsics() const = 0;
  virtual void setMotionIntrinsics(const MotionDeviceIntrinsics &intrinsics) = 0;
};

class MotionStreamProfilePrivate : public StreamProfileBase, public virtual MotionStreamProfileInterface {
 public:
  MotionDeviceIntrinsics getMotionIntrinsics() const override { return intrinsics_; }
  void setMotionIntrinsics(const MotionDeviceIntrinsics &intrinsics) override { intrinsics_ = intrinsics; }

  std::shared_ptr<StreamProfileInterface> clone() const override;

 private:
  MotionDeviceIntrinsics intrinsics_{};
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_STREAM_PROFILE_H
