//
// Created by xtp on 2021/1/20.
//

#include "gemini_serial_port.h"

#include "gemini_device.h"
#include "gemini_sensor.h"
#include "core/frame_data.h"
#include "serial/serial.h"
#include "tlv_data.h"
#include "easylogging++.h"

namespace libsmartereye2 {

GeminiSerialPort::GeminiSerialPort(GeminiSensor *owner)
    : sensor_owner_(owner),
      watchdog_(nullptr),
      is_connecting_(false),
      serial_running_(false),
      write_dispatcher_(std::make_shared<Dispatcher>(1)) {
  watchdog_ = std::make_shared<Watchdog>([this]() {
    onDisconnected();
  }, 5000);
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

  write_dispatcher_->start();
  connect();
}

void GeminiSerialPort::close() {
  disconnect();
  onDisconnected();

  serial_running_ = false;
  if (recv_thread_.joinable()) {
    recv_thread_.join();
  }

  write_dispatcher_->flush();
  write_dispatcher_->stop();
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
    serial_->write(buffer);
  });
}

void GeminiSerialPort::handleTlvData(uint32_t type, const uint8_t *data, uint32_t data_size) {
  if (type >= SerialDataUnit_None) {
    handleDataUnit(type, data, data_size);
  } else if (type >= SerialCommand_None) {
    handleCommand(type, data, data_size);
  } else {
    handleConnection(type, data, data_size);
  }
}

void GeminiSerialPort::connect() {
  // handshake 1: send sync to server
  TLVStruct conn_cmd;
  conn_cmd.type = SerialConnection_Sync;
  std::string conn_data;
  conn_data.append((char *) &kSerialConnectionToken, 4);
  conn_data.append((char *) &conn_cmd, sizeof(TLVStruct));
  serial_->write(conn_data);

  serial_running_ = true;
  recv_thread_ = std::thread([this]() {
    size_t nread = 0;
    std::string recv_buffer;
    TLVStruct tlv_head;
    bool is_head_found = false;
    int pack_read = 0;
    int pack_lack = 0;

    while (serial_running_) {
      if (!is_head_found) {
        nread = serial_->read((uint8_t *) &tlv_head, sizeof(TLVStruct));
        if (nread == sizeof(TLVStruct)) {
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
    }
  });
}

void GeminiSerialPort::disconnect() {
  send(SerialConnection_Disconnect);
}

void GeminiSerialPort::onConnected() {
  // handshake 2: recv sync from server
  is_connecting_ = true;
  LOG(INFO) << "onConnected!!!";

  // handshake 3: send ack to server
  send(SerialConnection_Heartbeat);
  watchdog_->start();

  send(SerialCommand_RequireUserFiles);
}

void GeminiSerialPort::onHeartbeat() {
  LOG(INFO) << "<<< tick";
  send(SerialConnection_Heartbeat);
  LOG(INFO) << "tock >>>";
  watchdog_->kick();
}

void GeminiSerialPort::onDisconnected() {
  is_connecting_ = false;
  LOG(INFO) << "onDisconnected";
  watchdog_->stop();
}

void GeminiSerialPort::handleConnection(uint32_t type, const uint8_t *data, uint32_t data_size) {
  switch (type) {
    case SerialConnection_Sync:onConnected();
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
      double speed = *(double *) data;
      speed_ = static_cast<int64_t>(speed);
      LOG(INFO) << "speed: " << speed_ << "... ";
    }
      break;
    case SerialDataUnit_J2Perception: {
      LOG(INFO) << "SerialDataUnit_J2Perception: " << data_size;
//      FrameExtension frame_ext;
//      frame_ext.speed = speed_;
//      FrameHolder frame_holder(sensor_owner_->frame_source_->alloc_frame(SeExtension::EXTENSION_JOURNEY_FRAME,
//                                                                         data_size, frame_ext, true));
//      if (frame_holder.frame) {
//        auto journey = reinterpret_cast<JourneyFrameData *>(frame_holder.frame);
//        journey->data().resize(data_size, 0);
//        journey->data().assign(data, data + data_size);
//        sensor_owner_->dispatch_threaded(std::move(frame_holder));
//      }
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
