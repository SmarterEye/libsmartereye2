//
// Created by xtp on 2020/8/18.
//

#include <smartereye2/sensor/notification.hpp>

#include "notification.h"
#include "easylogging++.h"

#include "sensor/notification.h"

namespace libsmartereye2 {

NotificationPrivate::NotificationPrivate(NotificationCategory category, int type, const std::string &description)
    : category(category), type(type), description(description) {
  timestamp = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
  LOG(INFO) << description;
}

}  // namespace libsmartereye2

namespace se2 {

Notification::Notification(SeNotification *notification) {
  auto notif = notification->notification;
  description_ = notif->description;
  timestamp_ = notif->timestamp;
  category_ = notif->category;
  serialized_data_ = notif->serialized_data;
}

}  // namespace se2
