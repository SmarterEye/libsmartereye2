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

#ifndef LIBSMARTEREYE2_FRAME_AGGREGATOR_H
#define LIBSMARTEREYE2_FRAME_AGGREGATOR_H

#include <map>
#include <set>
#include <atomic>

#include "proc/processing.h"
#include "concurrency/consumer_queue.h"

namespace libsmartereye2 {

using StreamId = int;

class FrameAggregator : public ProcessingBlock {
 public:
  explicit FrameAggregator(const std::vector<int>& streams_to_aggregate);

  bool dequeue(FrameHolder *item, uint32_t timeout_ms);
  bool tryDequeue(FrameHolder *item);
  void start();
  void stop();

 private:
  void handleFrame(const FrameHolder& frame, SyntheticSourceInterface *source);

  std::mutex mutex_;
  std::map<StreamId, FrameHolder> last_set_;
  std::unique_ptr<ConsumerQueue<FrameHolder>> queue_;
  std::set<int> streams_to_aggregate_ids_;
  std::atomic<bool> accepting_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_AGGREGATOR_H
