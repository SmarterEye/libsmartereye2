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
#include "streaming/stream_profile.h"
#include "core/frame_data.h"
#include "easylogging++.h"

static const int BUFFER_SIZE = 1024;  // Max size for control transfers

namespace libsmartereye2 {

GeminiSensor::GeminiSensor(GeminiDevice *owner)
    : SensorBase("Gemini Sensor", owner),
      device_(owner) {
  LOG(DEBUG) << "Making a Gemini Sensor" << this;

  profiles_ = initStreamProfiles();
  device_owner_->tagProfiles(profiles_);

  frame_source_->set_max_publish_list_size(256);
  data_dispatcher_ = std::make_shared<Dispatcher>(256);
  data_dispatcher_->start();
}

GeminiSensor::~GeminiSensor() = default;

StreamProfiles GeminiSensor::initStreamProfiles() {
  StreamProfiles results;
  uint8_t buf[BUFFER_SIZE] = {0};

  auto *response = reinterpret_cast<platform::UsbCommonPackHead *>(buf);
  platform::UsbStatus ret = device_->control_transfer_in(platform::UsbCommand::QUERY_FRAME_CAP,
                                                         0, response, BUFFER_SIZE);

  if (ret != platform::UsbStatus::SE2_USB_STATUS_SUCCESS || response->state < 0) {
    LOG(ERROR) << "QUERY_FRAME_CAP error";
    return results;
  }

  auto frame_info_count = response->frame_info_count;
  auto *frame_capacity = (platform::UsbFrameCapacity *) malloc(
      sizeof(platform::UsbFrameCapacity) + sizeof(platform::UsbFrameInfo) * frame_info_count
  );
  frame_capacity->supported_frame_count = frame_info_count;
  auto pack_data = reinterpret_cast<platform::UsbCommonPackData *>(response->data);
  memcpy(frame_capacity->frame_infos, pack_data->frame_info, sizeof(platform::UsbFrameInfo) * frame_info_count);

  supported_frame_infos_.clear();
  for (int i = 0; i < frame_capacity->supported_frame_count; i++) {
    auto frame_info = frame_capacity->frame_infos[i];
    supported_frame_infos_.push_back(frame_info);
    LOG(INFO) << "frame id:" << frame_info.frame_id
              << ", frame format:" << frame_info.frame_format
              << ", frame size:" << frame_info.data_size;

    platform::BackendProfile backend_profile{frame_info.width, frame_info.height, 25, frame_info.frame_format};
    auto profile = std::make_shared<VideoStreamProfilePrivate>(backend_profile);
    profile->setDims(frame_info.width, frame_info.height);
    profile->setFrameId(static_cast<FrameId>(frame_info.frame_id));
    profile->setFormat(static_cast<FrameFormat>(frame_info.frame_format));
    profile->setIndex(frame_info.frame_index);
    profile->setFrameRate(backend_profile.fps);
    profile->setUniqueId(Environment::instance().generateStreamId());
    profile->setIntrinsics(getIntrinsics());
    profile->tagProfile(ProfileTag::PROFILE_TAG_DEFAULT | ProfileTag::PROFILE_TAG_SUPERSET);  // unused now

    results.push_back(profile);
    frame_id_to_profile_[frame_info.frame_id] = profile;
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

  // set frame ids to device
  FrameId request_frame_ids = FrameId::NotUsed;
  for (auto &&r : requests) {
    if (r->frameId() == FrameId::NotUsed) {
      LOG(ERROR) << "Requested wrong frame id!";
      continue;
    }
    request_frame_ids = request_frame_ids | r->frameId();
  }

  uint8_t buf[BUFFER_SIZE] = {0};
  auto *response = reinterpret_cast<platform::UsbCommonPackHead *>(buf);
  int ret = device_->control_transfer_in(platform::UsbCommand::SET_FRAME_IDS,
                                         static_cast<int>(request_frame_ids), response, BUFFER_SIZE);
  if (ret < 0) {
    LOG(ERROR) << "SET_FRAME_IDS failed: " << libusb_error_name(ret);
    return;
  }

  active_frame_infos_.clear();
  std::copy_if(supported_frame_infos_.begin(),
               supported_frame_infos_.end(),
               std::back_inserter(active_frame_infos_),
               [request_frame_ids](const platform::UsbFrameInfo &frame_info) {
                 return frame_info.frame_id & static_cast<uint16_t>(request_frame_ids);
               });

  memset(&usb_frame_group_, 0, sizeof usb_frame_group_);
  for (auto &active_frame_info : active_frame_infos_) {
    usb_frame_group_.frame_count++;
    usb_frame_group_.total_size += active_frame_info.data_size; // only contains image bytes data

    int index = active_frame_info.frame_index;  // ordered
    usb_frame_group_.frame_infos[index] = &active_frame_info;
  }

  is_opened_ = true;
  setActiveStream(profiles_);
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
  setActiveStream({});
}

void GeminiSensor::start(FrameCallbackPtr callback) {
  LOG(DEBUG) << "Gemini Sensor start...";

  std::lock_guard<std::mutex> lock(operation_lock_);
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
  if (!is_streaming_) {
    throw std::runtime_error("stop(...) failed. Gemini device is not streaming!");
  }

  stopStream();
}

Intrinsics GeminiSensor::getIntrinsics() const {
  // TODO
  return {};
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

  // [imbededline(1280, option) + timestamp(8) + image(..)]
  auto suitable_buffer_size = usb_frame_group_.total_size + sizeof(platform::UsbCommonPackHead);  // over head
  buffer_.resize(suitable_buffer_size);
  std::fill(buffer_.begin(), buffer_.end(), 0);

  stream_thread = std::thread([this] {
    int ret = 0;

    while (is_streaming_) {
      auto stream_response = (platform::UsbCommonPackHead *) buffer_.data();
      ret = device_->stream_read(*stream_response);
      if (ret != 0) {
        LOG(ERROR) << "bulk_transer_in error: " << ret;
        continue;
      }

      auto pack_data = reinterpret_cast<platform::UsbCommonPackData *>(stream_response->data);
      usb_frame_group_.timestamp = *((uint64_t *) pack_data->timestamp);

//      LOG(INFO) << "received usb frame group, frame number: " << usb_frame_group_.frame_count
//                << ", timestamp: " << usb_frame_group_.timestamp;

      uint8_t *img_buf_ptr = buffer_.data() + sizeof(platform::UsbCommonPackHead);
      for (int i = 0; i < usb_frame_group_.frame_count; i++) {
        // parse data one by one
        auto frame_info = usb_frame_group_.frame_infos[i];
        usb_frame_group_.frame_datas[i] = img_buf_ptr;
        img_buf_ptr += frame_info->data_size;
      }

      handle_received_frames();
    }
  });
  return true;
}

void GeminiSensor::stopStream() {
  is_streaming_ = false;
  if (stream_thread.joinable()) {
    stream_thread.join();
  }
}

void GeminiSensor::handle_received_frames() {
  for (int i = 0; i < usb_frame_group_.frame_count; i++) {
    auto timestamp = usb_frame_group_.timestamp;
    auto info = usb_frame_group_.frame_infos[i];
    auto frame_id = static_cast<FrameId>(info->frame_id);
    auto frame_format = static_cast<FrameFormat>(info->frame_format);
    auto frame_index = info->frame_index;
    auto data_ptr = usb_frame_group_.frame_datas[i];

    auto profile = frame_id_to_profile_[static_cast<int>(frame_id)];
    if (profile == nullptr) {
      LOG(WARNING) << "Dropped frame. No valid profile";
      continue;
    }

    bool with_embededline = (frame_id == FrameId::LeftCamera || frame_id == FrameId::RightCamera
        || frame_id == FrameId::CalibLeftCamera || frame_id == FrameId::CalibRightCamera);

    FrameExtension frame_ext;
    frame_ext.index = frame_index;
    FrameHolder frame_holder(frame_source_->alloc_frame(SeExtension::EXTENSION_VIDEO_FRAME,
                                                        info->data_size, frame_ext, true));
    if (frame_holder.frame) {
      auto video = reinterpret_cast<VideoFrameData *>(frame_holder.frame);
      video->assign(info->width, info->height, 0, getBppByFormat(frame_format));
      video->setTimestamp(timestamp);
      video->setTimestampDomain(TimestampDomain::SYSTEM_TIME);
      video->setStream(profile);
      video->setSensor(shared_from_this());
      video->data().resize(info->data_size, 0);
      if (with_embededline) {
        auto raw_frame_with_embededline = reinterpret_cast<RawUsbImageFrame4Embededline *>(data_ptr);
        video->extension().metadata_value = FrameMetadataValue::EmbededLine;
        video->extension().metadata_blob.resize(sizeof(raw_frame_with_embededline->embededline));
        video->extension().metadata_blob.assign(raw_frame_with_embededline->image,
                                                raw_frame_with_embededline->image + info->data_size);
        video->data().assign(raw_frame_with_embededline->image, raw_frame_with_embededline->image + info->data_size);
      } else {
        auto raw_frame = reinterpret_cast<RawUsbImageFrame *>(data_ptr);
        video->data().assign(raw_frame->image, raw_frame->image + info->data_size);
      }
    } else {
      LOG(WARNING) << "Dropped frame. alloc_frame(...) returned nullptr";
      continue;
    }

    dispatch_threaded(std::move(frame_holder));
  }
}

}  // namespace libsmartereye2