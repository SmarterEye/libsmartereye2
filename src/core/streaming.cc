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

#include "streaming.h"

#include <memory>

namespace libsmartereye2 {

StreamProfileBase::StreamProfileBase(StreamBackendProfile backend_profile)
    : backend_profile_(backend_profile) {

}

std::shared_ptr<StreamProfileBase> StreamProfileBase::clone() const {
  std::shared_ptr<StreamProfileBase> res = std::make_shared<StreamProfileBase>(backend_profile_);
  res->setUniqueId(uid_ + 1);
  res->setFrameRate(fps());
  return res;
}

VideoStreamProfileBase::VideoStreamProfileBase(StreamBackendProfile backend_profile)
    : StreamProfileBase(backend_profile),
      width_(0), height_(0) {

}

std::shared_ptr<StreamProfileBase> VideoStreamProfileBase::clone() const {
  auto res = std::make_shared<VideoStreamProfileBase>(StreamBackendProfile{});
  res->setUniqueId(uniqueId() + 1);
  res->setDims(width(), height());
  res->setIntrinsics(getIntrinsics());
  res->setFrameRate(fps());
  return res;
}

MotionStreamProfileBase::MotionStreamProfileBase(StreamBackendProfile backend_profile)
    : StreamProfileBase(backend_profile) {

}

std::shared_ptr<StreamProfileBase> MotionStreamProfileBase::clone() const {
  auto res = std::make_shared<MotionStreamProfileBase>(StreamBackendProfile{});
  res->setUniqueId(uniqueId() + 1);
  res->setMotionIntrinsics(getMotionIntrinsics());
  res->setFrameRate(fps());
  return res;
}

}  // namespace libsmartereye2
