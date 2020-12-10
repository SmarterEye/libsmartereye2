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

#ifndef LIBSMARTEREYE2_FRAME_SET_HPP
#define LIBSMARTEREYE2_FRAME_SET_HPP

#include "frame.hpp"
#include "smartereye2/streaming/stream_types.hpp"

namespace se2 {

class DepthFrame;

class SMARTEREYE2_API FrameSet : public Frame {
 public:
  FrameSet() : size_(0) {}

  explicit FrameSet(const Frame &frame);

  Frame operator[](size_t index) const;

  VideoFrame operator[](FrameId frame_id) const;

  Frame firstOrDefault(FrameId frame_id, FrameFormat format = FrameFormat::Any) const;

  Frame first(FrameId frame_id, FrameFormat format = FrameFormat::Any) const {
    auto frame = firstOrDefault(frame_id, format);
    if (!frame) {
      throw std::runtime_error("Frame of requested stream type was not found!");
    }
    return frame;
  }

  VideoFrame getVideoFrame(FrameId frame_id) const {
    auto frame = firstOrDefault(frame_id);
    if (!frame) {
      throw std::runtime_error("Frame of requested stream type was not found!");
    }
    return frame.as<VideoFrame>();
  }

  VideoFrame getVideoFrame(int frame_id) const {
    return getVideoFrame(static_cast<FrameId>(frame_id));
  }

  DepthFrame getDepthFrame() const {
    auto frame = firstOrDefault(FrameId::Disparity, FrameFormat::Disparity16);
    return frame.as<DepthFrame>();
  }

  size_t size() const { return size_; }

  class iterator : public std::iterator<std::forward_iterator_tag, Frame> {
   public:
    explicit iterator(const FrameSet *owner, size_t index = 0) : index_(index), owner_(owner) {}
    iterator &operator++() {
      ++index_;
      return *this;
    }
    bool operator==(const iterator &other) const { return index_ == other.index_; }
    bool operator!=(const iterator &other) const { return !(*this == other); }

    Frame operator*() { return (*owner_)[index_]; }

   private:
    size_t index_ = 0;
    const FrameSet *owner_;
  };

  iterator begin() const { return iterator(this); }
  iterator end() const { return iterator(this, size()); }

 private:
  size_t size_;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_FRAME_SET_HPP
