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

#ifndef LIBSMARTEREYE2_PROCESSING_HPP
#define LIBSMARTEREYE2_PROCESSING_HPP

#include "core/options.hpp"
#include "core/frame.hpp"
#include "core/frame_queue.hpp"
#include "core/frame_set.hpp"

namespace se2 {

template<class T>
class FrameProcessorCallback : public SeFrameProcessorCallback {
 public:
  explicit FrameProcessorCallback(T on_frame) : on_frame_function_(on_frame) {}
  void onFrame(SeFrame *frame, SeSyntheticSource *source) override {}
  void release() override {}

 private:
  T on_frame_function_;
};

class SMARTEREYE2_API ProcessingBlock : public Options {
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
  std::shared_ptr<SeProcessingBlock> init();
};

class Syncer {
 public:
  /**
  * Sync instance to align frames from different streams
  */
  explicit Syncer(int queue_size = 1) : frame_queue_(queue_size) {
    sync_.start(frame_queue_);
  }

  /**
  * Wait until coherent set of frames becomes available
  * \param[in] timeout_ms   Max time in milliseconds to wait until an exception will be thrown
  * \return Set of coherent frames
  */
  FrameSet waitForFrame(unsigned int timeout_ms = 5000) const {
    return FrameSet(frame_queue_.waitForFrame(timeout_ms));
  }

  /**
  * Check if a coherent set of frames is available
  * \param[out] fs      New coherent frame-set
  * \return true if new frame-set was stored to result
  */
  bool pollForFrame(FrameSet *fs) const {
    Frame result;
    if (frame_queue_.pollForFrame(&result)) {
      *fs = FrameSet(result);
      return true;
    }
    return false;
  }

  /**
  * Wait until coherent set of frames becomes available
  * \param[in] timeout_ms     Max time in milliseconds to wait until an available frame
  * \param[out] fs            New coherent frame-set
  * \return true if new frame-set was stored to result
  */
  bool tryWaitForFrame(FrameSet *fs, unsigned int timeout_ms = 5000) const {
    Frame result;
    if (frame_queue_.tryWaitForFrame(&result, timeout_ms)) {
      *fs = FrameSet(result);
      return true;
    }
    return false;
  }

  void operator()(Frame f) const {
    sync_.invoke(std::move(f));
  }

 private:
  AsynchronousSyncer sync_;
  FrameQueue frame_queue_;
};

}  // namespace se2

#endif  // LIBSMARTEREYE2_PROCESSING_HPP
