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

#ifndef LIBSMARTEREYE2_PROCESSING_H
#define LIBSMARTEREYE2_PROCESSING_H

#include "core/options.h"
#include "device/sensor.h"
#include "se_callbacks.h"

namespace libsmartereye2 {

class Frame;
class OptionsInterface;
class FrameInterface;
class StreamProfileBase;
struct FrameHolder;
struct SeProcessingBlock;

template<class T>
class FrameProcessorCallback : public SeFrameProcessorCallback {
 public:
  explicit FrameProcessorCallback(T on_frame) : on_frame_function_(on_frame) {}
  void onFrame(FrameInterface *frame) override {}
  void release() override {}

 private:
  T on_frame_function_;
};

class SyntheticSourceInterface {
 public:
 public:
  virtual ~SyntheticSourceInterface() = default;

  virtual FrameInterface *allocateVideoFrame(std::shared_ptr<StreamProfileBase> stream,
                                             FrameInterface *original,
                                             int new_bpp = 0,
                                             int new_width = 0,
                                             int new_height = 0,
                                             int new_stride = 0,
                                             SeExtension frame_type = SeExtension::EXTENSION_VIDEO_FRAME) = 0;

  virtual FrameInterface *allocateMotionFrame(std::shared_ptr<StreamProfileBase> stream,
                                              FrameInterface *original,
                                              SeExtension frame_type = SeExtension::EXTENSION_MOTION_FRAME) = 0;

  virtual FrameInterface *allocateCompositeFrame(std::vector<FrameHolder> frames) = 0;

  virtual FrameInterface *allocatePoints(std::shared_ptr<StreamProfileBase> stream,
                                         FrameInterface *original,
                                         SeExtension frame_type = SeExtension::EXTENSION_POINTS) = 0;

  virtual void frameReady(FrameHolder result) = 0;
};

class ProcessingBlockInterface : public OptionsInterface {
 public:
  virtual void set_processing_callback(FrameCallbackPtr callback) = 0;
  virtual void set_output_callback(FrameCallbackPtr callback) = 0;
  virtual void invoke(FrameHolder frame) = 0;
  virtual SyntheticSourceInterface &get_source() = 0;

  virtual ~ProcessingBlockInterface() = default;
};

class ProcessingBlock : public Options {
 public:
  using Options::supports;

  explicit ProcessingBlock(const std::shared_ptr<SeProcessingBlock> &block)
      : Options((SeOptions *) block.get()), block_(block) {}

  template<class S>
  explicit ProcessingBlock(S processing_func) {}

  explicit operator SeOptions *() const { return (SeOptions *) get(); }
  virtual SeProcessingBlock *get() const { return block_.get(); }

  template<class S>
  void start(S on_frame) {}

  template<class S>
  S &operator>>(S &on_frame) {}

  void invoke(Frame frame) const;
  bool support(CameraInfo info) const;
  std::string getInfo(CameraInfo info) const;

 protected:
  void registerSimpleOption(OptionKey option_key, OptionRange range);
  std::shared_ptr<SeProcessingBlock> block_;
};

class AsynchronousSyncer : public ProcessingBlock {
 public:
  AsynchronousSyncer() : ProcessingBlock(init()) {};

 private:
  std::shared_ptr<SeProcessingBlock> init() { return nullptr; }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_PROCESSING_H
