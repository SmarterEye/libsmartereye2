#ifndef OPENSE_BASIC_TYPE_H_
#define OPENSE_BASIC_TYPE_H_
#include <array>
#include <vector>
namespace opense {
template <typename Tp_>
struct Point {
  Point() : x(static_cast<Tp_>(0)), y(static_cast<Tp_>(0)) {}
  Point(const Tp_ x_, const Tp_ y_) : x(x_), y(y_) {}

  Tp_ x, y;
};

template <typename Tp_>
struct Point3 {
  Point3()
      : x(static_cast<Tp_>(0)),
        y(static_cast<Tp_>(0)),
        z(static_cast<Tp_>(0)) {}
  Point3(const Tp_ x_, const Tp_ y_, const Tp_ z_) : x(x_), y(y_), z(z_) {}

  Tp_ x, y, z;
};

template <typename Tp_>
struct Box3D {
  float conf = 0.f;
  opense::Point<Tp_> lower_lt, lower_lb;
  opense::Point<Tp_> lower_rb, lower_rt;
  opense::Point<Tp_> upper_lt, upper_lb;
  opense::Point<Tp_> upper_rb, upper_rt;
};

template <typename Tp_>
struct Rect {
  Rect() : x(0), y(0), width(0), height(0) {}
  Rect(const Tp_ x, const Tp_ y, const Tp_ width, const Tp_ height)
      : x(x), y(y), width(width), height(height) {}

  Tp_ x = static_cast<Tp_>(0);
  Tp_ y = static_cast<Tp_>(0);
  Tp_ width = static_cast<Tp_>(0);
  Tp_ height = static_cast<Tp_>(0);
};

template <typename Tp_>
struct OBBox2D {
  opense::Point<Tp_> corner;
  std::vector<opense::Point<Tp_>> axes_pts;
};

template <typename Tp_>
struct KeyPoint {
  float type = 0.f;
  float conf = 0.f;
  opense::Point<Tp_> pt;
};

template <typename Tp_>
struct KeyPoint3 {
  float type = 0.f;
  float conf = 0.f;
  opense::Point3<Tp_> pt;
};

template <typename Tp_, int n_>
struct Image {
  int width = 0;
  int height = 0;
  int depth = n_;
  Tp_ *data = nullptr;
};

using Point2i = opense::Point<int>;
using Point2f = opense::Point<float>;
using Point2d = opense::Point<double>;

using Point3i = opense::Point3<int>;
using Point3f = opense::Point3<float>;
using Point3d = opense::Point3<double>;

using KeyPoint2i = opense::KeyPoint<int>;
using KeyPoint3f = opense::KeyPoint3<float>;

using Vec3f = std::array<float, 3>;

using Image1i = opense::Image<int, 1>;
using Image1f = opense::Image<float, 1>;
using Image1b = opense::Image<uint8_t, 1>;
using Image1s = opense::Image<uint16_t, 1>;

}  // namespace opense
#endif  // OPENSE_BASIC_TYPE_H_