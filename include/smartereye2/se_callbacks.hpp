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

#ifndef LIBSMARTEREYE2_SE_CALLBACKS_H
#define LIBSMARTEREYE2_SE_CALLBACKS_H

#include "se_util.hpp"
#include "se_types.hpp"
#include "device/device_types.hpp"

struct SeDevicesChangedCallback {
  virtual void onDevicesChanged(SeDeviceList *removed, SeDeviceList *added) = 0;
  virtual void release() = 0;
};
using DevicesChangedCallbackPtr = std::shared_ptr<SeDevicesChangedCallback>;

struct SeNotificationsCallback {
  virtual void onNotification(SeNotification *notif) = 0;
  virtual void release() = 0;
};

struct SePlaybackChangedCallback {
  virtual void onPlaybackStatusChanged(se2::PlaybackStatus status) = 0;
  virtual void release() = 0;
};

struct SeFrameCallback {
  virtual void onFrame(libsmartereye2::FrameInterface *frame) = 0;
  virtual void release() = 0;
};
using FrameCallbackPtr = std::shared_ptr<SeFrameCallback>;

struct SeFrameProcessorCallback {
  virtual void onFrame(SeFrame *frame, SeSyntheticSource *source) = 0;
  virtual void release() = 0;
};
using FrameProcessorCallbackPtr = std::shared_ptr<SeFrameProcessorCallback>;

struct CallbackInvocation {
  std::chrono::high_resolution_clock::time_point started;
  std::chrono::high_resolution_clock::time_point ended;
};
typedef SmallHeap<CallbackInvocation, 1> CallbacksHeap;

struct CallbackInvocationHolder : public noncopyable {
  CallbackInvocationHolder() : invocation(nullptr), owner(nullptr) {}
  CallbackInvocationHolder(CallbackInvocationHolder &&other) noexcept
      : invocation(other.invocation), owner(other.owner) {
    other.invocation = nullptr;
  }

  CallbackInvocationHolder(CallbackInvocation *invocation, CallbacksHeap *owner)
      : invocation(invocation), owner(owner) {}

  ~CallbackInvocationHolder() {
    if (invocation) owner->deallocate(invocation);
  }

  CallbackInvocationHolder &operator=(CallbackInvocationHolder &&other) noexcept {
    invocation = other.invocation;
    owner = other.owner;
    other.invocation = nullptr;
    return *this;
  }

  explicit operator bool() {
    return invocation != nullptr;
  }

 private:
  CallbackInvocation *invocation;
  CallbacksHeap *owner;
};

#endif //LIBSMARTEREYE2_SE_CALLBACKS_H
