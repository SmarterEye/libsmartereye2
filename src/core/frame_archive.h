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

#ifndef LIBSMARTEREYE2_FRAME_ARCHIVE_H
#define LIBSMARTEREYE2_FRAME_ARCHIVE_H

#include <utility>
#include <vector>
#include <atomic>
#include <mutex>

#include "frame.h"
#include "metadata_parser.h"
#include "easylogging++.h"
#include "se_common.h"

#include "se_callbacks.hpp"

namespace libsmartereye2 {

class ArchiveInterface {
 public:
  virtual std::shared_ptr<SensorInterface> get_sensor() const = 0;

  virtual void set_sensor(std::shared_ptr<SensorInterface> sensor) = 0;

  virtual CallbackInvocationHolder begin_callback() = 0;

  virtual FrameInterface *alloc_and_track(size_t size, const FrameExtension &additional_data, bool requires_memory) = 0;

  virtual std::shared_ptr<MetadataParserMap> get_md_parsers() const = 0;

  virtual void flush() = 0;

  virtual FrameInterface *publish_frame(FrameInterface *frame) = 0;

  virtual void unpublish_frame(FrameInterface *frame) = 0;

  virtual void keep_frame(FrameInterface *frame) = 0;
};

static const int kUserQueueSize(128);

template<class T>
class FrameArchive : public std::enable_shared_from_this<FrameArchive<T>>, public ArchiveInterface {
 public:
  explicit FrameArchive(std::atomic<uint32_t> *in_max_frame_queue_size,
                        std::shared_ptr<platform::TimeService> ts,
                        std::shared_ptr<MetadataParserMap> parsers)
      : max_frame_queue_size_(in_max_frame_queue_size),
        time_service_(std::move(ts)),
        metadata_parsers_(std::move(parsers)),
        published_frames_count_(0) {

  }

  ~FrameArchive() {
    if (pending_frames_size_ > 0) {
      LOG(INFO) << "All frames from stream 0x"
                << std::hex << this << " are now released by the user" << std::dec;
    }
  }

  CallbackInvocationHolder begin_callback() override {
    return {callback_inflight_.allocate(), &callback_inflight_};
  }

  void release_frame_ref(FrameInterface *ref) { ref->release(); }

  FrameInterface *alloc_and_track(size_t size, const FrameExtension &additional_data, bool requires_memory) override {
    auto frame = allocFrame(size, additional_data, requires_memory);
    return trackFrame(frame);
  }

  void flush() override {
    published_frames_.stopAllocating();
    callback_inflight_.stopAllocating();

    auto callbacks_infight_size = callback_inflight_.size();
    if (callbacks_infight_size > 0) {
      LOG(WARNING) << callbacks_infight_size
                   << " callbacks are still running on some other threads. Waiting until all callbacks return...";
    }
    callback_inflight_.waitUntilEmpty();

    pending_frames_size_ = published_frames_.size();
    if (pending_frames_size_ > 0) {
      LOG(INFO) << "The user was holding on to "
                << std::dec << pending_frames_size_ << " frames after stream 0x"
                << std::hex << this << " stopped" << std::dec;
    }
  }

 protected:
  friend class FrameData;

  std::shared_ptr<SensorInterface> get_sensor() const override { return sensor_.lock(); }
  void set_sensor(std::shared_ptr<SensorInterface> sensor) override { sensor_ = sensor; }

  FrameInterface *publish_frame(FrameInterface *frame) override {
    T *cur_frame = (T *) frame;
    uint32_t max_frames_size = *max_frame_queue_size_;

    if (max_frames_size > 0 && published_frames_count_ >= max_frames_size) {
      LOG(DEBUG) << "User didn't release frame resource.";
      return nullptr;
    }

    T *new_frame = (max_frames_size > 0 ? published_frames_.allocate() : new T());
    if (new_frame) {
      if (max_frames_size > 0) new_frame->markFixed();
    } else {
      new_frame = new T();
    }
    ++published_frames_count_;
    *new_frame = std::move(*cur_frame);
    return new_frame;
  }

  void unpublish_frame(FrameInterface *frame) override {
    if (frame) {
      T *f = (T *) frame;
      std::unique_lock<std::recursive_mutex> lock(mutex_);
      frame->keep();
      lock.unlock();

      if (f->isFixed())
        published_frames_.deallocate(f);
      else
        delete f;
    }
  }

  void keep_frame(FrameInterface *frame) override {
    --published_frames_count_;
  }

  std::shared_ptr<MetadataParserMap> get_md_parsers() const override { return metadata_parsers_; }

  T allocFrame(size_t size, const FrameExtension &frame_extension, bool requires_memory) {
    T back_buffer;

    if (requires_memory) {
      back_buffer.data().resize(size, 0);
    }
    back_buffer.extension() = frame_extension;
    return back_buffer;
  }

  FrameInterface *trackFrame(T &f) {
    std::unique_lock<std::recursive_mutex> locker(mutex_);
    auto published_frame = f.publish(this->shared_from_this());
    if (published_frame) {
      published_frame->acquire();
      return published_frame;
    }

    LOG(DEBUG) << "publish(...) failed";
    return nullptr;
  }

 private:
  friend class FrameData;

  std::atomic<uint32_t> *max_frame_queue_size_{};
  std::atomic<uint32_t> published_frames_count_{};
  SmallHeap<T, kUserQueueSize> published_frames_;
  std::shared_ptr<MetadataParserMap> metadata_parsers_ = nullptr;
  CallbacksHeap callback_inflight_;

//  std::vector<T> freelist_; // return frames here
//  std::atomic<bool> recycle_frames_{};
  int pending_frames_size_ = 0;
  std::recursive_mutex mutex_;
  std::shared_ptr<platform::TimeService> time_service_;

  std::weak_ptr<SensorInterface> sensor_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_ARCHIVE_H
