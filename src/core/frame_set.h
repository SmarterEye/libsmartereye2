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

#ifndef LIBSMARTEREYE2_FRAME_SET_H
#define LIBSMARTEREYE2_FRAME_SET_H

#include "frame.h"
#include "disparity_frame.h"

namespace libsmartereye2 {

class DepthFrame;

class FrameSet : public Frame {
 public:
  FrameSet() : size_(0) {}
  explicit FrameSet(const Frame &frame) : Frame(frame), size_(0) {}

  Frame operator[](size_t index) const {
    if (index < size()) {
      return Frame(frames_.at(index));
    }
    throw std::runtime_error("Requested index is out of range!");
  }

  Frame firstOrDefault(StreamType type, StreamFormat format = StreamFormat::FORMAT_CUSTOM) const {
    Frame result;
    for (size_t i = 0; i < size(); i++) {
      auto cur_frame = frames_.at(i);
      cur_frame->acquire();
      Frame frame(cur_frame);
      if (!result && type == frame.getProfile().type()
          && format == StreamFormat::FORMAT_CUSTOM || format == frame.getProfile().format()) {
        result = std::move(frame);
      }
    }
    return result;
  }

  Frame first(StreamType type, StreamFormat format = StreamFormat::FORMAT_CUSTOM) const {
    auto frame = firstOrDefault(type, format);
    if (!frame) {
      throw std::runtime_error("Frame of requested stream type was not found!");
    }
    return frame;
  }

  DepthFrame getDepthFrame() const {
    auto frame = firstOrDefault(StreamType::STREAM_DEPTH, StreamFormat::FORMAT_DISPARITY16);
    return frame.as<DepthFrame>();
  }

  VideoFrame getColorFrame() const {
    auto frame = firstOrDefault(StreamType::STREAM_COLOR);
    if (!frame) {
      throw std::runtime_error("Frame of requested stream type was not found!");
    }
    return frame.as<VideoFrame>();
  }

  size_t size() const { return frames_.size(); }

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
  std::vector<FrameInterface *> frames_;
  size_t size_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_SET_H
