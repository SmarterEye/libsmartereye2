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

#include "frame_data.h"
#include "streaming/streaming.h"
#include "streaming/stream_profile.h"

namespace libsmartereye2 {

std::shared_ptr<ArchiveInterface> makeArchive(SeExtension extension_type,
                                              std::atomic<uint32_t> *in_max_frame_queue_size,
                                              const std::shared_ptr<platform::TimeService> &ts,
                                              const std::shared_ptr<MetadataParserMap> &parsers) {
  switch (extension_type) {
    case SeExtension::EXTENSION_VIDEO_FRAME:
      return std::make_shared<FrameArchive<VideoFrameData>>(in_max_frame_queue_size,
                                                            ts,
                                                            parsers);

    case SeExtension::EXTENSION_COMPOSITE_FRAME:
      return std::make_shared<FrameArchive<CompositeFrameData>>(in_max_frame_queue_size,
                                                                ts,
                                                                parsers);

    case SeExtension::EXTENSION_MOTION_FRAME:
      return std::make_shared<FrameArchive<MotionFrameData>>(in_max_frame_queue_size,
                                                             ts,
                                                             parsers);

    case SeExtension::EXTENSION_POINTS:
      return std::make_shared<FrameArchive<PointsData>>(in_max_frame_queue_size,
                                                        ts,
                                                        parsers);

    case SeExtension::EXTENSION_DEPTH_FRAME:
      return std::make_shared<FrameArchive<DepthFrameData>>(in_max_frame_queue_size,
                                                            ts,
                                                            parsers);

    case SeExtension::EXTENSION_POSE_FRAME:
      return std::make_shared<FrameArchive<PoseData>>(in_max_frame_queue_size,
                                                      ts,
                                                      parsers);

    case SeExtension::EXTENSION_DISPARITY_FRAME:
      return std::make_shared<FrameArchive<DisparityData>>(in_max_frame_queue_size,
                                                           ts,
                                                           parsers);

    case SeExtension::EXTENSION_JOURNEY_FRAME:
      return std::make_shared<FrameArchive<JourneyFrameData>>(in_max_frame_queue_size,
                                                              ts,
                                                              parsers);

    case SeExtension::EXTENSION_OBSTACLE_FRAME:
      return std::make_shared<FrameArchive<ObstacleFrameData>>(in_max_frame_queue_size,
                                                               ts,
                                                               parsers);

    case SeExtension::EXTENSION_LANE_FRAME:
      return std::make_shared<FrameArchive<LaneFrameData>>(in_max_frame_queue_size,
                                                           ts,
                                                           parsers);

    case SeExtension::EXTENSION_FREESPACE_FRAME:
      return std::make_shared<FrameArchive<FreeSpaceFrameData>>(in_max_frame_queue_size,
                                                                ts,
                                                                parsers);

    case SeExtension::EXTENSION_SMALL_OBS_FRAME:
      return std::make_shared<FrameArchive<SmallObstacleFrameData>>(in_max_frame_queue_size,
                                                                    ts,
                                                                    parsers);
    case SeExtension::EXTENSION_TRAFFIC_SIGN_FRAME:
      return std::make_shared<FrameArchive<TrafficSignFrameData>>(in_max_frame_queue_size,
                                                                  ts,
                                                                  parsers);
    case SeExtension::EXTENSION_TRAFFIC_LIGHT_FRAME:
      return std::make_shared<FrameArchive<TrafficLightFrameData>>(in_max_frame_queue_size,
                                                                   ts,
                                                                   parsers);
    case SeExtension::EXTENSION_FLATNESS_FRAME:
      return std::make_shared<FrameArchive<FlatnessFrameData>>(in_max_frame_queue_size,
                                                               ts,
                                                               parsers);

    case SeExtension::EXTENSION_VEHICLE_INFO_FRAME:
      return std::make_shared<FrameArchive<VehicleInfoFrameData>>(in_max_frame_queue_size,
                                                               ts,
                                                               parsers);
    case SeExtension::EXTENSION_Matrix:
      return std::make_shared<FrameArchive<MatrixData>>(in_max_frame_queue_size,
                                                             ts,
                                                             parsers);

    default:throw std::runtime_error("Requested frame type is not supported!");
  }
}

FrameData::FrameData()
    : ref_count_(0), kept_(false), owner_(nullptr) {
}

FrameData::~FrameData() = default;

FrameData::FrameData(FrameData &&other) noexcept
    : ref_count_(other.ref_count_.exchange(0)),
      kept_(other.kept_.exchange(false)),
      owner_(other.owner_) {
  *this = std::move(other);
}

FrameData &FrameData::operator=(FrameData &&other) noexcept {
  data_ = std::move(other.data_);
  extension_data_ = std::move(other.extension_data_);
  ref_count_ = other.ref_count_.exchange(0);
  kept_ = other.kept_.exchange(false);
  owner_ = other.owner_;
  other.owner_.reset();
  return *this;
}

std::shared_ptr<SensorInterface> FrameData::getSensor() const {
  auto handle = sensor_.lock();
  if (!handle) {
    auto archive = getOwner();
    if (archive) return archive->get_sensor();
  }
  return handle;
}

void FrameData::setSensor(std::shared_ptr<SensorInterface> sensor) {
  sensor_ = sensor;
}

const char *FrameData::getFrameMetadata(const FrameMetadataValue &frame_metadata) const {
  return reinterpret_cast<const char *>(extension_data_.metadata_blob.data());
}

size_t FrameData::getFrameMetadataSize() const {
  return extension_data_.metadata_blob.size();
}

bool FrameData::supportsFrameMetadata(const FrameMetadataValue &frame_metadata) const {
  return extension_data_.metadata_value == frame_metadata;
}

size_t FrameData::getFrameDataSize() const {
  return data_.size();
}

const char *FrameData::getFrameData() const {
  return data_.data();
}

double FrameData::getFrameTimestamp() const {
  return extension_data_.timestamp;
}

TimestampDomain FrameData::getFrameTimestampDomain() const {
  return extension_data_.timestamp_domain;
}

int64_t FrameData::getFrameIndex() const {
  return extension_data_.index;
}

int64_t FrameData::getSpeed() const {
  return extension_data_.speed;
}

std::shared_ptr<StreamProfileInterface> FrameData::getStreamProfile() const {
  return stream_profile_;
}

void FrameData::setTimestamp(double new_ts) {
  extension_data_.timestamp = new_ts;
}

void FrameData::setTimestampDomain(TimestampDomain timestamp_domain) {
  extension_data_.timestamp_domain = timestamp_domain;
}

void FrameData::setStreamProfile(std::shared_ptr<StreamProfileInterface> sp) {
  stream_profile_ = sp;
}

void FrameData::acquire() {
  ref_count_.fetch_add(1);
}

void FrameData::release() {
  if (ref_count_.fetch_sub(1) == 1) {
    unpublish();
    owner_->unpublish_frame(this);
  }
}

void FrameData::keep() {
  if (!kept_.exchange(true)) {
    owner_->keep_frame(this);
  }
}

FrameInterface *FrameData::publish(std::shared_ptr<ArchiveInterface> new_owner) {
  owner_ = new_owner;
  kept_ = false;
  return owner_->publish_frame(this);
}

FrameInterface *CompositeFrameData::getFrame(size_t i) const {
  auto frames = getFrames();
  return frames[i];
}

void CompositeFrameData::release() {
  if (ref_count_.fetch_sub(1) == 1) {
    unpublish(); // not used
    auto frames = getFrames();
    for (int i = 0; i < getFrameCount(); i++) {
      if (frames[i]) {
        frames[i]->release();
      }
    }
    owner_->unpublish_frame(this);
  }
}

FrameInterface *DepthFrameData::publish(std::shared_ptr<ArchiveInterface> new_owner) {
  return VideoFrameData::publish(new_owner);
}

void DepthFrameData::keep() {
  if (original_) {
    original_->keep();
  }
  VideoFrameData::keep();
}

float DepthFrameData::distance(int x, int y) const {
  if (original_ && getStreamProfile()->format() != FrameFormat::Disparity16) {
    return (reinterpret_cast<DepthFrameData *>(original_.frame))->distance(x, y);
  }

  uint64_t pixel = 0;
  switch (bpp() / 8) // bits per pixel
  {
    case 1: pixel = getFrameData()[y * width() + x];
      break;
    case 2: pixel = reinterpret_cast<const uint16_t *>(getFrameData())[y * width() + x];
      break;
    case 4: pixel = reinterpret_cast<const uint32_t *>(getFrameData())[y * width() + x];
      break;
    case 8: pixel = reinterpret_cast<const uint64_t *>(getFrameData())[y * width() + x];
      break;
    default:
      throw std::runtime_error(toString() << "Unrecognized depth format " << int(bpp() / 8) << " bytes per pixel");
  }

  return pixel * units();
}

float DepthFrameData::units() const {
  return 0.f; // TODO
}

void DepthFrameData::setOriginal(FrameHolder holder) {
  original_ = std::move(holder);
}

float DepthFrameData::queryUnits(const std::shared_ptr<SensorInterface> &sensor) {
  return 0.f; // TODO
}

const Vertex *PointsData::vertex() const {
  getFrameData();
  const char *data = data_.data();
  const auto *vertex = reinterpret_cast<const Vertex *>(data);
  return vertex;
}

size_t PointsData::vertexCount() const {
  return data_.size() / (sizeof(Vertex) + sizeof(TextureCoordinate));
}

const TextureCoordinate *PointsData::textureCoordinate() const {
  auto xyz = vertex();
  auto ijs = (TextureCoordinate *) (xyz + vertexCount());
  return ijs;
}

void PointsData::exportToPly(const std::string &fname, const FrameHolder &texture) {

}

void JourneyFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto *meta = (SEMeta *) data;
  setTimestamp(meta->timestamp);
  if (meta_buffer_.size() < meta->data_size) {
    meta_buffer_.resize(meta->data_size);
  }
  meta_buffer_.assign(meta->data, meta->data + meta->data_size);
}

void ObstacleFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto obs_data = (SEObstacles *) data;
  setTimestamp(obs_data->timestamp);
  num_ = obs_data->obs_num;
  for (int i = 0; i < num_; i++) {
    obstacles_.push_back(std::make_shared<SEObstacle>(obs_data->obstacles[i]));
  }
}

void FreeSpaceFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto *fs_data = (SEFreeSpace *) data;
  setTimestamp(fs_data->timestamp);
  num_ = fs_data->point_num;
  for (int i = 0; i < num_; i++) {
    free_space_points_.push_back(std::make_shared<SEFreeSpacePoint>(fs_data->points[i]));
  }
}

void LaneFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto *lane_data = (SELane *) data;
  setTimestamp(lane_data->timestamp);
  num_ = lane_data->line_num;
  for (int i = 0; i < num_; i++) {
    lane_lines_.push_back(std::make_shared<SELaneLine>(lane_data->lines[i]));
  }
}

void SmallObstacleFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);
  // TODO
}

void TrafficSignFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto *tsr_data = (SETSR *) data;
  setTimestamp(tsr_data->timestamp);
  num_ = tsr_data->sign_num;
  for (int i = 0; i < num_; i++) {
    signs_.push_back(std::make_shared<SETSRData>(tsr_data->signs[i]));
  }
}

void TrafficLightFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);

  auto *tfl_data = (SETFL *) data;
  setTimestamp(tfl_data->timestamp);
  num_ = tfl_data->light_num;
  for (int i = 0; i < num_; i++) {
    lights_.push_back(std::make_shared<SETFLData>(tfl_data->lights[i]));
  }
}

void FlatnessFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);
  // TODO
}

void VehicleInfoFrameData::loadData(const uint8_t *data, uint32_t data_size) {
  FrameData::loadData(data, data_size);
  vehicle_info_ = *(VehicleInfo *) data;
}

void MatrixData::loadData(const uint8_t *data, uint32_t data_size)
{
   FrameData::loadData(data, data_size);
   auto *mat_data = (SEMatrix *) data;
   setTimestamp(mat_data->timestamp);
   num_ = mat_data->mat_num;
   for (int i = 0; i < num_; i++) {
     matrixs_.push_back(std::make_shared<SEMatrixData>(mat_data->data[i]));
   }
}

}  // namespace libsmartereye2
