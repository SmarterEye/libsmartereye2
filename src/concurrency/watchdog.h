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

#ifndef LIBSMARTEREYE2_WATCHDOG_H
#define LIBSMARTEREYE2_WATCHDOG_H

#include "dispatcher.h"

namespace libsmartereye2 {

class Watchdog {
 public:
  Watchdog(std::function<void()> operation, uint64_t timeout_ms) :
      timeout_ms_(timeout_ms), operation_(std::move(operation)) {
    watcher_ = std::make_shared<RepeatOperation<>>([this](Dispatcher::CancellableTimer cancellable_timer) {
      if (cancellable_timer.trySleep(timeout_ms_)) {
        if (!kicked_)
          operation_();
        std::lock_guard<std::mutex> lk(mutex_);
        kicked_ = false;
      }
    });
  }

  ~Watchdog() {
    if (running_)
      stop();
  }

  void start() {
    std::lock_guard<std::mutex> lk(mutex_);
    watcher_->start();
    running_ = true;
  }
  void stop() {
    {
      std::lock_guard<std::mutex> lk(mutex_);
      running_ = false;
    }
    watcher_->stop();
  }
  bool running() {
    std::lock_guard<std::mutex> lk(mutex_);
    return running_;
  }
  void set_timeout(uint64_t timeout_ms) {
    std::lock_guard<std::mutex> lk(mutex_);
    timeout_ms_ = timeout_ms;
  }
  void kick() {
    std::lock_guard<std::mutex> lk(mutex_);
    kicked_ = true;
  }

 private:
  std::mutex mutex_;
  uint64_t timeout_ms_;
  bool kicked_ = false;
  bool running_ = false;
  std::function<void()> operation_;
  std::shared_ptr<RepeatOperation<>> watcher_{};
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_WATCHDOG_H
