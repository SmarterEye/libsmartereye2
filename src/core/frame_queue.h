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

#ifndef LIBSMARTEREYE2_FRAME_QUEUE_H
#define LIBSMARTEREYE2_FRAME_QUEUE_H

#include <memory>
#include "consumer_queue.h"
#include "frame.h"

namespace libsmartereye2 {

class FrameQueue {
 public:
  struct FrameHolder {
    FrameInterface *frame;

    explicit FrameHolder(FrameInterface *f = nullptr) : frame(f) {}
    FrameHolder(FrameHolder &&other) noexcept: frame(other.frame) { other.frame = nullptr; }
    explicit operator FrameInterface *() const { return frame; }
  };
  using SeFrameQueue = ConsumerQueue<FrameHolder>;

  explicit FrameQueue(uint32_t capacity, bool keep_frames = false)
      : capacity_(capacity), keep_(keep_frames) {
  }

  FrameQueue() : FrameQueue(1) {}

  void operator()(Frame frame) const {
    enqueue(std::move(frame));
  }

  void enqueue(Frame frame) const {
    if (keep_) frame.keep();
    FrameHolder holder(frame.frame_ref_);
    queue_->enqueue(std::move(holder));
  }

  Frame waitForFrame(int32_t timeout_ms = 5000) const {
    FrameHolder holder;
    if (!queue_->dequeue(&holder, timeout_ms)) {
      throw std::runtime_error("Frame did not arrive in time!");
    }

    FrameInterface *result = nullptr;
    std::swap(result, holder.frame);
    return Frame(result);
  }

  size_t capacity() const { return capacity_; }

  bool keepFrames() const { return keep_; }

 private:
  std::shared_ptr<SeFrameQueue> queue_;
  size_t capacity_;
  bool keep_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_QUEUE_H
