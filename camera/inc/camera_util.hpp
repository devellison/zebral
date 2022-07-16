/// \file camera_util.hpp
/// Utility functions for cameras / optics
#ifndef LIGHTBOX_CAMERA_CAMERA_UTIL_HPP
#define LIGHTBOX_CAMERA_CAMERA_UTIL_HPP

#include <chrono>
#include <cmath>
#include <limits>
#include <numbers>

// Sensor dimension(mm) / Focal length(mm) = Field dimension / Distance to Field
// obj_height_mm *
namespace zebral
{
/// radians to degrees
static inline double rad2deg(double radians)
{
  return radians * 180.0 / std::numbers::pi;
}
/// degrees to radians
static inline double deg2rad(double degrees)
{
  return degrees * std::numbers::pi / 180.0;
}

// These are just useful to have around - less for inside applications,
// but more for deciding if a sensor might work quickly.  The are all
// *estimates*.  You'd need intrinsic calibration data for a specific camera
// to be accurate.

/// Returns FOV in degrees from sensor dimensions and focal length
double est_fov_degrees(double sensor_dimension_mm, double focal_length_mm);

/// Returns FOV in radians from sensor dimensions and focal length
double est_fov_radians(double sensor_dimension_mm, double focal_length_mm);

/// Returns the width/height in the FOV at a specified distance.
double est_fov_dim_at_dist(double distance_meters, double fov_axis_degrees);

/// Est. how many pixels an object gets at a distance
/// \param fov_dim_at_dist - size of the view plane at a distance in one dimension in meters
/// \param pixels_dim - number of pixels in image frame along the same axis
/// \param obj_dim - size of object in meters along same axis
/// \return double - est. number of pixels for object along that dimension
double est_pixels_for_object(double fov_dim_at_dist, int pixels_dim, double obj_dim);

/// Est distance to get a fov of a specified size on a dimension
double est_distance_for_fov_dim(double desired_fov_dim, double fov_axis_degrees);

}  // namespace zebral
#endif  // LIGHTBOX_CAMERA_CAMERA_UTIL_HPP