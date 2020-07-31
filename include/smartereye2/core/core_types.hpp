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

#ifndef LIBSMARTEREYE2_CORE_TYPES_HPP
#define LIBSMARTEREYE2_CORE_TYPES_HPP

#include <cstdint>
#include <vector>

using se_time_t = double;
static const size_t kMaxMetadataSize(0xff);

enum class TimestampDomain {
  HARDWARE_CLOCK, /**< Frame timestamp was measured in relation to the camera clock */
  SYSTEM_TIME,    /**< Frame timestamp was measured in relation to the OS system clock */
  GLOBAL_TIME,    /**< Frame timestamp was measured in relation to the camera clock and converted to OS system clock by constantly measure the difference*/
  COUNT
};

enum class OptionKey {
  FRAMES_QUEUE_SIZE,
  STREAM_FILTER,
  STREAM_FORMAT_FILTER,
  STREAM_INDEX_FILTER,
  COLOR_SCHEME,
  MIN_DISTANCE,
  MAX_DISTANCE,
  MOTION_MODULE_TEMPERATURE,
  MOTION_RANGE,
  LASER_POWER,
  // TODO
};

struct OptionRange {
  float min;
  float max;
  float step;
  float def;
};

struct Vertex {
  float x, y, z;
  explicit operator const float *() const { return &x; }
};

struct TextureCoordinate {
  float u, v;
  explicit operator const float *() const { return &u; }
};

enum class FrameMetadataValue {
  None,
  EmbededLine
};

struct FrameExtension {
  uint64_t index = 0;
  se_time_t timestamp = 0;
  TimestampDomain timestamp_domain = TimestampDomain::SYSTEM_TIME;
  bool is_blocking = false; // when running from recording, this bit indicates
  FrameMetadataValue metadata_value = FrameMetadataValue::None;
  std::vector<uint8_t> metadata_blob;
};

#endif //LIBSMARTEREYE2_CORE_TYPES_HPP
