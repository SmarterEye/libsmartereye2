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

#include "frame_interface.h"
#include "easylogging++.h"

namespace libsmartereye2 {

template<class T>
FrameArchive<T>::FrameArchive(std::atomic<uint32_t> *in_max_frame_queue_size,
                              std::shared_ptr<platform::TimeService> ts,
                              std::shared_ptr<MetadataParserMap> parsers)
    : max_frame_queue_size_(in_max_frame_queue_size),
      recycle_frames_(true), mutex_(), time_service_(ts),
      metadata_parsers_(parsers), published_frames_count_(0) {
}

template<class T>
FrameArchive<T>::~FrameArchive() {
  if (pending_frames_size_ > 0) {
    LOG(INFO) << "All frames from stream 0x"
              << std::hex << this << " are now released by the user" << std::dec;
  }
}

template<class T>
CallbackInvocationHolder FrameArchive<T>::begin_callback() {
  return {callback_inflight_.allocate(), &callback_inflight_};
}

template<class T>
void FrameArchive<T>::release_frame_ref(FrameInterface *ref) {
  ref->release();
}

template<class T>
FrameInterface *FrameArchive<T>::alloc_and_track(size_t size,
                                                 const FrameExtension &additional_data,
                                                 bool requires_memory) {
  auto frame = allocFrame(size, additional_data, requires_memory);
  return trackFrame(frame);
}

template<class T>
void FrameArchive<T>::flush() {
  published_frames_.stopAllocating();
  callback_inflight_.stopAllocating();
  recycle_frames_ = false;

  auto callbacks_infight_size = callback_inflight_.size();
  if (callbacks_infight_size > 0) {
    LOG(WARNING) << callbacks_inflight
                 << " callbacks are still running on some other threads. Waiting until all callbacks return...";
  }
  callback_inflight_.waitUntilEmpty();
  {
    std::lock_guard<std::recursive_mutex> locker(mutex_);
    freelist_.clear();
  }

  pending_frames_size_ = published_frames_.size();
  if (pending_frames_size_ > 0) {
    LOG(INFO) << "The user was holding on to "
              << std::dec << pending_frames << " frames after stream 0x"
              << std::hex << this << " stopped" << std::dec;
  }
}

template<class T>
FrameInterface *FrameArchive<T>::publish_frame(FrameInterface *frame) {
  T *foo = (T *) frame;
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
  *new_frame = std::move(*foo);
  return new_frame;
}

template<class T>
void FrameArchive<T>::unpublish_frame(FrameInterface *frame) {
  if (frame) {
    T *f = (T *) frame;
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    frame->keep();

    if (recycle_frames_) {
      freelist_.push_back(std::move(*f));
    }
    lock.unlock();

    if (f->isFixed())
      published_frames_.deallocate(f);
    else
      delete f;
  }
}

template<class T>
void FrameArchive<T>::keep_frame(FrameInterface *frame) {
  --published_frames_count_;
}

template<class T>
T FrameArchive<T>::allocFrame(size_t size, const FrameExtension &frame_extension, bool requires_memory) {
  T back_buffer;
  {
    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (requires_memory) {
      for (auto it = freelist_.begin(); it != freelist_.end(); ++it) {
        if (it->data.size == size) {
          back_buffer = std::move(*it);
          freelist_.erase(it);
          break;
        }
      }
    }

    for (auto it = freelist_.begin(); it != freelist_.end();) {
      if (frame_extension.timestamp > it->frame_extension.timestamp + 1000) {
        it = freelist_.erase(it);
      } else {
        ++it;
      }
    }
  }

  if (requires_memory) {
    back_buffer.data.resize(size, 0);
  }
  back_buffer.frame_extension = frame_extension;
  return back_buffer;
}

template<class T>
FrameInterface *FrameArchive<T>::trackFrame(T &f) {
  std::unique_lock<std::recursive_mutex> locker(mutex_);
  auto published_frame = f.publish(this->shared_from_this());
  if (published_frame) {
    published_frame->acquire();
    return published_frame;
  }

  LOG(DEBUG) << "publish(...) failed";
  return nullptr;
}

FrameData::FrameData()
    : ref_count_(0), kept_(false) {
}

FrameData::~FrameData() = default;

FrameData::FrameData(FrameData &&other) noexcept
    : ref_count_(other.ref_count_.exchange(0)),
      kept_(other.kept_.exchange(false)) {
  *this = std::move(other);
}

FrameData &FrameData::operator=(FrameData &&other) noexcept {
  data_ = std::move(other.data_);
  ref_count_ = other.ref_count_.exchange(0);
  kept_ = other.kept_.exchange(false);
  extension_data_ = other.extension_data_;
  return *this;
}

int64_t FrameData::getFrameMetadata(const FrameMetadataValue &frame_metadata) const {
  return 0;
}

bool FrameData::supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const {
  return false;
}

int32_t FrameData::getFrameDataSize() const {
  return data_.size();
}

const char *FrameData::getFrameData() const {
  return data_.data();
}

double FrameData::getFrameTimestamp() const {
  return extension_data_.timestamp;
}

int64_t FrameData::getFrameTimestampDomain() const {
  return static_cast<int64_t>(extension_data_.timestamp_domain);
}

int64_t FrameData::getFrameNumber() const {
  return extension_data_.frame_number;
}

std::shared_ptr<StreamProfileBase> FrameData::getStream() const {
  return stream_;
}

void FrameData::setTimestamp(double new_ts) {
  extension_data_.timestamp = new_ts;
}

void FrameData::setTimestampDomain(TimestampDomain timestamp_domain) {
  extension_data_.timestamp_domain = timestamp_domain;
}

void FrameData::setStream(std::shared_ptr<StreamProfileBase> sp) {
  stream_ = sp;
}

void FrameData::acquire() {
  ref_count_.fetch_add(1);
}

void FrameData::release() {

}
void FrameData::keep() {

}

}  // namespace libsmartereye2
