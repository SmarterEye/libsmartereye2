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

#ifndef LIBSMARTEREYE2_FRAME_QUEUE_HPP
#define LIBSMARTEREYE2_FRAME_QUEUE_HPP

#include "smartereye2/se_types.hpp"

namespace se2 {

class Frame;

class FrameQueue {
 public:
  explicit FrameQueue(uint32_t capacity, bool keep_frames = false)
      : capacity_(capacity), keep_(keep_frames) {
  }

  FrameQueue() : FrameQueue(1) {}

  void operator()(Frame frame);

  void enqueue(Frame frame);

  Frame waitForFrame(int32_t timeout_ms = 5000) const;

  template<typename T>
  bool pollForFrame(T *output) const {
    return false;
  }

  template<typename T>
  bool tryWaitForFrame(T *output, uint32_t timeout_ms = 5000) const {
    return false;
  }

  size_t capacity() const { return capacity_; }

  bool keepFrames() const { return keep_; }

 private:
  std::shared_ptr<SeFrameQueue> queue_;
  size_t capacity_;
  bool keep_;
};

}  // namespace se2

#endif //LIBSMARTEREYE2_FRAME_QUEUE_HPP
