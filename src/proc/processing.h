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
#include "core/info.h"
#include "core/frame_source.h"
#include "proc/synthetic_stream.h"

#include "se_callbacks.hpp"

namespace libsmartereye2 {
class ProcessingBlockInterface;
}

struct SeProcessingBlock : public SeOptions, public noncopyable {
  explicit SeProcessingBlock(const std::shared_ptr<libsmartereye2::ProcessingBlockInterface> &block)
      : SeOptions((libsmartereye2::OptionsInterface *) block.get()),
        block(block) {}

  std::shared_ptr<libsmartereye2::ProcessingBlockInterface> block;
};

namespace libsmartereye2 {

class FrameInterface;
struct FrameHolder;

class ProcessingBlockInterface : public virtual OptionsInterface, public virtual InfoInterface {
 public:
  virtual void setProcessingCallback(FrameProcessorCallbackPtr callback) = 0;
  virtual void setOutputCallback(FrameCallbackPtr callback) = 0;
  virtual void invoke(FrameHolder frame) = 0;
  virtual SyntheticSourceInterface &getSource() = 0;

  ~ProcessingBlockInterface() override = default;
};

class ProcessingBlock : public ProcessingBlockInterface, public OptionsContainer, public InfoContainer {
 public:
  explicit ProcessingBlock(const std::string &name);
  ~ProcessingBlock() override { frame_source_.flush(); }

  void setProcessingCallback(FrameProcessorCallbackPtr callback) override;
  void setOutputCallback(FrameCallbackPtr callback) override;
  void invoke(FrameHolder frame) override;
  SyntheticSourceInterface &getSource() override { return source_wrapper_; }

 protected:
  FrameSource frame_source_;
  std::mutex mutex_;
  FrameProcessorCallbackPtr callback_;
  SyntheticSource source_wrapper_;
};

template<class T>
class InternalFrameProcessorCallback : public SeFrameProcessorCallback {
  T on_frame_function;
 public:
  explicit InternalFrameProcessorCallback(T on_frame) : on_frame_function(on_frame) {}

  void onFrame(SeFrame *f, SeSyntheticSource *source) override {
    FrameHolder front((FrameInterface *) f);
    on_frame_function(std::move(front), source->source);
  }

  void release() override { delete this; }
};

}  // namespace libsmartereye2

#endif  // LIBSMARTEREYE2_PROCESSING_H
