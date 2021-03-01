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

#ifndef LIBSMARTEREYE2_PIPELINE_H
#define LIBSMARTEREYE2_PIPELINE_H

#include <utility>

#include "core/frame_aggregator.h"
#include "device/context.h"
#include "device/device_hub.h"
#include "pipeline_profile.h"
#include "concurrency/concurrency.h"

namespace libsmartereye2 {
class PipelinePrivate;
}

struct SePipeline {
  std::shared_ptr<libsmartereye2::PipelinePrivate> pipeline;
};

namespace libsmartereye2 {

class PipelineConfigPrivate;

class PipelinePrivate : public std::enable_shared_from_this<PipelinePrivate> {
 public:
  explicit PipelinePrivate(const std::shared_ptr<ContextPrivate>& context);
  virtual ~PipelinePrivate();

  std::shared_ptr<PipelineProfilePrivate> start(std::shared_ptr<PipelineConfigPrivate> conf,
                                                FrameCallbackPtr callback = nullptr);
  void stop(bool force);
  void restart(bool force);
  bool isConnected() const;

  int64_t registerInternalDeviceCallback(DevicesChangedCallbackPtr callback);
  void unregisterDevicesChangedCallback(int64_t cb_id);

  std::shared_ptr<PipelineProfilePrivate> getActiveProfile() const;
  FrameHolder waitForFrames(uint32_t timeout_ms);
  bool pollForFrames(FrameHolder *frame_holder);
  bool tryWaitForFrames(FrameHolder *frame_holder, uint32_t timeout_ms);
  std::shared_ptr<DeviceInterface> waitForDevice(const std::chrono::milliseconds &timeout,
                                                 const std::string &serial = "");
  std::shared_ptr<ContextPrivate> getContext() const { return context_; }

 protected:
  FrameCallbackPtr getCallback() const;
  std::vector<int> onStart(const std::shared_ptr<PipelineProfilePrivate>& profile);
  bool unsafeStart(std::shared_ptr<PipelineConfigPrivate> conf);
  void unsafeStop();

  mutable std::mutex mutex_;
  std::shared_ptr<PipelineProfilePrivate> active_profile_;
  std::shared_ptr<PipelineConfigPrivate> prev_conf_;
  DeviceHubPrivate device_hub_;

 private:
  std::shared_ptr<PipelineProfilePrivate> unsafeGetActiveProfile() const;

  std::shared_ptr<ContextPrivate> context_;

  std::unique_ptr<FrameAggregator> aggregator_;
  std::vector<FrameId> synced_streams_;
  FrameCallbackPtr streams_callback_;
  bool is_stopping_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_PIPELINE_H
