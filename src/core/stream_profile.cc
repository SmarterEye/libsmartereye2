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

namespace libsmartereye2 {

StreamProfile::StreamProfile(StreamProfileBase *profile_base)
    : profile_(profile_base) {}

StreamProfile::operator std::shared_ptr<StreamProfileBase>() {
  return clone_;
}

StreamProfile::operator const StreamProfileBase *() {
  return profile_;
}

StreamProfile::operator bool() const {
  return profile_ != nullptr;
}

bool StreamProfile::operator==(const StreamProfile &rhs) const {
  return index() == rhs.index()
      && type() == rhs.type()
      && format() == rhs.format()
      && fps() == rhs.fps();
}

StreamProfile libsmartereye2::StreamProfile::clone(StreamType type, int32_t index, StreamFormat format) const {
  auto sp = profile_->clone();
  sp->setType(type);
  sp->setIndex(index);
  sp->setFormat(format);
  return StreamProfile(sp.get());
}

VideoStreamProfile::VideoStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp),
      profile_(dynamic_cast<const VideoStreamProfileBase *>(get())) {
}

StreamProfile VideoStreamProfile::clone(StreamType type, int32_t index, StreamFormat format,
                                        int width, int height, const Intrinsics &intr) const {
  return StreamProfile();
}

MotionStreamProfile::MotionStreamProfile(const StreamProfile &sp)
    : StreamProfile(sp),
      profile_(dynamic_cast<const MotionStreamProfileBase *>(get())) {
}

}  // namespace libsmartereye2
