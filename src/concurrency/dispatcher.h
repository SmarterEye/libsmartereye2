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

#ifndef LIBSMARTEREYE2_DISPATCHER_H
#define LIBSMARTEREYE2_DISPATCHER_H

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include "consumer_queue.h"

namespace libsmartereye2 {

class Dispatcher {
 public:
  class CancellableTimer {
   public:
    explicit CancellableTimer(Dispatcher *owner)
        : owner_(owner) {}

    bool trySleep(std::chrono::milliseconds::rep ms) {
      using namespace std::chrono;

      std::unique_lock<std::mutex> lock(owner_->was_stopped_mutex_);
      auto good = [&]() { return owner_->was_stopped_.load(); };
      return !(owner_->was_stopped_cv_.wait_for(lock, milliseconds(ms), good));
    }

   private:
    Dispatcher *owner_;
  };

  explicit Dispatcher(unsigned int cap);

  template<class T>
  void invoke(T item, bool is_blocking = false) {
    if (!was_stopped_) {
      if (is_blocking)
        queue_.blockingEnqueue(std::move(item));
      else
        queue_.enqueue(std::move(item));
    }
  }

  template<class T>
  void invoke_and_wait(T item, std::function<bool()> exit_condition, bool is_blocking = false) {
    bool done = false;

    //action
    auto func = std::move(item);
    invoke([&, func](Dispatcher::CancellableTimer c) {
      func(c);
      done = true;
      blocking_invoke_cv_.notify_one();
    }, is_blocking);

    //wait
    std::unique_lock<std::mutex> lk(blocking_invoke_mutex_);
    while (blocking_invoke_cv_.wait_for(lk,
                                        std::chrono::milliseconds(10),
                                        [&]() { return !done && !exit_condition(); }));
  }

  void start();
  void stop();

  ~Dispatcher();

  bool flush();
  bool empty() const { return queue_.size() == 0; }

 private:
  friend CancellableTimer;

  ConsumerQueue<std::function<void(CancellableTimer)>> queue_;
  std::thread thread_;

  std::atomic<bool> was_stopped_;
  std::condition_variable was_stopped_cv_;
  std::mutex was_stopped_mutex_;

  std::atomic<bool> was_flushed_;
  std::condition_variable was_flushed_cv_;
  std::mutex was_flushed_mutex_;

  std::condition_variable blocking_invoke_cv_;
  std::mutex blocking_invoke_mutex_;

  std::atomic<bool> is_alive_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_DISPATCHER_H
