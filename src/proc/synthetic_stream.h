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

#ifndef LIBSMARTEREYE2_SYNTHETIC_STREAM_H
#define LIBSMARTEREYE2_SYNTHETIC_STREAM_H

#include "core/frame.h"
#include "se_types.hpp"

namespace libsmartereye2 {
class SyntheticSourceInterface;
}

struct SeSyntheticSource {
  libsmartereye2::SyntheticSourceInterface *source;
};

namespace libsmartereye2 {

class FrameSource;

class SyntheticSourceInterface {
 public:
 public:
  virtual ~SyntheticSourceInterface() = default;

  virtual FrameInterface *allocateVideoFrame(std::shared_ptr<StreamProfileInterface> stream,
                                             FrameInterface *original,
                                             int new_bpp = 0,
                                             int new_width = 0,
                                             int new_height = 0,
                                             int new_stride = 0,
                                             SeExtension frame_type = SeExtension::EXTENSION_VIDEO_FRAME) = 0;

  virtual FrameInterface *allocateMotionFrame(std::shared_ptr<StreamProfileInterface> stream,
                                              FrameInterface *original,
                                              SeExtension frame_type = SeExtension::EXTENSION_MOTION_FRAME) = 0;

  virtual FrameInterface *allocateCompositeFrame(std::vector<FrameHolder> frames) = 0;

  virtual FrameInterface *allocatePoints(std::shared_ptr<StreamProfileInterface> stream,
                                         FrameInterface *original,
                                         SeExtension frame_type = SeExtension::EXTENSION_POINTS) = 0;

  virtual void frameReady(FrameHolder result) = 0;
};

class SyntheticSource : public SyntheticSourceInterface {
 public:
  explicit SyntheticSource(FrameSource &actual);

  FrameInterface *allocateVideoFrame(std::shared_ptr<StreamProfileInterface> stream,
                                     FrameInterface *original,
                                     int new_bpp = 0,
                                     int new_width = 0,
                                     int new_height = 0,
                                     int new_stride = 0,
                                     SeExtension frame_type = SeExtension::EXTENSION_VIDEO_FRAME) override;

  FrameInterface *allocateMotionFrame(std::shared_ptr<StreamProfileInterface> stream,
                                      FrameInterface *original,
                                      SeExtension frame_type = SeExtension::EXTENSION_MOTION_FRAME) override;

  FrameInterface *allocateCompositeFrame(std::vector<FrameHolder> holders) override;

  FrameInterface *allocatePoints(std::shared_ptr<StreamProfileInterface> stream,
                                 FrameInterface *original,
                                 SeExtension frame_type = SeExtension::EXTENSION_POINTS) override;

  void frameReady(FrameHolder result) override;

 private:
  FrameSource &actual_source_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_SYNTHETIC_STREAM_H
