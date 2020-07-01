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

#ifndef LIBSMARTEREYE2_FRAME_INTERFACE_H
#define LIBSMARTEREYE2_FRAME_INTERFACE_H

#include <vector>
#include <atomic>
#include <mutex>

#include "frame_extension.h"
#include "metadata_parser.h"
#include "se_util.h"
#include "se_types.h"
#include "se_callbacks.h"

namespace libsmartereye2 {

class SensorInterface;
class StreamProfileBase;

class FrameInterface {
 public:
  virtual std::shared_ptr<SensorInterface> getSensor() const = 0;
  virtual int64_t getFrameMetadata(const FrameMetadataValue &frame_metadata) const = 0;
  virtual bool supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const = 0;
  virtual size_t getFrameDataSize() const = 0;
  virtual const char *getFrameData() const = 0;
  virtual double getFrameTimestamp() const = 0;
  virtual int64_t getFrameTimestampDomain() const = 0;
  virtual int64_t getFrameNumber() const = 0;
  virtual std::shared_ptr<StreamProfileBase> getStream() const = 0;

  virtual void setTimestamp(double new_ts) = 0;
  virtual void setTimestampDomain(TimestampDomain timestamp_domain) = 0;
  virtual void setStream(std::shared_ptr<StreamProfileBase> sp) = 0;

  virtual void acquire() = 0;
  virtual void release() = 0;
  virtual void keep() = 0;

  virtual void markFixed() = 0;
  virtual bool isFixed() const = 0;
  virtual void setBlocking(bool state) = 0;
  virtual bool isBlocking() const = 0;
};

struct FrameHolder {
  FrameInterface *frame;

  FrameHolder() : frame(nullptr) {}
  explicit FrameHolder(FrameInterface *frame) : frame(frame) {}
  FrameHolder(const FrameHolder &other) : frame(other.frame) {
    frame->acquire();
  }
  FrameHolder(FrameHolder &&other) noexcept
      : frame(other.frame) { other.frame = nullptr; }

  FrameHolder &operator=(FrameHolder &other) = delete;
  FrameHolder &operator=(FrameHolder &&other) noexcept {
    if (frame) {
      frame->release();
    }
    frame = other.frame;
    other.frame = nullptr;
    return *this;
  }

  explicit operator bool() const { return frame != nullptr; }
  explicit operator FrameInterface *() const { return frame; }
  FrameInterface *operator->() const { return frame; }

  FrameHolder clone() const { return FrameHolder(*this); }
  bool isBlocking() const { return frame->isBlocking(); }
};

static const int kUserQueueSize(128);

template<class T>
class FrameArchive : public std::enable_shared_from_this<FrameArchive<T>> {
 public:
  explicit FrameArchive(std::atomic<uint32_t> *in_max_frame_queue_size,
                        std::shared_ptr<platform::TimeService> ts,
                        std::shared_ptr<MetadataParserMap> parsers);
  ~FrameArchive();

  CallbackInvocationHolder begin_callback();
  void release_frame_ref(FrameInterface *ref);
  FrameInterface *alloc_and_track(size_t size, const FrameExtension &additional_data, bool requires_memory);
  void flush();

 protected:
  std::shared_ptr<SensorInterface> get_sensor() const { return sensor_.lock(); }
  void set_sensor(std::shared_ptr<SensorInterface> sensor) { sensor_ = sensor; }

  FrameInterface *publish_frame(FrameInterface *frame);
  void unpublish_frame(FrameInterface *frame);
  void keep_frame(FrameInterface *frame);
  std::shared_ptr<MetadataParserMap> get_md_parsers() const { return metadata_parsers_; }

  T allocFrame(size_t size, const FrameExtension &frame_extension, bool requires_memory);
  FrameInterface *trackFrame(T &f);

 private:
  friend class FrameData;

  std::atomic<uint32_t> *max_frame_queue_size_{};
  std::atomic<uint32_t> published_frames_count_{};
  SmallHeap<T, kUserQueueSize> published_frames_;
  std::shared_ptr<MetadataParserMap> metadata_parsers_ = nullptr;
  CallbacksHeap callback_inflight_;

  std::vector<T> freelist_; // return frames here
  std::atomic<bool> recycle_frames_{};
  int pending_frames_size_ = 0;
  std::recursive_mutex mutex_;
  std::shared_ptr<platform::TimeService> time_service_;

  std::weak_ptr<SensorInterface> sensor_;
};

class FrameData : public FrameInterface, public noncopyable {
 public:
  FrameData();
  virtual ~FrameData();
  FrameData(FrameData &&other) noexcept;
  FrameData &operator=(FrameData &&other) noexcept;

  int64_t getFrameMetadata(const FrameMetadataValue &frame_metadata) const override;
  bool supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const override;
  size_t getFrameDataSize() const override;
  const char *getFrameData() const override;
  double getFrameTimestamp() const override;
  int64_t getFrameTimestampDomain() const override;
  int64_t getFrameNumber() const override;
  std::shared_ptr<StreamProfileBase> getStream() const override;

  void setTimestamp(double new_ts) override;
  void setTimestampDomain(TimestampDomain timestamp_domain) override;
  void setStream(std::shared_ptr<StreamProfileBase> sp) override;

  void acquire() override;
  void release() override;
  void keep() override;

 private:
  std::vector<char> data_;
  FrameExtension extension_data_;
  std::atomic<int> ref_count_;
  std::atomic_bool kept_;
  bool fixed_{};
  std::shared_ptr<FrameArchive<FrameInterface>> owner_; // pointer to the owner to be returned to by last observe
  std::shared_ptr<SensorInterface> sensor_;
  std::shared_ptr<StreamProfileBase> stream_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_INTERFACE_H
