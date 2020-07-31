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

#ifndef LIBSMARTEREYE2_PLAYBACKSENSOR_H
#define LIBSMARTEREYE2_PLAYBACKSENSOR_H

#include "sensor/sensor.h"
#include "streaming/serialization.h"

#include <functional>

namespace libsmartereye2 {

class FrameHolder;
class Dispatcher;

class PlaybackSensor : public SensorInterface,
                       public InfoContainer,
                       public OptionsContainer,
                       public std::enable_shared_from_this<PlaybackSensor> {
 public:
  using frame_interface_callback_t = std::function<void(FrameHolder)>;
//  signal<PlaybackSensor, uint32_t, frame_callback_ptr> started;
//  signal<PlaybackSensor, uint32_t, bool> stopped;
//  signal<PlaybackSensor, const std::vector<device_serializer::stream_identifier> &> opened;
//  signal<PlaybackSensor, const std::vector<device_serializer::stream_identifier> &> closed;

  PlaybackSensor(DeviceInterface &parent_device);
  ~PlaybackSensor() override;

  StreamProfiles getStreamProfiles(int tag) const override;
  StreamProfiles getActiveStreams() const override;

  void open(const StreamProfiles &requests) override;
  void close() override;
  void start(FrameCallbackPtr callback) override;
  void stop() override;

  FrameCallbackPtr getFramesCallback() const override;
  void setFramesCallback(FrameCallbackPtr cb) override;
  bool isStreaming() const override;

  DeviceInterface &getDevice() override;
  
  void update_option(OptionKey id, std::shared_ptr<Option> option);
  void stop(bool invoke_required);
  void flush_pending_frames();
//  void update(const device_serializer::sensor_snapshot &sensor_snapshot);
  
//  void raise_notification(const notification &n);
//  bool streams_contains_one_frame_or_more();
  
//  virtual processing_blocks get_recommended_processing_blocks() const override {
//    auto processing_blocks_snapshot =
//        m_sensor_description.get_sensor_extensions_snapshots().find(RS2_EXTENSION_RECOMMENDED_FILTERS);
//    if (processing_blocks_snapshot == nullptr) {
//      throw invalid_value_exception("Recorded file does not contain sensor processing blocks");
//    }
//    auto processing_blocks_api = As<recommended_proccesing_blocks_interface>(processing_blocks_snapshot);
//    if (processing_blocks_api == nullptr) {
//      throw invalid_value_exception("Failed to get options interface from sensor snapshots");
//    }
//    return processing_blocks_api->get_recommended_processing_blocks();
//  }
 protected:
  void set_active_streams(const StreamProfiles &requests);

 private:
  void register_sensor_streams(const StreamProfiles &vector);
//  void register_sensor_infos(const device_serializer::sensor_snapshot &sensor_snapshot);
//  void register_sensor_options(const device_serializer::sensor_snapshot &sensor_snapshot);

  FrameCallbackPtr m_user_callback;
//  notifications_processor _notifications_processor;
  using stream_unique_id = int;
  std::map<stream_unique_id, std::shared_ptr<Dispatcher>> m_dispatchers;
  std::atomic<bool> m_is_started;
//  device_serializer::sensor_snapshot m_sensor_description;
  uint32_t m_sensor_id;
  std::mutex m_mutex;
  std::map<std::pair<FrameId, uint32_t>, std::shared_ptr<StreamProfileInterface>> m_streams;
  DeviceInterface &m_parent_device;
  StreamProfiles m_available_profiles;
  StreamProfiles m_active_streams;
  mutable std::mutex m_active_profile_mutex;
  const unsigned int _default_queue_size;

 public:
  //handle frame use 3 lambda functions that determines if and when a frame should be published.
  //calc_sleep - calculates the duration that the sensor should wait before publishing the frame,
  // the start point for this calculation is the last playback resume.
  //is_paused - check if the playback was paused while waiting for the frame publish time.
  //update_last_pushed_frame - lets the playback device know that a specific frame was published,
  // the playback device will use this info to determine which frames should be played next in a pause/resume scenario.
//  template<class T, class K, class P>
//  void handle_frame(FrameHolder frame, bool is_real_time, T calc_sleep, K is_paused, P update_last_pushed_frame) {
//    if (frame == nullptr) {
//      throw invalid_value_exception("null frame passed to handle_frame");
//    }
//    if (m_is_started) {
//      frame->get_owner()->set_sensor(shared_from_this());
//      auto type = frame->get_stream()->get_stream_type();
//      auto index = static_cast<uint32_t>(frame->get_stream()->get_stream_index());
//      frame->set_stream(m_streams[std::make_pair(type, index)]);
//      frame->set_sensor(shared_from_this());
//      auto stream_id = frame.frame->get_stream()->get_unique_id();
//      //TODO: Ziv, remove usage of shared_ptr when FrameHolder is cpoyable
//      auto pf = std::make_shared<FrameHolder>(std::move(frame));
//
//      auto callback =
//          [this, is_real_time, stream_id, pf, calc_sleep, is_paused, update_last_pushed_frame](Dispatcher::cancellable_timer t) {
//            device_serializer::nanoseconds sleep_for = calc_sleep();
//            if (sleep_for.count() > 0)
//              t.try_sleep(sleep_for.count() * 1e-6);
//            if (is_paused())
//              return;
//
//            FrameInterface *pframe = nullptr;
//            std::swap((*pf).frame, pframe);
//
//            m_user_callback->onFrame((SeFrame *) pframe);
//            update_last_pushed_frame();
//          };
//      m_dispatchers.at(stream_id)->invoke(callback, !is_real_time);
//    }
//  }
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PLAYBACKSENSOR_H
