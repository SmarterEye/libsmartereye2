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

#include "frame_queue.h"

#include "core/frame_queue.hpp"
#include "core/frame.hpp"

using namespace libsmartereye2;

namespace se2 {

void FrameQueue::operator()(Frame frame) {
  enqueue(std::move(frame));
}

void FrameQueue::enqueue(Frame frame) {
  if (keep_) frame.keep();
  FrameHolder fh(frame.frame_ref_);
  queue_->queue.enqueue(std::move(fh));
}

Frame FrameQueue::waitForFrame(int32_t timeout_ms) const {
  FrameHolder fh;
  if (!queue_->queue.dequeue(&fh, timeout_ms)) {
    throw std::runtime_error("Frame did not arrive in time!");
  }

  FrameInterface *frame = nullptr;
  std::swap(frame, fh.frame);
  return Frame(frame);
}

//template<typename T>
//bool FrameQueue::pollForFrame(T *output) const {
//  if (!std::is_base_of<Frame, T>::value) return false;
//
//  FrameHolder fh;
//  if (queue_->queue.tryDequeue(&fh)) {
//    FrameInterface *frame = nullptr;
//    std::swap(frame, fh.frame);
//    *output = frame;
//  }
//
//  return false;
//}
//
//template<typename T>
//bool FrameQueue::tryWaitForFrame(T *output, uint32_t timeout_ms) const {
//  if (!std::is_base_of<Frame, T>::value) return false;
//
//  libsmartereye2::FrameHolder fh;
//  if (queue_->queue.dequeue(&fh, timeout_ms)) {
//    return false;
//  }
//
//  FrameInterface *frame = nullptr;
//  std::swap(frame, fh.frame);
//  *output = frame;
//  return true;
//}

}  // namespace se2
