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

#ifndef LIBSMARTEREYE2_FRAME_HPP
#define LIBSMARTEREYE2_FRAME_HPP

#include "core_types.hpp"
#include "smartereye2/se_global.hpp"
#include "smartereye2/se_types.hpp"
#include "smartereye2/se_callbacks.hpp"
#include "smartereye2/streaming/stream_profile.hpp"
#include "smartereye2/alg/packed_types.h"

namespace se2 {

class SMARTEREYE2_API Frame {
 public:
  Frame() : frame_ref_(nullptr) {}

  explicit Frame(SeFrame *ref);

  Frame(const Frame &other) : frame_ref_(other.frame_ref_) {
    if (frame_ref_) {
      addRef();
      frame_index_ = other.frame_index_;
      profile_ = other.profile_;
    }
  }

  Frame(Frame &&other) noexcept: frame_ref_(other.frame_ref_) {
    other.frame_ref_ = nullptr;
    frame_index_ = other.frame_index_;
    profile_ = other.profile_;
  }

  Frame &operator=(Frame other) {
    swap(other);
    return *this;
  }

  explicit operator bool() const { return frame_ref_ != nullptr; }
  explicit operator SeFrame *() { return frame_ref_; }

  void swap(Frame &other) {
    std::swap(frame_ref_, other.frame_ref_);
    std::swap(frame_index_, other.frame_index_);
    std::swap(profile_, other.profile_);
  }

  virtual ~Frame();

  void keep();
  SeSensor *getSensor() const;
  StreamProfile getProfile() const;

  double timestamp() const;
  const char *getFrameMetadata(FrameMetadataValue frame_metadata) const;
  size_t getFrameMetadataSize() const;
  bool supportsFrameMetadata(FrameMetadataValue frame_metadata) const;
  int64_t getFrameIndex() const;
  int64_t getSpeed() const;
  size_t dataSize() const;
  const char *data() const;

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

  SeFrame *get() const { return frame_ref_; }

 protected:
  void addRef() const;
  void reset();

 private:
  friend class Points;
  friend class ProcessingBlock;
  friend class FrameQueue;

  SeFrame *frame_ref_;
  StreamProfile profile_;
  uint64_t frame_index_ = 0;
};

class SMARTEREYE2_API VideoFrame : public Frame {
 public:
  explicit VideoFrame(const Frame &frame) : Frame(frame) {
    if (!frame) {
      reset();
    }
  }

  int width() const;
  int height() const;
  int strideInBytes() const;
  int bitsPerPixel() const;
  int bytesPerPixel() const;
};

class SMARTEREYE2_API DepthFrame : public VideoFrame {
 public:
  explicit DepthFrame(const Frame &frame) : VideoFrame(frame) {}

  float distance(int x, int y) const;
  float units() const;
};

class DisparityFrame : public DepthFrame {
 public:
  explicit DisparityFrame(const Frame &frame) : DepthFrame(frame) {}

  float baseline() const;
};

class SMARTEREYE2_API Points : public Frame {
 public:
  explicit Points() : Frame(), size_(0) {}
  explicit Points(const Frame &frame);

  const Vertex *vertices() const;
  const TextureCoordinate *textureCoordinate() const;
  void exportToPly(const std::string &fnmae, VideoFrame texture) const;
  size_t size() const { return size_; }

 private:
  size_t size_;
};

class SMARTEREYE2_API MotionFrame : public Frame {
 public:
  explicit MotionFrame(const Frame &frame) : Frame(frame) {}
  SeVector3f getMotionData() const;
};

class SMARTEREYE2_API JourneyFrame : public Frame {
 public:
  explicit JourneyFrame(const Frame &frame) : Frame(frame) {}
  const std::string &meta() const;
};

class SMARTEREYE2_API ObstacleFrame : public Frame {
 public:
  explicit ObstacleFrame(const Frame &frame) : Frame(frame) {}
  int num() const;
  std::vector<std::shared_ptr<SEObstacle>> obstacles() const;
};

class SMARTEREYE2_API FreeSpaceFrame : public Frame {
 public:
  explicit FreeSpaceFrame(const Frame &frame) : Frame(frame) {}
  int pointNum() const;
  std::vector<std::shared_ptr<SEFreeSpacePoint>> freeSpacePoints() const;
};

class SMARTEREYE2_API VehicleInfoFrame : public Frame {
 public:
  explicit VehicleInfoFrame(const Frame &frame) : Frame(frame) {}
  VehicleInfo vehicleInfo() const;
};

template<class T>
class FramCallback : public SeFrameCallback {
  T on_frame_function_;

 public:
  explicit FramCallback(T on_frame) : on_frame_function_(on_frame) {}
  void onFrame(libsmartereye2::FrameInterface *frame) override { on_frame_function_(frame); }
  void release() override { delete this; }
};

}  // namespace se2

#endif  // LIBSMARTEREYE2_FRAME_HPP
