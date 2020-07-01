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

#ifndef LIBSMARTEREYE2_FILTER_H
#define LIBSMARTEREYE2_FILTER_H

#include "processing.h"
#include "core/frame_queue.h"
#include "core/streaming.h"

namespace libsmartereye2 {

class Frame;
class Points;
class VideoFrame;

class FilterInterface {
 public:
  virtual Frame process(Frame frame) const = 0;
  virtual ~FilterInterface() = default;
};

class Filter : public ProcessingBlock, public FilterInterface {
 public:
  explicit Filter(const std::shared_ptr<SeProcessingBlock> &block, uint32_t queue_size = 1);

  template<class S>
  explicit Filter(S processing_func, uint32_t queue_size = 1) {}

  Frame process(Frame frame) const override;

  FrameQueue getQueue() const;
  SeProcessingBlock *get() const override { return block_.get(); }

  template<class T>
  bool is() const {
    T extension(*this);
    return extension;
  }

  template<class T>
  T as() const {
    T extension(*this);
    return extension;
  }

  explicit operator bool() const { return block_ != nullptr; }

 private:
  FrameQueue queue_;
};

class PointCloud : public Filter {
 public:
  PointCloud();
  explicit PointCloud(StreamType stream, int index = 0);
  explicit PointCloud(const std::shared_ptr<SeProcessingBlock> &block);
  std::shared_ptr<SeProcessingBlock> init();

  Points calculate(Frame depth);
  void mapTo(Frame mapped);
};

class YuvDecoder : public Filter {
 public:
  YuvDecoder();
  explicit YuvDecoder(const std::shared_ptr<SeProcessingBlock> &block);
  std::shared_ptr<SeProcessingBlock> init();
};

class UnitsTransform : public Filter {
 public:
  UnitsTransform();
  explicit UnitsTransform(const std::shared_ptr<SeProcessingBlock> &block);
  std::shared_ptr<SeProcessingBlock> init();
};

class Colorizer : public Filter {
 public:
  Colorizer();
  explicit Colorizer(float color_scheme);
  explicit Colorizer(const std::shared_ptr<SeProcessingBlock> &block);
  std::shared_ptr<SeProcessingBlock> init();

  VideoFrame colorize(Frame depth) const;
};

class DisparityTransform : public Filter {
 public:
  explicit DisparityTransform(bool transform_to_disparity = true);
  explicit DisparityTransform(Filter filter);
  std::shared_ptr<SeProcessingBlock> init(bool transform_to_disparity);

  friend class Context;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FILTER_H
