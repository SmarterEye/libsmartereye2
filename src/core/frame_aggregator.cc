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

#include "frame_aggregator.h"

#include <utility>
#include "frame_data.h"
#include "streaming/stream_profile.h"
#include "easylogging++.h"

#include "proc/processing.hpp"

namespace libsmartereye2 {

static const int kFrameNum2Aggregate(250);

FrameAggregator::FrameAggregator(const std::vector<int>& streams_to_aggregate)
    : ProcessingBlock("Aggregator"),
      queue_(new ConsumerQueue<FrameHolder>(kFrameNum2Aggregate)),
      accepting_(true) {
  for (auto uid : streams_to_aggregate) {
    streams_to_aggregate_ids_.insert(uid);
  }
}

bool FrameAggregator::dequeue(FrameHolder *item, uint32_t timeout_ms) {
  return queue_->dequeue(item, timeout_ms);
}

bool FrameAggregator::tryDequeue(FrameHolder *item) {
  return queue_->tryDequeue(item);
}

void FrameAggregator::start() {
  auto processing_callback = [&](const FrameHolder& frame, SyntheticSourceInterface *source) {
    handleFrame(frame, source);
  };

  setProcessingCallback(FrameProcessorCallbackPtr(
      new InternalFrameProcessorCallback<decltype(processing_callback)>(processing_callback)
  ));

  accepting_ = true;
}

void FrameAggregator::stop() {
  accepting_ = false;
  queue_->clear();
  setProcessingCallback(nullptr);
}

void FrameAggregator::handleFrame(const FrameHolder& frame, SyntheticSourceInterface *source) {
  if (!accepting_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  auto comp = dynamic_cast<CompositeFrameData *>(frame.frame);
  if (comp) {
    for (size_t i = 0; i < comp->getFrameCount(); i++) {
      FrameInterface *current_frame = comp->getFrame(i);
      current_frame->acquire();
      int uid = current_frame->getStreamProfile()->uniqueId();
      last_set_[uid] = FrameHolder(current_frame);
    }

    for (int stream_id : streams_to_aggregate_ids_) {
      if (!last_set_[stream_id]) return;
    }

    std::vector<FrameHolder> sync_set;
    for (auto &&pair : last_set_) {
      sync_set.push_back(pair.second.clone());
    }

    FrameHolder sync_fref(source->allocateCompositeFrame(std::move(sync_set)));
    if (!sync_fref) {
      LOG(ERROR) << "Failed to allocate composite frame";
      return;
    }
    queue_->enqueue(sync_fref.clone());
  } else {
//    source->frameReady(frame.clone());

    // filter invalid frame
    if (!frame->getStreamProfile()) return;

    auto uid = frame->getStreamProfile()->uniqueId();
    if (streams_to_aggregate_ids_.find(uid) == streams_to_aggregate_ids_.end()) {
      return;
    }

    last_set_[uid] = frame.clone();
    if (last_set_.size() == streams_to_aggregate_ids_.size()) {
      std::vector<FrameHolder> sync_set;
      for (auto &&s : last_set_) {
        sync_set.push_back(s.second.clone());
      }
      FrameHolder sync_fref(source->allocateCompositeFrame(std::move(sync_set)));
      if (!sync_fref) {
        LOG(ERROR) << "Failed to allocate composite frame";
        return;
      }
      sync_fref->setTimestamp(frame->getFrameTimestamp());
      sync_fref->setTimestampDomain(frame->getFrameTimestampDomain());
      sync_fref->setSensor(frame->getSensor());
      sync_fref->setStreamProfile(frame->getStreamProfile());
      (queue_->size() < kFrameNum2Aggregate) ? queue_->enqueue(sync_fref.clone()) : queue_->popFront();

      last_set_.clear();
    }
  }

}

}  // namespace libsmartereye2
