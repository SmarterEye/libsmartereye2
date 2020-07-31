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

#include "filter.h"

#include "proc/filter.hpp"
#include "core/frame_set.hpp"

#include <utility>

namespace se2 {

Filter::Filter(const std::shared_ptr<SeProcessingBlock> &block, uint32_t queue_size)
    : ProcessingBlock(block), queue_(queue_size) {
  start(queue_);
}

Frame Filter::process(Frame frame) const {
//  invoke(frame);
  Frame result_frame;
  if (!queue_.pollForFrame(&result_frame)) {
    throw std::runtime_error("Error occured during execution of the processing block! See the log for more info");
  }
  return result_frame;
}

FrameQueue Filter::getQueue() const {
  return queue_;
}

PointCloud::PointCloud() : Filter(init(), 1) {}

PointCloud::PointCloud(FrameId frame_id, int index)
    : Filter(init(), 1) {
  setOption(OptionKey::STREAM_FILTER, static_cast<float>(frame_id));
  setOption(OptionKey::STREAM_INDEX_FILTER, static_cast<float>(index));
}

PointCloud::PointCloud(const std::shared_ptr<SeProcessingBlock> &block)
    : Filter(block, 1) {}

Points PointCloud::calculate(Frame depth) {
  auto res = process(std::move(depth));
  if (res.as<Points>()) {
    return Points(res);
  }

  if (auto frame_set = res.as<FrameSet>()) {
    for (auto frame : frame_set) {
      if (frame.as<Points>()) return Points(frame);
    }
  }

  throw std::runtime_error("Error occured during execution of the processing block! See the log for more info");
}

void PointCloud::mapTo(Frame mapped) {
  setOption(OptionKey::STREAM_FILTER, float(mapped.getProfile().frameId()));
  setOption(OptionKey::STREAM_FORMAT_FILTER, float(mapped.getProfile().format()));
  setOption(OptionKey::STREAM_INDEX_FILTER, float(mapped.getProfile().index()));
  process(std::move(mapped));
}

std::shared_ptr<SeProcessingBlock> PointCloud::init() {
  // TODO
  return std::shared_ptr<SeProcessingBlock>();
}

YuvDecoder::YuvDecoder() : Filter(init(), 1) {}

YuvDecoder::YuvDecoder(const std::shared_ptr<SeProcessingBlock> &block) : Filter(block, 1) {}

std::shared_ptr<SeProcessingBlock> YuvDecoder::init() {
  // TODO
  return std::shared_ptr<SeProcessingBlock>();
}

UnitsTransform::UnitsTransform()
    : Filter(init(), 1) {}

UnitsTransform::UnitsTransform(const std::shared_ptr<SeProcessingBlock> &block)
    : Filter(block, 1) {}

std::shared_ptr<SeProcessingBlock> UnitsTransform::init() {
  // TODO
  return std::shared_ptr<SeProcessingBlock>();
}

Colorizer::Colorizer()
    : Filter(init(), 1) {}

Colorizer::Colorizer(float color_scheme)
    : Filter(init(), 1) {
  setOption(OptionKey::COLOR_SCHEME, color_scheme);
}

Colorizer::Colorizer(const std::shared_ptr<SeProcessingBlock> &block)
    : Filter(block, 1) {}

std::shared_ptr<SeProcessingBlock> Colorizer::init() {
  // TODO
  return std::shared_ptr<SeProcessingBlock>();
}

VideoFrame Colorizer::colorize(Frame depth) const {
  return VideoFrame(process(std::move(depth)));
}

DisparityTransform::DisparityTransform(bool transform_to_disparity)
    : Filter(init(transform_to_disparity), 1) {
}

DisparityTransform::DisparityTransform(Filter filter)
    : Filter(std::move(filter)) {
  // TODO
}

std::shared_ptr<SeProcessingBlock> DisparityTransform::init(bool transform_to_disparity) {
  return std::shared_ptr<SeProcessingBlock>();
}

}  // namespace se2
