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

#ifndef LIBSMARTEREYE2_FILTER_HPP
#define LIBSMARTEREYE2_FILTER_HPP

#include <utility>

#include "processing.hpp"
#include "core/frame_queue.hpp"
#include "streaming/stream_types.hpp"

namespace se2 {

class Frame;
class Points;
class VideoFrame;

class FilterInterface {
 public:
  virtual Frame process(Frame frame) const = 0;
  virtual ~FilterInterface() = default;
};

class SMARTEREYE2_API Filter : public ProcessingBlock, public FilterInterface {
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

class SMARTEREYE2_API PointCloud : public Filter {
 public:
  PointCloud();
  explicit PointCloud(FrameId frame_id, int index = 0);
  explicit PointCloud(const std::shared_ptr<SeProcessingBlock> &block);

  Points calculate(Frame depth);
  void mapTo(Frame mapped);

 private:
  friend class Context;

  std::shared_ptr<SeProcessingBlock> init();
};

class SMARTEREYE2_API YuvDecoder : public Filter {
 public:
  YuvDecoder();
  explicit YuvDecoder(const std::shared_ptr<SeProcessingBlock> &block);

 private:
  std::shared_ptr<SeProcessingBlock> init();
};

class SMARTEREYE2_API UnitsTransform : public Filter {
 public:
  UnitsTransform();
  explicit UnitsTransform(const std::shared_ptr<SeProcessingBlock> &block);

 private:
  std::shared_ptr<SeProcessingBlock> init();
};

class SMARTEREYE2_API Colorizer : public Filter {
 public:
  Colorizer();
  explicit Colorizer(float color_scheme);
  explicit Colorizer(const std::shared_ptr<SeProcessingBlock> &block);

  VideoFrame colorize(Frame depth) const;

 private:
  std::shared_ptr<SeProcessingBlock> init();
};

class SMARTEREYE2_API DisparityTransform : public Filter {
 public:
  explicit DisparityTransform(bool transform_to_disparity = true);
  explicit DisparityTransform(Filter filter);

 private:
  friend class Context;

  std::shared_ptr<SeProcessingBlock> init(bool transform_to_disparity);
};

}

#endif //LIBSMARTEREYE2_FILTER_HPP
