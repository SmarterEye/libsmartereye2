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

#ifndef LIBSMARTEREYE2_SYNCER_PROCESS_H
#define LIBSMARTEREYE2_SYNCER_PROCESS_H

#include "processing.h"
#include "concurrency/consumer_queue.h"
#include "streaming/stream_types.hpp"

namespace libsmartereye2 {

class SyncerLock {
 public:
  explicit SyncerLock(std::mutex &mutex) : mutex_(mutex) {
    mutex.lock();
  }

  ~SyncerLock() {
    if (is_locked_) {
      mutex_.unlock();
    }
  }

  void unlock() {
    if (is_locked_) {
      mutex_.unlock();
      is_locked_ = false;
    }
  }

 private:
  bool is_locked_ = true;
  std::mutex &mutex_;
};

template<class T>
class ConsumerFrameQueue {
 public:
  explicit ConsumerFrameQueue(uint32_t capacity = kQueueMaxSize) : queue_(capacity) {}

  void enqueue(T &&item) {
    if (item.isBlocking()) {
      queue_.blockingEnqueue(std::move(item));
    } else {
      queue_.enqueue(std::move(item));
    }
  }

  bool dequeue(T *item, uint32_t timeout_ms) { return queue_.dequeue(item, timeout_ms); }
  bool peek(T **item) { return queue_.peek(item); }
  bool tryDequeue(T *item) { return queue_.tryDequeue(item); }
  void clear() { queue_.clear(); }
  void start() { queue_.start(); }
  size_t size() const { return queue_.size(); }

 private:
  ConsumerQueue<T> queue_;
};

class SyntheticSourceInterface;

struct SyncronizationEnvironment {
  SyntheticSourceInterface *source;
  ConsumerFrameQueue<FrameHolder> &matches;
};

using StreamId = int;
using SyncCallback = std::function<void(FrameHolder, SyncronizationEnvironment)>;

class MatcherInterface {
 public:
  virtual void dispatch(FrameHolder f, SyncronizationEnvironment env) = 0;
  virtual void sync(FrameHolder f, SyncronizationEnvironment env) = 0;
  virtual void setCallback(SyncCallback f) = 0;
  virtual const std::vector<StreamId> &getStreams() const = 0;
  virtual const std::vector<FrameId> &getStreamsTypes() const = 0;
  virtual std::string getName() const = 0;
};

class Matcher : public MatcherInterface {
 public:
  explicit Matcher(std::vector<StreamId> streams_id = {});
  virtual ~Matcher();

  void sync(FrameHolder f, SyncronizationEnvironment env) override;
  void setCallback(SyncCallback f) override;
  const std::vector<StreamId> &getStreams() const override;
  const std::vector<FrameId> &getStreamsTypes() const override;
  std::string getName() const override;

  CallbackInvocationHolder beginCallback();

  bool getActive() const { return active_; }
  void setActive(const bool active) { active_ = active; }

 protected:
  std::vector<StreamId> streams_id_;
  std::vector<FrameId> streams_type_;
  SyncCallback callback_;
  CallbacksHeap callback_inflight_;
  std::string name_;
  bool active_ = true;
};

class IdentityMatcher : public Matcher {
 public:
  IdentityMatcher(StreamId stream_id, FrameId frame_id);
  void dispatch(FrameHolder f, SyncronizationEnvironment env) override;
};

class CompositeMatcher : public Matcher {
 public:
  CompositeMatcher(const std::vector<std::shared_ptr<Matcher>> &matchers, std::string name);

  virtual bool equal(const FrameHolder &a, const FrameHolder &b) = 0;
  virtual bool lessThan(const FrameHolder &a, const FrameHolder &b) = 0;
  virtual bool skipMissingStream(std::vector<Matcher *> synced, Matcher *missing) = 0;
  virtual void cleanInactiveStreams(const FrameHolder &frame_holder) = 0;
  virtual void updateLastArrived(FrameHolder &frame_holder, Matcher *matcher) = 0;

  void dispatch(FrameHolder frame_holder, SyncronizationEnvironment env) override;
  void sync(FrameHolder f, SyncronizationEnvironment env) override;

  std::string framesToString(std::vector<Matcher *> matchers);
  std::shared_ptr<Matcher> findMatcher(const FrameHolder &frame_holder);

 protected:
  virtual void updateNextExpected(const FrameHolder &f) = 0;
  static std::string frameToString(FrameHolder &frame_holder);
  static std::string createCompositeName(const std::vector<std::shared_ptr<Matcher>> &matchers,
                                         const std::string &name);

  std::map<Matcher *, ConsumerFrameQueue<FrameHolder>> frames_queue_;
  std::map<StreamId, std::shared_ptr<Matcher>> matchers_;
  std::map<Matcher *, double> _next_expected;
  std::map<Matcher *, TimestampDomain> _next_expected_domain;
};

class TimestampCompositeMatcher : public CompositeMatcher {
 public:
  explicit TimestampCompositeMatcher(std::vector<std::shared_ptr<Matcher>> matchers);

  bool equal(const FrameHolder &a, const FrameHolder &b) override;
  bool lessThan(const FrameHolder &a, const FrameHolder &b) override;
  bool skipMissingStream(std::vector<Matcher *> synced, Matcher *missing) override;
  void cleanInactiveStreams(const FrameHolder &frame_holder) override;
  void updateLastArrived(FrameHolder &frame_holder, Matcher *matcher) override;
  void updateNextExpected(const FrameHolder &frame_holder) override;

 private:
  static uint32_t fps(const FrameHolder &frame_holder);
  static std::pair<double, double> extractTimestamps(const FrameHolder &a, const FrameHolder &b);
  static bool equivalent(double a, double b, int fps);

  std::map<Matcher *, double> last_arrived_;
  std::map<Matcher *, unsigned int> fps_;
};

class SyncerProcessUnit : public ProcessingBlock {
 public:
  SyncerProcessUnit(std::initializer_list<bool> enable_opts);

  explicit SyncerProcessUnit(bool is_enabled_opt = false)
      : SyncerProcessUnit({is_enabled_opt}) {}

  void add_enabling_option(bool is_enabled_opt) {
    enable_opts_.push_back(is_enabled_opt);
  }

  ~SyncerProcessUnit() override {
    matcher_.reset();
  }

 private:
  std::unique_ptr<TimestampCompositeMatcher> matcher_;
  std::vector<bool> enable_opts_;
};

}  // namespace libsmartereye2

#endif //LIBSMARTEREYE2_SYNCER_PROCESS_H
