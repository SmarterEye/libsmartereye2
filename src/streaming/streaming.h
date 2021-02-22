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

#include "device/backend.h"
#include "streaming/stream_types.hpp"

using namespace se2;

namespace libsmartereye2 {

enum ProfileTag {
  PROFILE_TAG_ANY = 0,
  PROFILE_TAG_SUPERSET = 1, // to be included in enable_all
  PROFILE_TAG_DEFAULT = 2,  // to be included in default pipeline start
  PROFILE_TAG_CAPTURE = 4   // tag profile for image capture only
};

struct TaggedProfile {
  FrameId frame_id;
  int stream_index;
  uint32_t width, height;
  FrameFormat format;
  uint32_t fps;
  int tag;
};

struct StreamProfileConfig {
  explicit StreamProfileConfig(FrameFormat format = FrameFormat::Any,
                               FrameId frame_id = FrameId::NotUsed,
                               int idx = 0,
                               uint32_t width = 0, uint32_t height = 0,
                               uint32_t framerate = 0) :
      format(format), frame_id(frame_id), index(idx), width(width), height(height), fps(framerate) {}

  FrameFormat format;
  FrameId frame_id;
  int index;
  uint32_t width, height, fps;

  std::pair<uint32_t, uint32_t> width_height() const { return std::make_pair(width, height); }
};

class StreamInterface : public std::enable_shared_from_this<StreamInterface> {
 public:
  virtual int index() const = 0;
  virtual void setIndex(int index) = 0;

  virtual int uniqueId() const = 0;
  virtual void setUniqueId(int uid) = 0;

  virtual FrameId frameId() const = 0;
  virtual void setFrameId(FrameId frame_id) = 0;
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_STREAMING_H
