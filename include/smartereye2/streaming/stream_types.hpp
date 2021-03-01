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

#ifndef LIBSMARTEREYE2_STREAM_TYPES_HPP
#define LIBSMARTEREYE2_STREAM_TYPES_HPP

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <map>

namespace se2 {

enum class FrameId : int {
  NotUsed = 0,
  LeftCamera = 1 << 0,
  RightCamera = 1 << 1,
  CalibLeftCamera = 1 << 2,
  CalibRightCamera = 1 << 3,
  DisparityDSBak = 1 << 4,
  FreeSpace = 1 << 5,
  Disparity = 1 << 6,
  DisparityPlus = 1 << 7,
  DisparityDS = 1 << 8,
  Lane = 1 << 9,
  Obstacle = 1 << 10,
  Compound = 1 << 11,
  LDownSample = 1 << 12,
  RDownSample = 1 << 13,
  J2Perception = 1 << 14,
  SmallObstacle = 1 << 15,
};

constexpr FrameId operator|(const FrameId self_value, const FrameId in_value) {
  return static_cast<FrameId>(static_cast<uint32_t>(self_value) | static_cast<uint32_t>(in_value));
}

constexpr uint32_t operator&(const enum FrameId self_value, const enum FrameId in_value) {
  return static_cast<uint32_t>(self_value) & static_cast<uint32_t>(in_value);
}

enum class FrameFormat : int {
  Any = -1,
  Gray = 0,
  Color = 1,
  YUV422 = 2,
  RGB565 = 3,
  YUV422Plannar = 4,
  Custom = 65,
  Disparity7 = 512,
  Disparity8,
  Disparity10,
  Disparity12,
  DisparitySparse,
  Disparity16,
  DefaultFormat = Gray,
};

static const std::map<FrameFormat, int> kFrameFormate2bpp = {
    {FrameFormat::Gray, 1},
    {FrameFormat::Color, 3},
    {FrameFormat::YUV422, 2},
    {FrameFormat::YUV422Plannar, 2},
    {FrameFormat::Disparity16, 2}
};

static int getBppByFormat(FrameFormat format) {
  if (kFrameFormate2bpp.find(format) != kFrameFormate2bpp.end()) {
    return kFrameFormate2bpp.at(format);
  }
  return 0;
}

}  // namespace se2

#pragma clang diagnostic pop

#endif //LIBSMARTEREYE2_STREAM_TYPES_HPP
