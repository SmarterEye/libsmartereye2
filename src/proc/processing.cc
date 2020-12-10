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

#include "processing.h"
#include "proc/processing.hpp"

#include <memory>

#include "proc/syncer_process.h"
#include "easylogging++.h"

namespace libsmartereye2 {

ProcessingBlock::ProcessingBlock(const std::string &name)
    : source_wrapper_(frame_source_) {
  registerOption(OptionKey::FRAMES_QUEUE_SIZE, frame_source_.get_published_size_option());
  registerInfo(CameraInfo::CAMERA_INFO_NAME, name);
  frame_source_.init(std::shared_ptr<MetadataParserMap>());
}

void ProcessingBlock::setProcessingCallback(FrameProcessorCallbackPtr callback) {
  std::lock_guard<std::mutex> lock(mutex_);
  callback_ = callback;
}

void ProcessingBlock::setOutputCallback(FrameCallbackPtr callback) {
  frame_source_.set_callback(callback);
}

void ProcessingBlock::invoke(FrameHolder frame) {
  auto callback = frame_source_.begin_callback();
  try {
    if (callback_) {
      FrameInterface *frame_ptr = nullptr;
      std::swap(frame.frame, frame_ptr);
      auto source = new SeSyntheticSource{&source_wrapper_};
      callback_->onFrame(frame_ptr, source);
    }
  }
  catch (std::exception const &e) {
    LOG(ERROR) << "Exception was thrown during user processing callback: " << std::string(e.what());
  }
  catch (...) {
    LOG(ERROR) << "Exception was thrown during user processing callback!";
  }
}

}  // namespace libsmartereye2

namespace se2 {

void ProcessingBlock::invoke(Frame frame) const {
  SeFrame *ptr = nullptr;
  std::swap(frame.frame_ref_, ptr);
  block_->block->invoke(libsmartereye2::FrameHolder(ptr));
}

bool ProcessingBlock::support(CameraInfo info) const {
  return block_->block->supportsInfo(info);
}

std::string ProcessingBlock::getInfo(CameraInfo info) const {
  return block_->block->getInfo(info);
}

void ProcessingBlock::registerSimpleOption(OptionKey option_key, OptionRange range) {
  if (block_->options->supportsOption(option_key)) return;

  // TODO
  std::shared_ptr<libsmartereye2::Option> foo = nullptr;
  dynamic_cast<libsmartereye2::OptionsContainer *>(block_->options)->registerOption(option_key, foo);
}

}  // namespace se2
