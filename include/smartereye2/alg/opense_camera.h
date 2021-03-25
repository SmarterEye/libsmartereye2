
#ifndef OPENSE_CAMERA_H_
#define OPENSE_CAMERA_H_
#include "opense_basic_type.h"
namespace opense {
// \param rotation : (m) rotation angle of the camera relative to VCS(Vehicle
// Coordinate System) origin.
// \param translation : (m) the translation of the camera relative to VCS(Vehicle
// Coordinate System) origin
struct CameraPositionParams {
  bool is_valid = false;
  opense::Point3f rotation;
  opense::Point3f translation;
};

// \param fov :(º) the FOV of X,Y direction
// \param optic_center : optical center position of camera
struct CameraCalibParams {
  int img_width = 1280;  // unit:pixel
  int img_height = 720;  // unit:pixel

  float x_focus;         // (m/pixels)
  float y_focus;         // (m/pixels)
  float bf_value = 117.5f;        //  baseline * focus / pixel_size
  float base_line = 0.12f;        // unit:m
  float lens_focus = 0.004f;      // unit:m
  float pixel_size = 0.0000042f;  // (m) the actual size of a pixel
  float fov_horizon = 38.f;
  float fov_vertical = 21.3f;

  opense::Point2i optic_center = opense::Point2i(640, 360);
};

}  // namespace opense
#endif  // OPENSE_CAMERA_H_