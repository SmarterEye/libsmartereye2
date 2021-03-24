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

#ifdef __clang__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#endif

#include <map>

#ifndef FRAMEID_Q_ENUM
namespace se2 {
#endif

// 0-15 bytes sync with device, only for video frame now
// 16+ bytes for algorithm output(device's frameId does not contain)

#ifdef FRAMEID_Q_ENUM
enum Enumeration {
#else
enum class FrameId : uint32_t {
#endif
  NotUsed = 0,
  LeftCamera = 1 << 0,
  RightCamera = 1 << 1,
  CalibLeftCamera = 1 << 2,
  CalibRightCamera = 1 << 3,
  Disparity = 1 << 6,
  LDownSample = 1 << 12,
  RDownSample = 1 << 13,
  Lane = 1 << 16,
  Obstacle = 1 << 17,
  FreeSpace = 1 << 18,
  TrafficSign = 1 << 19,
  TrafficLight = 1 << 20,
  J2Perception = 1 << 21,
  SmallObstacle = 1 << 22,
};
#ifdef FRAMEID_Q_ENUM
Q_ENUM(Enumeration)
#else

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
  YUV422Planar = 4,
  Custom = 65,
  Disparity7 = 512,
  Disparity8,
  Disparity10,
  Disparity12,
  DisparitySparse,
  Disparity16,
  DefaultFormat = Gray,
};

static const std::map<FrameFormat, int> kFrameFormat2bpp = {
    {FrameFormat::Gray, 1},
    {FrameFormat::Color, 3},
    {FrameFormat::YUV422, 2},
    {FrameFormat::YUV422Planar, 2},
    {FrameFormat::Disparity16, 2}
};

static int getBppByFormat(FrameFormat format) {
  if (kFrameFormat2bpp.find(format) != kFrameFormat2bpp.end()) {
    return kFrameFormat2bpp.at(format);
  }
  return 0;
}

#endif

#ifndef FRAMEID_Q_ENUM
}  // namespace se2
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif //LIBSMARTEREYE2_STREAM_TYPES_HPP
