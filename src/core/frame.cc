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

#include "frame.h"
#include "sensor/sensor.h"
#include "streaming/stream_profile.h"
#include "core/frame_data.h"

#include "core/frame.hpp"
#include "core/frame_set.hpp"

namespace se2 {

FrameSet::FrameSet(const Frame &frame) : Frame(frame), size_(0) {
  if (get()) {
    // is composite frame ?
    auto cf = dynamic_cast<libsmartereye2::CompositeFrameData *>(get());
    if (cf) {
      size_ = cf->getFrameCount();
    }
  }
}

Frame::Frame(SeFrame *ref) : frame_ref_(ref) {
  if (ref) {
    frame_index_ = ref->getFrameIndex();
    auto frame_data = dynamic_cast<libsmartereye2::CompositeFrameData *>(ref);
    if (!frame_data) {
      profile_ = StreamProfile(new SeStreamProfile{ref->getStream().get()});
    }
  }
}

Frame::~Frame() {
  if (frame_ref_) {
    frame_ref_->release();
  }
}

void Frame::keep() {
  frame_ref_->keep();
}

SeSensor *Frame::getSensor() const {
  std::shared_ptr<libsmartereye2::SensorInterface> sensor(frame_ref_->getSensor());
  auto &dev = sensor->getDevice();
  return nullptr;   // TODO
}

StreamProfile Frame::getProfile() const {
  return profile_;
}

double Frame::timestamp() const {
  return frame_ref_->getFrameTimestamp();
}

const char* Frame::getFrameMetadata(FrameMetadataValue frame_metadata) const {
  return frame_ref_->getFrameMetadata(frame_metadata);
}

size_t Frame::getFrameMetadataSize() const {
  return frame_ref_->getFrameDataSize();
}

bool Frame::supportsFrameMetadata(FrameMetadataValue frame_metadata) const {
  return frame_ref_->supportsFrameMetadata(frame_metadata);
}

int64_t Frame::getFrameIndex() const {
  return frame_ref_->getFrameIndex();
}

size_t Frame::dataSize() const {
  return frame_ref_->getFrameDataSize();
}

const char *Frame::data() const {
  return frame_ref_->getFrameData();
}

void Frame::addRef() const {
  frame_ref_->acquire();
}

void Frame::reset() {
  if (frame_ref_) {
    frame_ref_->release();
  }
}

Frame FrameSet::operator[](size_t index) const {
  if (index < size()) {
    auto frame_interface = dynamic_cast<libsmartereye2::CompositeFrameData *>(get())->getFrame(index);
    return Frame(frame_interface);
  }
  throw std::runtime_error("Requested index is out of range!");
}

VideoFrame FrameSet::operator[](FrameId frame_id) const {
  auto frame = firstOrDefault(frame_id);
  if (!frame) {
    throw std::runtime_error("Frame of requested stream type was not found!");
  }
  return frame.as<VideoFrame>();
}

Frame FrameSet::firstOrDefault(FrameId frame_id, FrameFormat format) const {
  Frame result;
  for (size_t i = 0; i < size(); i++) {
    auto cur_frame = dynamic_cast<libsmartereye2::CompositeFrameData *>(get())->getFrame(i);
    cur_frame->acquire();
    Frame frame(cur_frame);
    if (!result && frame_id == frame.getProfile().frameId()
        && (format == FrameFormat::Any || format == frame.getProfile().format())) {
      result = std::move(frame);
      break;
    }
  }
  return result;
}

int VideoFrame::width() const {
  return dynamic_cast<libsmartereye2::VideoFrameData *>(get())->width();
}

int VideoFrame::height() const {
  return dynamic_cast<libsmartereye2::VideoFrameData *>(get())->height();
}

int VideoFrame::strideInBytes() const {
  return dynamic_cast<libsmartereye2::VideoFrameData *>(get())->stride();
}
int VideoFrame::bitsPerPixel() const {
  return dynamic_cast<libsmartereye2::VideoFrameData *>(get())->bpp();
}

int VideoFrame::bytesPerPixel() const {
  return bitsPerPixel() / 8;
}

float DepthFrame::distance(int x, int y) const {
  return dynamic_cast<libsmartereye2::DepthFrameData *>(get())->distance(x, y);
}

float DepthFrame::units() const {
  return dynamic_cast<libsmartereye2::DepthFrameData *>(get())->units();
}

float DisparityFrame::baseline() const {
  return dynamic_cast<libsmartereye2::DisparityData *>(get())->stereoBaseline();
}

Points::Points(const Frame &frame)
    : Frame(frame), size_(0) {
  if (frame_ref_) {
    size_ = dynamic_cast<libsmartereye2::PointsData *>(get())->vertexCount();
  }
}

const Vertex *Points::vertices() const {
  return dynamic_cast<libsmartereye2::PointsData *>(get())->vertex();
}

const TextureCoordinate *Points::textureCoordinate() const {
  return dynamic_cast<libsmartereye2::PointsData *>(get())->textureCoordinate();
}

void Points::exportToPly(const std::string &fnmae, VideoFrame texture) const {
  SeFrame *ptr = nullptr;
  std::swap(texture.frame_ref_, ptr);
  dynamic_cast<libsmartereye2::PointsData *>(get())->exportToPly(fnmae, libsmartereye2::FrameHolder(ptr));
}

SeVector3f MotionFrame::getMotionData() const {
  auto float_data = reinterpret_cast<const float *>(data());
  return SeVector3f{float_data[0], float_data[1], float_data[2]};
}

}  // namespace se2
