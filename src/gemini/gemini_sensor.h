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

#ifndef LIBSMARTEREYE2_GEMINI_SENSOR_H
#define LIBSMARTEREYE2_GEMINI_SENSOR_H

#include "sensor/sensor.h"

namespace libsmartereye2 {

class GeminiDevice;
class PlaybackDevice;

struct UsbFrameGroup {
  int32_t frame_count;
  int32_t total_size;
  platform::UsbFrameInfo *frame_infos[15];
  uint8_t *frame_datas[15];
  int64_t timestamp;
};

struct RawUsbImageFrame {
  uint8_t image[0];
};

struct RawUsbImageFrame4Embededline {
  uint8_t embededline[1280];
  uint8_t image[0];
};

class GeminiSensor : public SensorBase, public VideoSensorInterface {
 public:
  explicit GeminiSensor(GeminiDevice *owner);
  ~GeminiSensor() override;

  StreamProfiles initStreamProfiles() override;

  void open(const StreamProfiles &requests) override;
  void close() override;
  void start(FrameCallbackPtr callback) override;
  void stop() override;

  // override from VideoSensorInterface
  Intrinsics getIntrinsics() const override;

  void init();
  void dispose();

 protected:
  bool startStream();
  void stopStream();

 private:
  mutable std::mutex operation_lock_;

  std::vector<platform::UsbFrameInfo> supported_frame_infos_;
  std::map<uint32_t, platform::UsbFrameInfo> active_frame_infos_;
  std::map<int, std::shared_ptr<StreamProfileInterface>> frame_id_to_profile_;

  UsbFrameGroup usb_frame_group_{};
  std::vector<uint8_t> buffer_;

  // threaded dispatch
  std::shared_ptr<Dispatcher> data_dispatcher_;
  void dispatch_threaded(FrameHolder frame);

  // stream
  std::thread stream_thread_;
  void handle_received_frames();
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_GEMINI_SENSOR_H
