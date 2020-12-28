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

#ifndef LIBSMARTEREYE2_NOTIFICATION_HPP
#define LIBSMARTEREYE2_NOTIFICATION_HPP

#include <string>

#include "sensor_types.hpp"
#include "smartereye2/se_callbacks.hpp"
#include "smartereye2/se_global.hpp"

namespace se2 {

class SMARTEREYE2_API Notification {
 public:
  explicit Notification(SeNotification *notification);

  NotificationCategory getCategory() const { return category_; }
  std::string getDescription() const { return description_; }
  double getTimestamp() const { return timestamp_; }
  std::string getSerializedData() const { return serialized_data_; }

 private:
  std::string description_;
  double timestamp_ = -1.0;
  NotificationCategory category_ = NOTIFICATION_CATEGORY_COUNT;
  std::string serialized_data_;
};

template<class T>
class notifications_callback : public SeNotificationsCallback {
  T on_notification_function;

 public:
  explicit notifications_callback(T on_notification) : on_notification_function(on_notification) {}

  void onNotification(SeNotification *_notification) override {
    on_notification_function(Notification{_notification});
  }

  void release() override { delete this; }
};

}  //  namespace se2

#endif //LIBSMARTEREYE2_NOTIFICATION_HPP
