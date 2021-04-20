#ifndef OPENSE_VEHICLE_INFO_H_
#define OPENSE_VEHICLE_INFO_H_
#include "opense_basic_type.h"
namespace opense {
// \param steer_angle:(rad) steering wheel angle, positive value is
// counterclockwise,negative value is clockwise.
// \param steer_angle_rate:  (rad/s) steering wheel angle rate, positive value
// is counterclockwise,negative value is clockwise.
struct VehicleRealTimeInfo {
  bool brake_swith = false;        // driver brake status
  bool left_indicator = false;     // vehicle left turn signal status
  bool right_indicator = false;    // vehicle right turn signal status
  bool high_beam = false;          // indicate if high beam is valid
  bool low_beam = false;           // indicate if low beam is valid
  bool fog_lamp = false;           // indicate if fog lamp is valid
  bool wind_shield_wiper = false;  // indicate if wind shield wiper is valid
  
  int gear = 0;                  // car gear
  float ego_speed = 0.f;         //(m/s)
  float ego_acceleration = 0.f;  //(m/s^2)
  float yaw_rate = 0.f;          //(rad/s)
  float steer_angle = 0.f;       //(rad)
  
  float estimate_speed = 0.f;          //(m/s)
  float estimate_acceleration = 0.f;   //(m/s^2)
};

struct VehicleIntrinsicInfo {
  float rear;                    // (m) distance from rear axle to rear
  float mass;                    // (m/s)
  float width;                   // (m)
  float height;                  // (m)
  float length;                  // (m)
  float car_head_height;         // (m)
  float wheel_base;              //(m)
  float steer_ratio;             //
  float rotational_inertia;      //
  float centroid_to_front_axle;  // (m)
  float front_tire_stiffness;    //(N/rad)
  float rear_tire_stiffness;     // (N/rad)
};
}  // namespace opense
#endif  // OPENSE_VEHICLE_INFO_H_