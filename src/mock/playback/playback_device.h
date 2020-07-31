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

#ifndef LIBSMARTEREYE2_PLAYBACK_DEVICE_H
#define LIBSMARTEREYE2_PLAYBACK_DEVICE_H

#include "device/device.h"
#include "concurrency/dispatcher.h"

namespace libsmartereye2 {

class PlaybackSensor;

class PlaybackDevice : public DeviceInterface, public InfoContainer {
 public:
//  PlaybackDevice(std::shared_ptr<ContextPrivate> context, std::shared_ptr<device_serializer::reader> serializer);
  ~PlaybackDevice() override;

  SensorInterface &getSensor(size_t i) override;
  const SensorInterface &getSensor(size_t i) const override;
  size_t getSensorCount() const override;
  void hardwareReset() override;

  std::shared_ptr<ContextPrivate> getContext() const override;
  std::pair<int32_t, Extrinsics> getExtrinsics(const StreamInterface &stream) const override;
  platform::BackendDeviceGroup getDeviceData() const override;

  bool isValid() const override;

  std::shared_ptr<Matcher> createMatcher(const FrameHolder &frame) const override;

  std::vector<TaggedProfile> getProfilesTags() const override;
  void tagProfiles(StreamProfiles profiles) const override;

  std::string getInfo(CameraInfo info) const override;
  bool supportsInfo(CameraInfo info) const override;

//  void setFrameRate(double rate);
//  void seekToTime(std::chrono::nanoseconds time);
//  PlaybackStatus getCurrentStatus() const;
//  uint64_t getDuration() const;
//  void pause();
//  void resume();
//  void stop();
//  void setRealTime(bool real_time);
//  bool isRealTime() const;
  std::string getFileName() const { return ""; }
//  uint64_t getPosition() const;

 private:
//  void update_time_base(device_serializer::nanoseconds base_timestamp);
//  device_serializer::nanoseconds calc_sleep_time(device_serializer::nanoseconds timestamp);
//  void start();
//  void stop_internal();
//  void try_looping();
//  template<typename T>
//  void do_loop(T op);
//  std::map<uint32_t,
//           std::shared_ptr<playback_sensor>> create_playback_sensors(const device_serializer::device_snapshot &device_description);
//  std::shared_ptr<stream_profile_interface> get_stream(const std::map<unsigned,
//                                                                      std::shared_ptr<playback_sensor>> &sensors_map,
//                                                       device_serializer::stream_identifier stream_id);
//  rs2_extrinsics calc_extrinsic(const rs2_extrinsics &from, const rs2_extrinsics &to);
//  void catch_up();
//  void register_device_info(const device_serializer::device_snapshot &device_description);
//  void register_extrinsics(const device_serializer::device_snapshot &device_description);
//  void update_extensions(const device_serializer::device_snapshot &device_description);
//  bool prefetch_done();

 private:
  std::shared_ptr<Dispatcher> m_read_thread;
  std::shared_ptr<ContextPrivate> m_context;
//  std::shared_ptr<device_serializer::reader> m_reader;
//  device_serializer::device_snapshot m_device_description;
  std::atomic_bool m_is_started;
  std::atomic_bool m_is_paused;
//  std::chrono::high_resolution_clock::time_point
//      m_base_sys_time; // !< System time when reading began (first frame was read)
//  device_serializer::nanoseconds
//      m_base_timestamp; // !< Timestamp of the first frame that has a real timestamp (different than 0)
  std::map<uint32_t, std::shared_ptr<PlaybackSensor>> m_sensors;
  std::map<uint32_t, std::shared_ptr<PlaybackSensor>> m_active_sensors;
  std::atomic<double> m_sample_rate;
  std::atomic_bool m_real_time;
//  device_serializer::nanoseconds m_prev_timestamp;
//  std::vector<std::shared_ptr<lazy < rs2_extrinsics>>>
//  m_extrinsics_fetchers;
//  std::map<int, std::pair<uint32_t, rs2_extrinsics>> m_extrinsics_map;
//  device_serializer::nanoseconds m_last_published_timestamp;
  std::mutex m_last_published_timestamp_mutex;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PLAYBACK_DEVICE_H
