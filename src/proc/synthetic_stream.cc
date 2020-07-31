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

#include "easylogging++.h"
#include "synthetic_stream.h"
#include "core/frame.h"
#include "core/frame_data.h"
#include "core/frame_source.h"

namespace libsmartereye2 {

SyntheticSource::SyntheticSource(FrameSource &actual)
    : actual_source_(actual) {}

FrameInterface *SyntheticSource::allocateVideoFrame(std::shared_ptr<StreamProfileInterface> stream,
                                                    FrameInterface *original,
                                                    int new_bpp,
                                                    int new_width,
                                                    int new_height,
                                                    int new_stride,
                                                    SeExtension frame_type) {
  return nullptr;
}

FrameInterface *SyntheticSource::allocateMotionFrame(std::shared_ptr<StreamProfileInterface> stream,
                                                     FrameInterface *original,
                                                     SeExtension frame_type) {
  return nullptr;
}

FrameInterface *SyntheticSource::allocateCompositeFrame(std::vector<FrameHolder> holders) {
  FrameExtension frame_ext{};
  auto req_size = holders.size();
  auto alloc_size = req_size * sizeof(FrameInterface*);

  auto frame_interface = actual_source_.alloc_frame(SeExtension::EXTENSION_COMPOSITE_FRAME, alloc_size, frame_ext, true);
  if (!frame_interface) return nullptr;

  auto composite_frame = dynamic_cast<CompositeFrameData *>(frame_interface);
  composite_frame->setFrameCount(holders.size());

  auto frames = composite_frame->getFrames();
  for (int i = 0; i < holders.size(); i++) {
    frames[i] = nullptr;
    std::swap(frames[i], holders[i].frame);
  }

  return frame_interface;
}

FrameInterface *SyntheticSource::allocatePoints(std::shared_ptr<StreamProfileInterface> stream,
                                                FrameInterface *original,
                                                SeExtension frame_type) {
  // TODO
  return nullptr;
}

void SyntheticSource::frameReady(FrameHolder result) {
  actual_source_.invoke_callback(std::move(result));
}

}  // namespace libsmartereye2
