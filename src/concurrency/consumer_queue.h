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

#ifndef LIBSMARTEREYE2_CONSUMER_QUEUE_H
#define LIBSMARTEREYE2_CONSUMER_QUEUE_H

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace libsmartereye2 {

const uint32_t kQueueMaxSize = 256;

template<class T>
class ConsumerQueue {
 public:
  explicit ConsumerQueue(uint32_t capacity = kQueueMaxSize)
      : queue_(), mutex_(), deq_cv_(), enq_cv_(),
        capacity_(capacity), need_to_flush_(false), was_flushed_(false), accepting_(true) {
  }

  void enqueue(T &&item) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (accepting_) {
      queue_.push_back(std::move(item));
      if (queue_.size() > capacity_) {
        queue_.pop_front();
      }
    }
    lock.unlock();
    deq_cv_.notify_one();
  }

  void blockingEnqueue(T &&item) {
    auto pred = [this]() -> bool { return (queue_.size() < capacity_ || need_to_flush_); };

    std::unique_lock<std::mutex> lock(mutex_);
    if (accepting_) {
      enq_cv_.wait(lock, pred);
      queue_.push_back(std::move(item));
    }
    lock.unlock();
    deq_cv_.notify_one();
  }

  bool dequeue(T *item, uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    accepting_ = true;
    was_flushed_ = true;
    const auto ready = [this]() -> bool { return queue_.size() > 0 || need_to_flush_; };
    if (!ready() && !deq_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), ready)) {
      return false;
    }

    if (queue_.size() <= 0) return false;

    *item = std::move(queue_.front());
    queue_.pop_front();
    enq_cv_.notify_one();
    return true;
  }

  bool tryDequeue(T *item) {
    std::unique_lock<std::mutex> lock(mutex_);
    accepting_ = true;
    if (!queue_.empty()) {
      auto val = std::move(queue_.front());
      queue_.pop_front();
      *item = std::move(val);
      enq_cv_.notify_one();
      return true;
    }
    return false;
  }

  void popFront() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!queue_.empty()) queue_.pop_front();
  }

  bool peek(T **item) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) return false;
    *item = &queue_.front();
    return true;
  }

  void clear() {
    std::unique_lock<std::mutex> lock(mutex_);
    accepting_ = false;
    need_to_flush_ = true;
    enq_cv_.notify_all();
    while (!queue_.empty()) {
      auto item = std::move(queue_.front());
      queue_.pop_front();
    }
    deq_cv_.notify_all();
  }

  void start() {
    std::unique_lock<std::mutex> lock(mutex_);
    need_to_flush_ = false;
    accepting_ = true;
  }

  size_t size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::deque<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable deq_cv_; // not empty signal
  std::condition_variable enq_cv_; // not full signal

  uint32_t capacity_;
  bool accepting_;

  // flush mechanism is required to abort wait on cv
  // when need to stop
  std::atomic<bool> need_to_flush_;
  std::atomic<bool> was_flushed_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_CONSUMER_QUEUE_H
