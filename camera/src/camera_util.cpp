#include "camera_util.hpp"
#include <cmath>
#include <limits>
#include "errors.hpp"
namespace zebral
{
double est_fov_degrees(double sensor_dimension_mm, double focal_length_mm)
{
  return rad2deg(est_fov_radians(sensor_dimension_mm, focal_length_mm));
}
double est_fov_radians(double sensor_dimension_mm, double focal_length_mm)
{
  if (focal_length_mm < std::numeric_limits<double>::epsilon())
  {
    ZBA_THROW("Focal length cannot be negative or zero.", Result::ZBA_INVALID_PARAMETER);
  }
  return 2 * atan((sensor_dimension_mm / 2.0) / focal_length_mm);
}
double est_fov_dim_at_dist(double distance_meters, double fov_axis_degrees)
{
  if ((fov_axis_degrees <= 0) || (fov_axis_degrees >= 180))
  {
    ZBA_THROW("Invalid angle - must be greater than 0 and less than 180 degrees.",
              Result::ZBA_INVALID_PARAMETER);
  }

  return 2 * tan(deg2rad(fov_axis_degrees / 2.0)) * distance_meters;
}

double est_pixels_for_object(double fov_dim_at_dist, int pixels_dim, double obj_dim)
{
  double pixels_per_meter = static_cast<double>(pixels_dim) / fov_dim_at_dist;
  return obj_dim * pixels_per_meter;
}

double est_distance_for_fov_dim(double desired_fov_dim, double fov_axis_degrees)
{
  if ((fov_axis_degrees <= 0) || (fov_axis_degrees >= 180))
  {
    ZBA_THROW("Invalid angle - must be greater than 0 and less than 180 degrees.",
              Result::ZBA_INVALID_PARAMETER);
  }
  return (desired_fov_dim / 2.0) / tan(deg2rad(fov_axis_degrees) / 2.0);
}

}  // namespace zebral
