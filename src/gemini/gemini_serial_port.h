//
// Created by xtp on 2021/1/20.
//

#ifndef GEMINI_SERIAL_PORT_H
#define GEMINI_SERIAL_PORT_H

#include <thread>
#include <atomic>

namespace serial {
class Serial;
}

namespace libsmartereye2 {

class GeminiSensor;
class Dispatcher;
class Watchdog;
struct TLVStruct;

class GeminiSerialPort {
 public:
  explicit GeminiSerialPort(GeminiSensor *owner);

  void open();
  void close();

  void requirePerception();
  void requireUserFiles();

 private:
  void send(uint32_t command_type, const char *data = nullptr, uint32_t data_size = 0);
  void handleTlvData(uint32_t type, const uint8_t *data, uint32_t data_size);

  void connect();
  void disconnect();

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
  bool is_connecting_;

  // virtual COM
  std::shared_ptr<serial::Serial> serial_;
  std::shared_ptr<Dispatcher> write_dispatcher_;
  std::thread recv_thread_;
  bool serial_running_;

  std::atomic<int64_t> speed_;
};

}  // namespace libsmartereye2

#endif //GEMINI_SERIAL_PORT_H
