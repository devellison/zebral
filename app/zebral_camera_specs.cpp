#include "camera_util.hpp"
#include "errors.hpp"
#include "log.hpp"
#include "platform.hpp"

using namespace zebral;
int main(int, char**)
{
  Result res = Result::ZBA_OK;

  double fov_dim    = est_fov_dim_at_dist(18.44, 43.5);
  double obj_pixels = est_pixels_for_object(fov_dim, 672, 0.075);
  double distance   = est_distance_for_fov_dim(150, 43.5);
  std::cout << "FOV of 43.5, at distance of 18.44, you'll see a width of " << fov_dim << std::endl;
  std::cout << "Pixels in an 0.075m object with a 672x? frame size: " << obj_pixels << std::endl;
  std::cout << "Distance to get 150m wide fov: " << distance << std::endl;

  // W 87±3 / V:58±1 /
  fov_dim    = est_fov_dim_at_dist(1.5, 87);
  obj_pixels = est_pixels_for_object(fov_dim, 1280, 0.0127);
  std::cout << "FOV 87 at 1.5m is " << fov_dim << " m across with " << obj_pixels
            << " for an 0.0127m tube at high res" << std::endl;

  return static_cast<int>(res);
}
