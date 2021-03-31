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

#ifndef GEMINI_SERIAL_PORT_H
#define GEMINI_SERIAL_PORT_H

#include <map>
#include <thread>
#include <atomic>

#include "se_types.hpp"
#include "core/core_types.hpp"

namespace serial {
class Serial;
}

namespace libsmartereye2 {

class GeminiSensor;
class Dispatcher;
class Watchdog;
class StreamProfileBase;
struct TLVStruct;

class GeminiSerialPort {
 public:
  enum class WorkingState{
    Disconnected,
    Syncing,
    Connected,
  };

  explicit GeminiSerialPort(GeminiSensor *owner);
  void init();

  void open();
  void close();

  void requirePerception();
  void requireUserFiles();

 private:
  void send(uint32_t command_type, const char *data = nullptr, uint32_t data_size = 0);
  void handleTlvData(uint32_t type, const uint8_t *data, uint32_t data_size);
  void offerHand();

  void connect();
  void disconnect();
  void retry();

  void onSyncing();
  void onConnected();
  void onHeartbeat();
  void onDisconnected();

  void handleConnection(uint32_t type, const uint8_t *data, uint32_t data_size);
  void handleCommand(uint32_t type, const uint8_t *data, uint32_t data_size);
  void handleDataUnit(uint32_t type, const uint8_t *data, uint32_t data_size);

  void receiveFile(uint32_t type, const uint8_t *data, uint32_t data_size);

  friend class GeminiSensor;
  GeminiSensor *sensor_owner_;

  std::shared_ptr<Watchdog> watchdog_;
  WorkingState working_state_;

  // virtual COM
  std::shared_ptr<serial::Serial> serial_;
  std::shared_ptr<Dispatcher> write_dispatcher_;
  std::thread recv_thread_;
  bool serial_running_;

  std::atomic<int64_t> speed_;
  std::map<SeExtension, std::shared_ptr<StreamProfileBase>> profiles_;
};

}  // namespace libsmartereye2

#endif //GEMINI_SERIAL_PORT_H
