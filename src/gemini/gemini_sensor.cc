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

#include "gemini_sensor.h"
#include "gemini_device.h"
#include "gemini_serial_port.h"
#include "streaming/stream_profile.h"
#include "core/frame_data.h"
#include "easylogging++.h"

static const int BUFFER_SIZE = 1024;  // Max size for control transfers

namespace libsmartereye2 {

static const FrameId kNecessaryFrameIds = (FrameId::LeftCamera | FrameId::RightCamera
    | FrameId::CalibLeftCamera | FrameId::CalibRightCamera
    | FrameId::Disparity
);

GeminiSensor::GeminiSensor(GeminiDevice *owner)
    : SensorBase("Gemini Sensor", owner) {
  LOG(DEBUG) << "Making a Gemini Sensor " << this;
  init();
}

GeminiSensor::~GeminiSensor() = default;

StreamProfiles GeminiSensor::initStreamProfiles() {
  StreamProfiles results = {};
  uint8_t buf[BUFFER_SIZE] = {0};

  auto *response = reinterpret_cast<platform::UsbCommonPackHead *>(buf);
  auto gemini_device = dynamic_cast<GeminiDevice *>(device_owner_);

  // wait for TaskRunner
  int16_t open_cam_stat = -1;
  while (open_cam_stat < 0) {
    platform::UsbStatus ret = gemini_device->control_transfer_in(platform::UsbCommand::OPEN_CAM,
                                                                 0, response, BUFFER_SIZE);

    if (ret != platform::UsbStatus::SE2_USB_STATUS_SUCCESS || response->state != 0) {
      LOG(ERROR) << "OPEN_CAM error";
      std::this_thread::sleep_for(std::chrono::seconds(5));
    } else {
      open_cam_stat = response->state;
    }
  }

  int16_t frame_info_count = -1;
  while (frame_info_count < 0) {
    platform::UsbStatus ret = gemini_device->control_transfer_in(platform::UsbCommand::QUERY_FRAME_CAP,
                                                                 0, response, BUFFER_SIZE);

    if (ret != platform::UsbStatus::SE2_USB_STATUS_SUCCESS || response->state < 0) {
      LOG(ERROR) << "QUERY_FRAME_CAP error";
      std::this_thread::sleep_for(std::chrono::seconds(5));
    } else {
      frame_info_count = response->frame_info_count;
    }
  }

  auto *frame_capacity = (platform::UsbFrameCapacity *) malloc(
      sizeof(platform::UsbFrameCapacity) + sizeof(platform::UsbFrameInfo) * frame_info_count
  );
  frame_capacity->supported_frame_count = frame_info_count;
  auto pack_data = reinterpret_cast<platform::UsbCommonPackData *>(response->data);
  memcpy(frame_capacity->frame_infos, pack_data->frame_info, sizeof(platform::UsbFrameInfo) * frame_info_count);

  supported_frame_infos_.clear();
  for (int i = 0; i < frame_capacity->supported_frame_count; i++) {
    auto frame_info = frame_capacity->frame_infos[i];
    LOG(INFO) << "frame id:" << frame_info.frame_id
              << ", frame format:" << frame_info.frame_format
              << ", frame size:" << frame_info.data_size;

    auto profile = std::make_shared<VideoStreamProfilePrivate>();
    profile->setDims(frame_info.width, frame_info.height);
    profile->setFrameId(static_cast<FrameId>(frame_info.frame_id));
    profile->setFormat(static_cast<FrameFormat>(frame_info.frame_format));
    profile->setIndex(frame_info.frame_index);
    profile->setFrameRate(25);
    profile->setUniqueId(Environment::instance().generateStreamId());
    profile->setIntrinsics(getIntrinsics());
    int tags = ProfileTag::PROFILE_TAG_DEFAULT | ProfileTag::PROFILE_TAG_SUPERSET;
    profile->tagProfile(tags);
    results.push_back(profile);

    frame_id_to_profile_[frame_info.frame_id] = profile;
    supported_frame_infos_.push_back(frame_info);
  }

  free(frame_capacity);

  return results;
}

void GeminiSensor::open(const StreamProfiles &requests) {
  LOG(DEBUG) << "Gemini Sensor open!";

  std::lock_guard<std::mutex> lock(operation_lock_);
  if (is_streaming_) {
    throw std::runtime_error("open(...) failed. Gemini device is streaming!");
  } else if (is_opened_) {
    throw std::runtime_error("open(...) failed. Gemini device already opened!");
  }

  frame_source_->init(metadata_parsers_);
  frame_source_->set_sensor(shared_from_this());

  // filter frameid
  StreamProfiles necessary_profiles = {};
  std::copy_if(requests.begin(),
               requests.end(),
               std::back_inserter(necessary_profiles),
               [](const std::shared_ptr<StreamProfileInterface> &profile) {
                 return (profile->frameId() & kNecessaryFrameIds);
               }
  );

  // set frame ids to device
  FrameId request_frame_ids = FrameId::NotUsed;
  for (auto &&r : necessary_profiles) {
    if (r->frameId() == FrameId::NotUsed) {
      LOG(ERROR) << "Requested wrong frame id!";
      continue;
    }
    request_frame_ids = request_frame_ids | r->frameId();
  }

  uint8_t buf[BUFFER_SIZE] = {0};
  auto *response = reinterpret_cast<platform::UsbCommonPackHead *>(buf);
  auto gemini_device = dynamic_cast<GeminiDevice *>(device_owner_);
  int ret = gemini_device->control_transfer_in(platform::UsbCommand::SET_FRAME_IDS,
                                               static_cast<int>(request_frame_ids), response, BUFFER_SIZE);
  if (ret < 0) {
    LOG(ERROR) << "SET_FRAME_IDS failed: " << libusb_error_name(ret);
    return;
  }

  active_frame_infos_.clear();
  for (auto &info : supported_frame_infos_) {
    auto frame_id = info.frame_id;
    if (frame_id & static_cast<uint16_t>(request_frame_ids)) {
      auto frame_index = info.frame_index;
      active_frame_infos_[frame_index] = info;
    }
  }

  memset(&usb_frame_group_, 0, sizeof usb_frame_group_);
  for (auto &pair : active_frame_infos_) {
    auto frame_index = pair.first;
    usb_frame_group_.frame_count++;
    usb_frame_group_.total_size += pair.second.data_size;  // include embededline if exists
    usb_frame_group_.frame_infos[frame_index] = &active_frame_infos_[frame_index];  // ordered
  }

  is_opened_ = true;
  serial_port_->open();
  setActiveStream(necessary_profiles);
}

void GeminiSensor::close() {
  LOG(DEBUG) << "Gemini Sensor close...";

  std::lock_guard<std::mutex> lock(operation_lock_);
  if (is_streaming_) {
    throw std::runtime_error("close(...) failed. Gemini device is streaming!");
  } else if (!is_opened_) {
    throw std::runtime_error("close(...) failed. Gemini device was not opened!");
  }

  active_frame_infos_.clear();
  is_opened_ = false;
  serial_port_->close();
  setActiveStream({});
}

void GeminiSensor::start(FrameCallbackPtr callback) {
  LOG(DEBUG) << "Gemini Sensor start...";

  std::lock_guard<std::mutex> lock(operation_lock_);
  data_dispatcher_->start();

  if (is_streaming_) {
    throw std::runtime_error("start(...) failed. Gemini device is already streaming!");
  } else if (!is_opened_) {
    throw std::runtime_error("start(...) failed. Gemini device was not opened!");
  }

  frame_source_->set_callback(callback);
  startStream();
}

void GeminiSensor::stop() {
  LOG(DEBUG) << "Gemini Sensor stop...";

  std::lock_guard<std::mutex> lock(operation_lock_);
  data_dispatcher_->flush();
  data_dispatcher_->stop();

  if (!is_streaming_) {
    throw std::runtime_error("stop(...) failed. Gemini device is not streaming!");
  }

  stopStream();
}

Intrinsics GeminiSensor::getIntrinsics() const {
  // TODO
  return {};
}

void GeminiSensor::init() {
  profiles_ = initStreamProfiles();
  device_owner_->tagProfiles(profiles_);
  frame_source_->set_max_publish_list_size(256);
  data_dispatcher_ = std::make_shared<Dispatcher>(256);

  serial_port_ = std::make_shared<GeminiSerialPort>(this);
}

void GeminiSensor::dispose() {
  // TODO
}

void GeminiSensor::dispatch_threaded(FrameHolder frame) {
  auto frame_holder_ptr = std::make_shared<FrameHolder>();
  *frame_holder_ptr = std::move(frame);
  data_dispatcher_->invoke([this, frame_holder_ptr](Dispatcher::CancellableTimer timer) {
    frame_source_->invoke_callback(std::move(*frame_holder_ptr));
  });
}

bool GeminiSensor::startStream() {
  is_streaming_ = true;

  auto suitable_buffer_size = usb_frame_group_.total_size + sizeof(platform::UsbCommonPackHead);  // over head
  buffer_.resize(suitable_buffer_size);
  std::fill(buffer_.begin(), buffer_.end(), 0);

  stream_thread_ = std::thread([this] {
    auto gemini_device = dynamic_cast<GeminiDevice *>(device_owner_);
    int ret = 0;
    int missing_cnt = 0;
    const int kMissingFrameThreshold = 25; // 25 frames for 1 sec

    while (is_streaming_) {
      auto stream_response = (platform::UsbCommonPackHead *) buffer_.data();
      ret = gemini_device->stream_read(*stream_response);
      if (ret != 0) {
        missing_cnt++;
        if (missing_cnt > kMissingFrameThreshold) {
          gemini_device->hardwareReset();
          gemini_device->setValid(false);
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        continue;
      }

      if (missing_cnt > 0) {
        missing_cnt = 0;
        gemini_device->setValid(true);
      }

      uint8_t *img_buf_ptr = buffer_.data() + sizeof(platform::UsbCommonPackHead);
      usb_frame_group_.timestamp = *((int64_t *) img_buf_ptr);
      img_buf_ptr += sizeof(int64_t);

      for (auto &pair : active_frame_infos_) {
        // parse data one by one
        auto index = pair.first;
        auto frame_info = usb_frame_group_.frame_infos[index];
        if (frame_info == nullptr) {
          LOG(ERROR) << "active_frame_infos is not valide";
          break;
        }
        usb_frame_group_.frame_datas[index] = img_buf_ptr;
        img_buf_ptr += frame_info->data_size;
      }

      handle_received_frames();
    }
  });
  return true;
}

void GeminiSensor::stopStream() {
  is_streaming_ = false;
  if (stream_thread_.joinable()) {
    stream_thread_.join();
  }
}

void GeminiSensor::handle_received_frames() {
  auto timestamp = usb_frame_group_.timestamp;

  for (auto &pair : active_frame_infos_) {
    auto index = pair.first;
    auto info = usb_frame_group_.frame_infos[index];
    auto frame_id = static_cast<FrameId>(info->frame_id);
    auto frame_format = static_cast<FrameFormat>(info->frame_format);
    auto frame_index = info->frame_index;
    auto data_ptr = usb_frame_group_.frame_datas[index];

    auto profile = frame_id_to_profile_[static_cast<int>(frame_id)];
    if (profile == nullptr) {
      LOG(WARNING) << "Dropped frame. No valid profile";
      continue;
    }

    bool with_embededline = (frame_id == FrameId::LeftCamera || frame_id == FrameId::RightCamera
        || frame_id == FrameId::CalibLeftCamera || frame_id == FrameId::CalibRightCamera);

    FrameExtension frame_ext;
    frame_ext.index = frame_index;
    frame_ext.speed = serial_port_->speed_;
    FrameHolder frame_holder(frame_source_->alloc_frame(SeExtension::EXTENSION_VIDEO_FRAME,
                                                        info->data_size, frame_ext, true));
    if (frame_holder.frame) {
      auto video = reinterpret_cast<VideoFrameData *>(frame_holder.frame);
      video->assign(info->width, info->height, 0, getBppByFormat(frame_format));
      video->setTimestamp(timestamp);
      video->setTimestampDomain(TimestampDomain::SYSTEM_TIME);
      video->setStreamProfile(profile);
      video->setSensor(shared_from_this());
      if (with_embededline) {
        auto raw_frame_with_embededline = reinterpret_cast<RawUsbImageFrame4Embededline *>(data_ptr);
        auto embededline_size = sizeof(raw_frame_with_embededline->embededline);

        // record j2 timestamp
        if (frame_id == FrameId::LeftCamera) {
          // get j2counter from embededline first 4 bytes
          uint32_t j2counter = *reinterpret_cast<uint32_t *>(raw_frame_with_embededline->embededline);
          j2counter_queue_.push(j2counter);
          j2counter_to_timestamp_[j2counter] = timestamp;

          if (j2counter_to_timestamp_.size() > 25) {
            j2counter_to_timestamp_.erase(j2counter_queue_.front());
            j2counter_queue_.pop();
          }
        }

        video->extension().metadata_value = FrameMetadataValue::EmbededLine;
        video->extension().metadata_blob.resize(embededline_size);
        video->extension().metadata_blob.assign(raw_frame_with_embededline->embededline,
                                                raw_frame_with_embededline->embededline + embededline_size);

        auto img_size = info->data_size - 1280;
        video->loadData(raw_frame_with_embededline->image, img_size);
      } else {
        auto raw_frame = reinterpret_cast<RawUsbImageFrame *>(data_ptr);
        video->loadData(raw_frame->image, info->data_size);
      }
    } else {
      LOG(WARNING) << "Dropped frame. alloc_frame(...) returned nullptr";
      continue;
    }

    dispatch_threaded(std::move(frame_holder));
  }
}

void GeminiSensor::sendOpenCamCommand() {
  std::unique_ptr<platform::UsbCommonPackHead> response(new platform::UsbCommonPackHead);
  dynamic_cast<GeminiDevice *>(device_owner_)->control_transfer_in(platform::UsbCommand::OPEN_CAM,
                                                                   0, response.get(), sizeof(*response));
}

}  // namespace libsmartereye2
