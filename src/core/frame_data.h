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

#ifndef LIBSMARTEREYE2_FRAME_DATA_H
#define LIBSMARTEREYE2_FRAME_DATA_H

#include "frame.h"
#include "frame_archive.h"
#include "core/core_types.hpp"
#include "se_util.hpp"

namespace libsmartereye2 {

std::shared_ptr<ArchiveInterface> makeArchive(SeExtension extension_type,
                                              std::atomic<uint32_t> *in_max_frame_queue_size,
                                              const std::shared_ptr<platform::TimeService> &ts,
                                              const std::shared_ptr<MetadataParserMap> &parsers);

class FrameData : public FrameInterface, public noncopyable {
 public:
  FrameData();

  virtual ~FrameData();

  FrameData(FrameData &&other) noexcept;

  FrameData &operator=(FrameData &&other) noexcept;

  std::shared_ptr<SensorInterface> getSensor() const override;

  void setSensor(std::shared_ptr<SensorInterface> sensor) override;

  int64_t getFrameMetadata(const FrameMetadataValue &frame_metadata) const override;

  bool supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const override;

  size_t getFrameDataSize() const override;

  const char *getFrameData() const override;

  double getFrameTimestamp() const override;

  TimestampDomain getFrameTimestampDomain() const override;

  int64_t getFrameIndex() const override;

  std::shared_ptr<StreamProfileInterface> getStream() const override;

  void setTimestamp(double new_ts) override;

  void setTimestampDomain(TimestampDomain timestamp_domain) override;

  void setStream(std::shared_ptr<StreamProfileInterface> sp) override;

  void acquire() override;

  void release() override;

  void keep() override;

  FrameInterface *publish(std::shared_ptr<ArchiveInterface> new_owner) override;

  void unpublish() override {}

  void markFixed() override { fixed_ = true; }

  bool isFixed() const override { return fixed_; }

  void setBlocking(bool state) override { extension_data_.is_blocking = state; }

  bool isBlocking() const override { return extension_data_.is_blocking; }

  ArchiveInterface *getOwner() const override { return owner_.get(); }

  std::vector<char> &data() { return data_; }

  FrameExtension &extension() { return extension_data_; }

 protected:
  std::vector<char> data_;
  FrameExtension extension_data_;
  std::atomic<int> ref_count_;
  std::atomic_bool kept_;
  bool fixed_{};
  std::shared_ptr<ArchiveInterface> owner_; // pointer to the owner to be returned to by last observe
  std::weak_ptr<SensorInterface> sensor_;
  std::shared_ptr<StreamProfileInterface> stream_ = nullptr;
};

class CompositeFrameData : public FrameData {
 public:
  CompositeFrameData() : FrameData() {}

  FrameInterface *getFrame(size_t i) const;

  FrameInterface **getFrames() const { return (FrameInterface **) data_.data(); }

  const FrameInterface *first() const { return getFrame(0); }

  void release() override;

  size_t getFrameCount() const { return frame_count_; }

  void setFrameCount(size_t count) { frame_count_ = count; }

 private:
  size_t frame_count_ = 0;
};

class VideoFrameData : public FrameData {
 public:
  VideoFrameData() : FrameData(), width_(0), height_(0), bpp_(0), stride_(0) {}

  VideoFrameData(int width, int height, int stride, int bpp)
      : FrameData(), width_(width), height_(height), bpp_(bpp), stride_(stride) {}

  void assign(int width, int height, int stride, int bpp) {
    width_ = width;
    height_ = height;
    bpp_ = bpp;
    stride_ = stride;
  }

  int width() const { return width_; }

  int height() const { return height_; }

  int bpp() const { return bpp_; }

  int stride() const { return stride_; }

 private:
  int width_, height_, bpp_, stride_;
};

class DepthFrameData : public VideoFrameData {
 public:
  DepthFrameData() : VideoFrameData(), depth_units_(0.f) {}

  FrameInterface *publish(std::shared_ptr<ArchiveInterface> new_owner) override;

  void keep() override;

  float distance(int x, int y) const;

  float units() const;

  void setOriginal(FrameHolder holder);

 private:
  static float queryUnits(const std::shared_ptr<SensorInterface> &sensor);

  FrameHolder original_;
  mutable float depth_units_;
};

class DisparityData : public DepthFrameData {
 public:
  DisparityData() : DepthFrameData() {}

  float stereoBaseline() const { return queryStereoBaseline(getSensor()); }

 private:
  static float queryStereoBaseline(const std::shared_ptr<SensorInterface> &sensor);
};

class MotionFrameData : public FrameData {
 public:
  MotionFrameData() : FrameData() {}
};

class PointsData : public FrameData {
 public:
  const Vertex *vertex() const;

  size_t vertexCount() const;

  const TextureCoordinate *textureCoordinate() const;

  void exportToPly(const std::string &fname, const FrameHolder &texture);
};

class PoseData : public FrameData {
 public:
  // TODO
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_FRAME_DATA_H