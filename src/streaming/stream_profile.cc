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

StreamProfile::StreamProfile(SeStreamProfile *profile) : profile_(nullptr) {
  CHECK_PTR_NOT_NULL(profile)
  CHECK_PTR_NOT_NULL(profile->profile)
  profile_ = (profile->profile);
}

int32_t StreamProfile::index() const {
  return profile_->index();
}

int32_t StreamProfile::uniqueId() const {
  return profile_->uniqueId();
}

FrameId StreamProfile::frameId() const {
  return profile_->frameId();
}

FrameFormat StreamProfile::format() const {
  return profile_->format();
}

uint32_t StreamProfile::fps() const {
  return profile_->fps();
}

Extrinsics StreamProfile::getExtrinsics() const {
  return profile_->getExtrinsics();
}

VideoStreamProfile::VideoStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp) {
  auto vsp = dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_);
  CHECK_PTR_NOT_NULL(vsp)
}

int VideoStreamProfile::width() const {
  return dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_)->width();
}

int VideoStreamProfile::height() const {
  return dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_)->height();
}

Intrinsics VideoStreamProfile::getIntrinsics() const {
  return dynamic_cast<libsmartereye2::VideoStreamProfilePrivate *>(profile_)->getIntrinsics();
}

MotionStreamProfile::MotionStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp) {
  auto msp = dynamic_cast<libsmartereye2::MotionStreamProfilePrivate *>(profile_);
  CHECK_PTR_NOT_NULL(msp)
}

MotionDeviceIntrinsics MotionStreamProfile::getMotionIntrinsics() const {
  auto msp = dynamic_cast<libsmartereye2::MotionStreamProfilePrivate *>(profile_);
  return msp->getMotionIntrinsics();
}

}  // namespace se2
