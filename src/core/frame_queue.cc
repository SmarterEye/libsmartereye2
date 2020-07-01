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

#include "frame.h"

namespace libsmartereye2 {

void libsmartereye2::FrameQueue::operator()(Frame frame) {
  enqueue(std::move(frame));
}

void FrameQueue::enqueue(Frame frame) {
  if (keep_) frame.keep();
  FrameHolder holder(frame.frame_ref_);
  queue_->enqueue(std::move(holder));
}

Frame FrameQueue::waitForFrame(int32_t timeout_ms) const {
  FrameHolder holder;
  if (!queue_->dequeue(&holder, timeout_ms)) {
    throw std::runtime_error("Frame did not arrive in time!");
  }

  FrameInterface *result = nullptr;
  std::swap(result, holder.frame);
  return Frame(result);
}

}  // namespace libsmartereye2
