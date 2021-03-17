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

#include "gemini_serial_port.h"

#include "gemini_device.h"
#include "gemini_sensor.h"
#include "core/frame_data.h"
#include "serial/serial.h"
#include "tlv_data.h"
#include "easylogging++.h"

#include "alg/LdwDataInterface.h"
#include "alg/obstacleData.h"
#include "alg/algorithmresult.h"
#include "alg/calibrationparams.h"

#include "streaming/stream_profile.h"
#include "se_types.hpp"

namespace libsmartereye2 {

GeminiSerialPort::GeminiSerialPort(GeminiSensor *owner)
    : sensor_owner_(owner),
      watchdog_(nullptr),
      working_state_(WorkingState::Disconnected),
      write_dispatcher_(std::make_shared<Dispatcher>(1)),
      serial_running_(false),
      speed_(0) {
  init();
}

void GeminiSerialPort::init() {
  watchdog_ = std::make_shared<Watchdog>([this]() {
    LOG(DEBUG) << "onDisconnected by Watchdog";
    serial_->flush();
    sensor_owner_->sendOpenCamCommand();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait for device open
    offerHand();  // reconnect
  }, 5000);

  auto obstacle_profile = std::make_shared<VideoStreamProfilePrivate>();
  obstacle_profile->setDims(1280, 720);
  obstacle_profile->setFrameId(FrameId::Obstacle);
  obstacle_profile->setFormat(FrameFormat::Custom);
  obstacle_profile->setFrameRate(25);
  obstacle_profile->setUniqueId(Environment::instance().generateStreamId());
  obstacle_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto lane_profile = std::make_shared<VideoStreamProfilePrivate>();
  lane_profile->setDims(1280, 720);
  lane_profile->setFrameId(FrameId::Lane);
  lane_profile->setFormat(FrameFormat::Custom);
  lane_profile->setFrameRate(25);
  lane_profile->setUniqueId(Environment::instance().generateStreamId());
  lane_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto free_space_profile = std::make_shared<VideoStreamProfilePrivate>();
  free_space_profile->setDims(1280, 720);
  free_space_profile->setFrameId(FrameId::FreeSpace);
  free_space_profile->setFormat(FrameFormat::Custom);
  free_space_profile->setFrameRate(25);
  free_space_profile->setUniqueId(Environment::instance().generateStreamId());
  free_space_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto small_obstacle_profile = std::make_shared<VideoStreamProfilePrivate>();
  small_obstacle_profile->setDims(1280, 720);
  small_obstacle_profile->setFrameId(FrameId::SmallObstacle);
  small_obstacle_profile->setFormat(FrameFormat::Custom);
  small_obstacle_profile->setFrameRate(25);
  small_obstacle_profile->setUniqueId(Environment::instance().generateStreamId());
  small_obstacle_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto j2_profile = std::make_shared<VideoStreamProfilePrivate>();
  j2_profile->setDims(1280, 720);
  j2_profile->setFrameId(FrameId::J2Perception);
  j2_profile->setFormat(FrameFormat::Custom);
  j2_profile->setFrameRate(25);
  j2_profile->setUniqueId(Environment::instance().generateStreamId());
  j2_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto tsr_profile = std::make_shared<VideoStreamProfilePrivate>();
  tsr_profile->setDims(1280, 720);
  tsr_profile->setFrameId(FrameId::TrafficSign);
  tsr_profile->setFormat(FrameFormat::Custom);
  tsr_profile->setFrameRate(25);
  tsr_profile->setUniqueId(Environment::instance().generateStreamId());
  tsr_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  auto tfl_profile = std::make_shared<VideoStreamProfilePrivate>();
  tfl_profile->setDims(1280, 720);
  tfl_profile->setFrameId(FrameId::TrafficLight);
  tfl_profile->setFormat(FrameFormat::Custom);
  tfl_profile->setFrameRate(25);
  tfl_profile->setUniqueId(Environment::instance().generateStreamId());
  tfl_profile->tagProfile(ProfileTag::PROFILE_TAG_SUPERSET);

  profiles_[SeExtension::EXTENSION_OBSTACLE_FRAME] = obstacle_profile;
  profiles_[SeExtension::EXTENSION_LANE_FRAME] = lane_profile;
  profiles_[SeExtension::EXTENSION_FREESPACE_FRAME] = free_space_profile;
  profiles_[SeExtension::EXTENSION_SMALL_OBS_FRAME] = small_obstacle_profile;
  profiles_[SeExtension::EXTENSION_JOURNEY_FRAME] = j2_profile;
  profiles_[SeExtension::EXTENSION_TRAFFIC_SIGN_FRAME] = tsr_profile;
  profiles_[SeExtension::EXTENSION_TRAFFIC_LIGHT_FRAME] = tfl_profile;
}

void GeminiSerialPort::open() {
  auto gemini_device = dynamic_cast<GeminiDevice *>(&sensor_owner_->getDevice());

  for (const auto &port_entry : serial::list_ports()) {
    auto port_details = port_entry.hardware_id;
    int vid = 0;
    int pid = 0;
    auto pos = std::string::npos;
    pos = port_details.find("VID_");
    if (pos != std::string::npos) {
      std::string vid_str = port_details.substr(pos + 4, 4);
      vid = std::stoi(vid_str, nullptr, 16);
    }
    pos = port_details.find("PID_");
    if (pos != std::string::npos) {
      std::string pid_str = port_details.substr(pos + 4, 4);
      pid = std::stoi(pid_str, nullptr, 16);
    }
    if (vid == gemini_device->usb_info_.vid && pid == gemini_device->usb_info_.pid) {
      serial_ = std::make_shared<serial::Serial>(port_entry.port, 128000, serial::Timeout::simpleTimeout(1000));
      std::cout << "Is the serial port open?";
      if (serial_->isOpen())
        std::cout << " Yes." << std::endl;
      else
        std::cout << " No." << std::endl;
    }
    std::cout << ": " << port_entry.port << ", " << port_entry.description << ", " << port_entry.hardware_id
              << std::endl;
  }
  serial_->flush();

  write_dispatcher_->start();
  connect();
}

void GeminiSerialPort::close() {
  disconnect();

  watchdog_->stop();

  serial_running_ = false;
  if (recv_thread_.joinable()) {
    recv_thread_.join();
  }

  write_dispatcher_->flush();
  write_dispatcher_->stop();

  serial_->close();
  serial_.reset();
}

void GeminiSerialPort::requirePerception() {
  send(SerialCommand_RequirePerception);
}

void GeminiSerialPort::requireUserFiles() {
  send(SerialCommand_RequireUserFiles);
}

void GeminiSerialPort::send(uint32_t command_type, const char *data, uint32_t data_size) {
  write_dispatcher_->invoke([=](Dispatcher::CancellableTimer timer) {
    std::string buffer;
    buffer.resize(sizeof(TLVStruct) + data_size);
    auto *cmd = (TLVStruct *) buffer.data();
    cmd->type = command_type;
    cmd->length = data_size;
    if (data_size > 0) {
      memcpy(cmd->data, data, data_size);
    }
    try {
      serial_->write(buffer);
    } catch (serial::IOException &e) {
      LOG(DEBUG) << "serial write error: " << e.what();
    }
  }, true);
}

void GeminiSerialPort::handleTlvData(uint32_t type, const uint8_t *data, uint32_t data_size) {
  switch (working_state_) {
    case WorkingState::Disconnected: {
      LOG(ERROR) << "why received data before connected?";
      break;
    }
    case WorkingState::Syncing: {
      if (type != SerialConnection_Sync && type != SerialConnection_Ack) {
        LOG(ERROR) << "waiting for sync and ack, but received wrong type " << type;
        offerHand();  // wrong response while syncing, offerhand again
      }
      break;
    }
    default:break;
  }

  if (type >= SerialDataUnit_None) {
    handleDataUnit(type, data, data_size);
  } else if (type >= SerialCommand_None) {
    handleCommand(type, data, data_size);
  } else {
    handleConnection(type, data, data_size);
  }
}

void GeminiSerialPort::offerHand() {
  // handshake 1: send sync to server, wait sync response
  working_state_ = WorkingState::Syncing;

  TLVStruct conn_cmd;
  conn_cmd.type = SerialConnection_Sync;
  std::string conn_data;
  conn_data.append((char *) &kSerialConnectionToken, 4);
  conn_data.append((char *) &conn_cmd, sizeof(TLVStruct));
  serial_->write(conn_data);
}

void GeminiSerialPort::connect() {
  serial_running_ = true;
  recv_thread_ = std::thread([this]() {
    size_t nread = 0;
    std::string recv_buffer;
    TLVStruct tlv_head;
    bool is_head_found = false;
    int pack_read = 0;
    int pack_lack = 0;

    auto check_head_valid_func = [](const TLVStruct &tlv_head) -> bool {
      bool invalid = (tlv_head.type < kMinValidTypeValue
          || tlv_head.type > kMaxValidTypeValue
          || tlv_head.length > kMaxTlvDataLength);
      return !invalid;
    };

    while (serial_running_) {
      try {
        if (!is_head_found) {
          nread = serial_->read((uint8_t *) &tlv_head, sizeof(TLVStruct));
          if (nread != sizeof(TLVStruct)) {
            LOG(DEBUG) << "read TLV head timeout";
            retry();
            continue;
          } else if (check_head_valid_func(tlv_head)) {
            is_head_found = true;
          } else {
            continue;
          }
        }

        // head found
        if (recv_buffer.size() < tlv_head.length) {
          recv_buffer.resize(tlv_head.length);
        }

        if (pack_lack > 0) {
          nread = serial_->read((uint8_t *) recv_buffer.data() + pack_read, pack_lack);
          pack_read += nread;
          pack_lack -= nread;
        } else {
          nread = serial_->read((uint8_t *) recv_buffer.data(), tlv_head.length);
          if (nread < tlv_head.length) {
            pack_read = nread;
            pack_lack = tlv_head.length - nread;
          }
        }

        if (pack_lack > 0) continue;

        // pack ready
        pack_read = 0;
        is_head_found = false;
        handleTlvData(tlv_head.type, (uint8_t *) recv_buffer.data(), tlv_head.length);
      } catch (serial::IOException &e) {
        LOG(WARNING) << "serial receive exception: " << e.what();
      }
    }
  });

  // wait for recv thread start
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  offerHand();
}

void GeminiSerialPort::disconnect() {
  send(SerialConnection_Disconnect);
}

void GeminiSerialPort::retry() {
  switch (working_state_) {
    case WorkingState::Disconnected: {
      LOG(ERROR) << "why received data before connected?";
      break;
    }
    case WorkingState::Syncing: {
      static uint32_t retry_times = 0;
      if (++retry_times > 5) {
        // sync failed, offer hand for reconnect...
        retry_times = 0;
        offerHand();
      }
      break;
    }
    case WorkingState::Connected: {
      LOG(INFO) << "GeminiSerialPort: send fix Heartbeat";
      send(SerialConnection_Heartbeat);
      break;
    }
    default:break;
  }
}

void GeminiSerialPort::onSycing() {
// handshake 2: received sync response from server
  if (working_state_ == WorkingState::Syncing) {
    LOG(INFO) << "received sync response, send ack to server";
  }
}

void GeminiSerialPort::onConnected() {
  if (working_state_ == WorkingState::Connected) return;

  working_state_ = WorkingState::Connected;
  LOG(INFO) << "serial port connected";

  // handshake 3: send ack to server
  send(SerialConnection_Ack);
  watchdog_->start();

  // when connected, request stereo calib params first...
  send(SerialCommand_RequireStereoCalibParams);
  std::this_thread::sleep_for(std::chrono::microseconds(10));
  requireUserFiles();
}

void GeminiSerialPort::onHeartbeat() {
//  LOG(INFO) << "<<< tick";
  send(SerialConnection_Heartbeat);
//  LOG(INFO) << "tock >>>";
  watchdog_->kick();
}

void GeminiSerialPort::onDisconnected() {
  working_state_ = WorkingState::Disconnected;
  LOG(INFO) << "serial port disconnected";
}

void GeminiSerialPort::handleConnection(uint32_t type, const uint8_t *data, uint32_t data_size) {
  switch (type) {
    case SerialConnection_Sync:onSycing();
      break;
    case SerialConnection_Ack:onConnected();
      break;
    case SerialConnection_Heartbeat:onHeartbeat();
      break;
    case SerialConnection_Disconnect:onDisconnected();
      break;
    default:break;
  }
}

void GeminiSerialPort::handleCommand(uint32_t type, const uint8_t *data, uint32_t data_size) {
  switch (type) {
    case SerialCommand_RespondStereoCalibParams: {
      if (data_size == sizeof(StereoCalibrationParameters)) {
        auto *calib_params = (StereoCalibrationParameters *) data;
        for (const auto &kvp : profiles_) {
          auto video_stream_profile = dynamic_cast<VideoStreamProfileInterface *>(kvp.second.get());
          if (video_stream_profile != nullptr) {
            video_stream_profile->setStereoCalibParams(*calib_params);
          }
        }
        for (const auto &kvp : sensor_owner_->frame_id_to_profile_) {
          auto video_stream_profile = dynamic_cast<VideoStreamProfileInterface *>(kvp.second.get());
          if (video_stream_profile != nullptr) {
            video_stream_profile->setStereoCalibParams(*calib_params);
          }
        }
      } else {
        LOG(WARNING) << "SerialCommand_RespondStereoCalibParams error";
      }
    }
      break;
    default:break;
  }
}

void GeminiSerialPort::handleDataUnit(uint32_t type, const uint8_t *data, uint32_t data_size) {
  switch (type) {
    case SerialDataUnit_FileHeader:
    case SerialDataUnit_FileData:
    case SerialDataUnit_FileTail: {
      receiveFile(type, data, data_size);
    }
      break;
    case SerialDataUnit_Speed: {
      int speed = *(int *) data;
      speed_ = static_cast<int64_t>(speed);
//      LOG(INFO) << "speed: " << speed_ << "... ";
    }
      break;
    case SerialDataUnit_J2Perception: {
//      LOG(INFO) << "SerialDataUnit_J2Perception: " << data_size;
      FrameExtension frame_ext;
      frame_ext.speed = speed_;
      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_JOURNEY_FRAME,
                                                                         data_size, frame_ext, true));
      if (frame_holder.frame) {
        auto journey = reinterpret_cast<JourneyFrameData *>(frame_holder.frame);
        journey->setTimestamp(0); // TODO
        journey->setTimestampDomain(TimestampDomain::SYSTEM_TIME);
        journey->setStreamProfile(profiles_[SeExtension::EXTENSION_JOURNEY_FRAME]);
        journey->setSensor(sensor_owner_->shared_from_this());
        journey->data().resize(data_size, 0);
        journey->data().assign(data, data + data_size);
        sensor_owner_->dispatch_threaded(std::move(frame_holder));
      }
    }
      break;
    case SerialDataUnit_Obstacle: {
      FrameExtension frame_ext;
      frame_ext.speed = speed_;
      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_OBSTACLE_FRAME,
                                                                         data_size, frame_ext, true));
      if (frame_holder.frame) {
        auto obstacle_frame = reinterpret_cast<ObstacleFrameData *>(frame_holder.frame);
        obstacle_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_OBSTACLE_FRAME]);
        obstacle_frame->setSensor(sensor_owner_->shared_from_this());
        obstacle_frame->data().resize(data_size, 0);
        obstacle_frame->data().assign(data, data + data_size);
        obstacle_frame->loadObstacles(data, data_size);
        sensor_owner_->dispatch_threaded(std::move(frame_holder));
      }
    }
      break;
    case SerialDataUnit_Lane: {
        FrameExtension frame_ext;
        frame_ext.speed = speed_;
        FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_LANE_FRAME,
                                                                           data_size, frame_ext, true));
        if (frame_holder.frame) {
          auto lane_frame = reinterpret_cast<LaneFrameData *>(frame_holder.frame);
          lane_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_LANE_FRAME]);
          lane_frame->setSensor(sensor_owner_->shared_from_this());
          lane_frame->data().resize(data_size, 0);
          lane_frame->data().assign(data, data + data_size);
          sensor_owner_->dispatch_threaded(std::move(frame_holder));
        }
    }
      break;
    case SerialDataUnit_FreeSpace: {
      FrameExtension frame_ext;
      frame_ext.speed = speed_;
      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_FREESPACE_FRAME,
                                                                         data_size, frame_ext, true));
      if (frame_holder.frame) {
        auto free_space_frame = reinterpret_cast<FreeSpaceFrameData *>(frame_holder.frame);
        free_space_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_FREESPACE_FRAME]);
        free_space_frame->setSensor(sensor_owner_->shared_from_this());
        free_space_frame->data().resize(data_size, 0);
        free_space_frame->data().assign(data, data + data_size);
        free_space_frame->loadFreeSpacePoints(data, data_size);
        sensor_owner_->dispatch_threaded(std::move(frame_holder));
      }
    }
      break;
    case SerialDataUnit_TrafficSign: {
      FrameExtension frame_ext;
      frame_ext.speed = speed_;
      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_TRAFFIC_SIGN_FRAME,
                                                                         data_size, frame_ext, true));
      if (frame_holder.frame) {
        auto free_space_frame = reinterpret_cast<FrameData *>(frame_holder.frame);
        free_space_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_TRAFFIC_SIGN_FRAME]);
        free_space_frame->setSensor(sensor_owner_->shared_from_this());
        free_space_frame->data().resize(data_size, 0);
        free_space_frame->data().assign(data, data + data_size);
        sensor_owner_->dispatch_threaded(std::move(frame_holder));
      }
    }
      break;
    case SerialDataUnit_TrafficLight: {
      FrameExtension frame_ext;
      frame_ext.speed = speed_;
      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_TRAFFIC_LIGHT_FRAME,
                                                                         data_size, frame_ext, true));
      if (frame_holder.frame) {
        auto free_space_frame = reinterpret_cast<FrameData *>(frame_holder.frame);
        free_space_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_TRAFFIC_LIGHT_FRAME]);
        free_space_frame->setSensor(sensor_owner_->shared_from_this());
        free_space_frame->data().resize(data_size, 0);
        free_space_frame->data().assign(data, data + data_size);
        sensor_owner_->dispatch_threaded(std::move(frame_holder));
      }
    }
      break;
    case SerialDataUnit_AlgorithResult: {
      auto alg_res = (AlgorithmResult *) data;
//      LOG(INFO) << "alg_res->dataType: " << alg_res->dataType;
//      LOG(INFO) << "alg_res->dataSize: " << alg_res->dataSize;
      if (alg_res->dataType == AlgorithmResult::SmallObsLabel) {
        FrameExtension frame_ext;
        frame_ext.speed = speed_;
        FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_SMALL_OBS_FRAME,
                                                                           data_size, frame_ext, true));
        if (frame_holder.frame) {
          auto small_obs_frame = reinterpret_cast<SmallObstacleFrameData *>(frame_holder.frame);
          small_obs_frame->setStreamProfile(profiles_[SeExtension::EXTENSION_SMALL_OBS_FRAME]);
          small_obs_frame->setSensor(sensor_owner_->shared_from_this());
          small_obs_frame->data().resize(alg_res->dataSize, 0);
          small_obs_frame->data().assign(alg_res->data, alg_res->data + alg_res->dataSize);
          sensor_owner_->dispatch_threaded(std::move(frame_holder));
        }
       }
    }
      break;
    default:break;
  }
}

void GeminiSerialPort::receiveFile(uint32_t type, const uint8_t *data, uint32_t data_size) {
  static std::string filename;
  static std::ofstream file_stream;
  static int received_size = 0;

  auto send_file_resp_func = [=](int received, bool finished) {
    static SerialFileResp resp;
    resp.received = received_size;
    resp.continued = !finished;
    std::string buf;
    buf.append((const char *) &resp, sizeof(SerialFileResp));
    buf.append(filename);
    send(SerialDataUnit_FileResp, buf.data(), buf.size());
  };

  switch (type) {
    case SerialDataUnit_FileHeader: {
      auto *file_header = (SerialFileHeader *) data;
      filename = file_header->fileName();
      file_stream.open(filename);
    }
      break;
    case SerialDataUnit_FileData: {
      file_stream.write(reinterpret_cast<const char *>(data), data_size);
      received_size += data_size;
      send_file_resp_func(received_size, false);
    }
      break;
    case SerialDataUnit_FileTail: {
      send_file_resp_func(received_size, true);
      LOG(INFO) << "received file " << filename << "...";
      received_size = 0;
      file_stream.close();
      filename.clear();
    }
      break;
    default:break;
  }
}

}  // namespace libsmartereye2
