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

#include "core/stream_profile.h"
#include "device/device.h"
#include "se_types.h"
#include "frame_interface.h"

namespace libsmartereye2 {

class SensorInterface;
class StreamProfile;
struct SeSensor;

class Frame {
 public:
  Frame() : frame_ref_(nullptr) {}
  explicit Frame(FrameInterface *ref) : frame_ref_(nullptr) {}
  Frame(const Frame &other) : frame_ref_(other.frame_ref_) {
    if (frame_ref_) {
      addRef();
    }
  }
  Frame(Frame &&other) noexcept: frame_ref_(other.frame_ref_) {}
  Frame &operator=(Frame other) {
    swap(other);
    return *this;
  }
  explicit operator bool() const { return frame_ref_ != nullptr; }
  explicit operator FrameInterface *() { return frame_ref_; }

  void swap(Frame &other) {
    std::swap(frame_ref_, other.frame_ref_);
//    std::swap(frame_number_, other.frame_number_);
//    std::swap(profile_, other.profile_);
  }

  virtual ~Frame() {
    if (frame_ref_) {
      frame_ref_->release();
    }
  }

  void keep() { frame_ref_->keep(); }

  SeSensor *getSensor() const {
    std::shared_ptr<SensorInterface> sensor(frame_ref_->getSensor());
    DeviceInterface &dev = sensor->getDevice();
    return nullptr; // TODO
  }
  double getTimestamp() const {
    return frame_ref_->getFrameTimestamp();
  }
  int64_t getFrameMetadata(FrameMetadataValue frame_metadata) const {
    return frame_ref_->getFrameMetadata(frame_metadata);
  }
  bool supportsFrameMetadata(FrameMetadataValue frame_metadata) const {
    return frame_ref_->supportsFrameMetadata(frame_metadata);
  }
  int64_t getFrameNumber() const {
    return frame_ref_->getFrameNumber();
  }
  int32_t getDataSize() const {
    return frame_ref_->getFrameDataSize();
  }
  const char *getData() const {
    return frame_ref_->getFrameData();
  }
  StreamProfile getProfile() const {
    auto stream = frame_ref_->getStream();
    return StreamProfile(stream.get());
  }

  template<class T>
  bool is() const {
    T extension(*this);
    return extension;
  }

  template<class T>
  T as() const {
    T extension(*this);
    return extension;
  }

  FrameInterface *get() const { return frame_ref_; }

 protected:
  void addRef() const { frame_ref_->acquire(); }
  void reset() {
    if (frame_ref_) {
      frame_ref_->release();
    }
    frame_ref_ = nullptr;
  }

 private:
  friend class FrameQueue;

  FrameInterface *frame_ref_;
//  StreamProfile profile_;
//  uint64_t frame_number_ = 0;
};

class VideoFrame : public Frame {
 public:
  explicit VideoFrame(const Frame &frame) : Frame(frame) {}

  int width() const;
  int height() const;
  int strideInBytes() const;
  int bitsPerPixel() const;
  int bytesPerPixel() const;
};

class Points : public Frame {
 public:
  struct Vertex {
    float x, y, z;
    explicit operator const float *() const { return &x; }
  };

  struct TextureCoordinate {
    float u, v;
    explicit operator const float *() const { return &u; }
  };

  explicit Points() : Frame(), size_(0) {}
  explicit Points(const Frame &frame) : Frame(frame), size_(0) {}

  const Vertex *vertices() const;
  const TextureCoordinate *textureCoordinate() const;
  void exportToPly(const std::string *fnmae, VideoFrame texture) const;
  size_t size() const { return size_; }

 private:
  size_t size_;
};

class MotionFrame : public Frame {
 public:
  explicit MotionFrame(const Frame &frame) : Frame(frame) {}

  SeVector3f getMotionData() const;

};

template<class T>
class FramCallback : public SeFrameCallback {
  T on_frame_function_;

 public:
  explicit FramCallback(T on_frame) : on_frame_function_(on_frame) {}
  void onFrame(FrameInterface *frame) override {}
  void release() override { delete this; }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_FRAME_H
