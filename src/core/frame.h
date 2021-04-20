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

#ifndef LIBSMARTEREYE2_FRAME_H
#define LIBSMARTEREYE2_FRAME_H

#include <memory>
#include "core/core_types.hpp"
#include "se_callbacks.hpp"

namespace libsmartereye2 {

using namespace se2;

class SensorInterface;
class StreamProfileInterface;
class ArchiveInterface;

class FrameInterface {
 public:
  virtual std::shared_ptr<SensorInterface> getSensor() const = 0;
  virtual void setSensor(std::shared_ptr<SensorInterface> sensor) = 0;

  virtual const char* getFrameMetadata(const FrameMetadataValue &frame_metadata) const = 0;
  virtual size_t getFrameMetadataSize() const = 0;
  virtual bool supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const = 0;
  virtual size_t getFrameDataSize() const = 0;
  virtual const char *getFrameData() const = 0;
  virtual double getFrameTimestamp() const = 0;
  virtual TimestampDomain getFrameTimestampDomain() const = 0;
  virtual int64_t getFrameIndex() const = 0;
  virtual int64_t getSpeed() const = 0;
  virtual std::shared_ptr<StreamProfileInterface> getStreamProfile() const = 0;

  virtual void setTimestamp(double new_ts) = 0;
  virtual void setTimestampDomain(TimestampDomain timestamp_domain) = 0;
  virtual void setStreamProfile(std::shared_ptr<StreamProfileInterface> sp) = 0;

  virtual void acquire() = 0;
  virtual void release() = 0;
  virtual void keep() = 0;

  virtual FrameInterface *publish(std::shared_ptr<ArchiveInterface> new_owner) = 0;
  virtual void unpublish() = 0;

  virtual void markFixed() = 0;
  virtual bool isFixed() const = 0;
  virtual void setBlocking(bool state) = 0;
  virtual bool isBlocking() const = 0;

  virtual ArchiveInterface *getOwner() const = 0;
};

struct FrameHolder {
  FrameInterface *frame;

  FrameHolder() : frame(nullptr) {}
  explicit FrameHolder(FrameInterface *frame) : frame(frame) {}
  FrameHolder(FrameHolder &&other) noexcept: frame(other.frame) {
    other.frame = nullptr;
  }

  ~FrameHolder() {
    if (frame) {
      frame->release();
    }
  }

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

 private:
  FrameHolder(const FrameHolder &other) : frame(other.frame) {
    if (frame) frame->acquire();
  }
};

template<class T>
class InternalFrameCallback : public SeFrameCallback {
  T on_frame_function; //Callable of type: void(FrameInterface* frame)
 public:
  explicit InternalFrameCallback(T on_frame) : on_frame_function(on_frame) {}

  virtual void onFrame(FrameInterface *frame) override {
    on_frame_function(FrameHolder(frame));
  }
  virtual void release() override { delete this; }
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_H
