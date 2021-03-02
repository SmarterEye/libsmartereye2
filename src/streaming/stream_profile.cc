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

#include "stream_profile.h"
#include "streaming/stream_profile.hpp"

namespace libsmartereye2 {

std::shared_ptr<StreamProfileInterface> StreamProfileBase::clone() const {
  auto res = std::make_shared<StreamProfileBase>();
  res->setUniqueId(uniqueId() + 1);
  res->setFrameRate(fps());
  return res;
}

std::shared_ptr<StreamProfileInterface> VideoStreamProfilePrivate::clone() const {
  auto res = std::make_shared<VideoStreamProfilePrivate>();
  res->setUniqueId(uniqueId() + 1);
  res->setDims(width(), height());
  res->setIntrinsics(getIntrinsics());
  res->setFrameRate(fps());
  return res;
}

std::shared_ptr<StreamProfileInterface> MotionStreamProfilePrivate::clone() const {
  auto res = std::make_shared<MotionStreamProfilePrivate>();
  res->setUniqueId(uniqueId() + 1);
  res->setMotionIntrinsics(getMotionIntrinsics());
  res->setFrameRate(fps());
  return res;
}

}  // namespace libsmartereye2

namespace se2 {

StreamProfile::StreamProfile(SeStreamProfile *profile) : profile_(profile) {
  if (profile_ != nullptr && profile_->profile != nullptr) {
    auto stream = profile_->profile;

    setIndex(stream->index());
    setUniqueId(stream->uniqueId());
    setFrameId(stream->frameId());
    setFormat(stream->format());
    setFrameRate(stream->fps());
  }
}

Extrinsics StreamProfile::getExtrinsicsTo(const StreamProfile &to) const {
  return Extrinsics();
}

void StreamProfile::registerExtrinsicsTo(const StreamProfile &to, Extrinsics extrinsics) {

}

VideoStreamProfile::VideoStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp) {
  if (profile_) {
    auto vsp = dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_->profile);
    width_ = vsp->width();
    height_ = vsp->height();
  }
}

StreamProfile VideoStreamProfile::clone(FrameId frame_id, int32_t index, FrameFormat format,
                                        int width, int height, const Intrinsics &intr) const {
  auto sp = profile_->profile->clone();
  sp->setFrameId(frame_id);
  sp->setIndex(index);
  sp->setFormat(format);

  auto vsp = std::dynamic_pointer_cast<libsmartereye2::VideoStreamProfilePrivate>(sp);
  vsp->setDims(width, height);
  vsp->setIntrinsics(intr);

  return StreamProfile(new SeStreamProfile{sp.get()});
}

StereoCalibrationParameters VideoStreamProfile::getStereoCalibParams() const {
  auto vsp = dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_->profile);
  return vsp->getStereoCalibParams();
}

Intrinsics VideoStreamProfile::getIntrinsics() const {
  auto vsp = dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_->profile);
  return vsp->getIntrinsics();
}

MotionStreamProfile::MotionStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp) {

}

MotionDeviceIntrinsics MotionStreamProfile::getMotionIntrinsics() const {
  auto msp = dynamic_cast<libsmartereye2::MotionStreamProfilePrivate *>(profile_->profile);
  return msp->getMotionIntrinsics();
}

}  // namespace se2
