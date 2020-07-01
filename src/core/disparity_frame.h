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

#ifndef LIBSMARTEREYE2_DISPARITYFRAME_H
#define LIBSMARTEREYE2_DISPARITYFRAME_H

#include "frame.h"

namespace libsmartereye2 {

class DepthFrame : public VideoFrame {
 public:
  explicit DepthFrame(const Frame &frame) : VideoFrame(frame) {}

  float distance(int x, int y) const {
    return 0.f;
  }

  float units() const {
    return 0.f;
  }
};

class DisparityFrame : public DepthFrame {
 public:
  explicit DisparityFrame(const Frame &frame) : DepthFrame(frame) {}

  float baseline() const {
    return 0.f;
  }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_DISPARITYFRAME_H
