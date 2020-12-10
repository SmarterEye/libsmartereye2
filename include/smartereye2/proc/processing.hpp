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

#include "smartereye2/core/options.hpp"
#include "smartereye2/core/frame.hpp"
#include "smartereye2/core/frame_queue.hpp"
#include "smartereye2/core/frame_set.hpp"

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

}  // namespace se2

#endif  // LIBSMARTEREYE2_PROCESSING_HPP
