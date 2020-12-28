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

#include "dispatcher.h"

namespace libsmartereye2 {

Dispatcher::Dispatcher(unsigned int cap)
    : queue_(cap),
      was_stopped_(true),
      was_flushed_(false),
      is_alive_(true) {
  thread_ = std::thread([&]() {
    int timeout_ms = 500000;
    while (is_alive_) {
      std::function<void(CancellableTimer)> item;

      if (queue_.dequeue(&item, timeout_ms)) {
        CancellableTimer time(this);

        try {
          item(time);
        }
        catch (...) {}
      }

      std::unique_lock<std::mutex> lock(was_flushed_mutex_);
      was_flushed_ = true;
      was_flushed_cv_.notify_all();
      lock.unlock();
    }
  });
}

void Dispatcher::start() {
  std::unique_lock<std::mutex> lock(was_stopped_mutex_);
  was_stopped_ = false;

  queue_.start();
}

void Dispatcher::stop() {
  {
    std::unique_lock<std::mutex> lock(was_stopped_mutex_);
    was_stopped_ = true;
    was_stopped_cv_.notify_all();
  }

  queue_.clear();

  {
    std::unique_lock<std::mutex> lock(was_flushed_mutex_);
    was_flushed_ = false;
  }

  std::unique_lock<std::mutex> lock_was_flushed(was_flushed_mutex_);
  was_flushed_cv_.wait_for(lock_was_flushed, std::chrono::hours(999999), [&]() { return was_flushed_.load(); });

  queue_.start();
}

Dispatcher::~Dispatcher() {
  stop();
  queue_.clear();
  is_alive_ = false;
  thread_.join();
}

bool Dispatcher::flush() {
  std::mutex m;
  std::condition_variable cv;
  bool invoked = false;
  auto wait_sucess = std::make_shared<std::atomic_bool>(true);
  invoke([&, wait_sucess](CancellableTimer t) {
    ///TODO: use _queue to flush, and implement properly
    if (was_stopped_ || !(*wait_sucess))
      return;

    {
      std::lock_guard<std::mutex> locker(m);
      invoked = true;
    }
    cv.notify_one();
  });
  std::unique_lock<std::mutex> locker(m);
  *wait_sucess = cv.wait_for(locker, std::chrono::seconds(10), [&]() { return invoked || was_stopped_; });
  return *wait_sucess;
}

}  // namespace libsmartereye2
