/// \file store_error.hpp
/// Utility class for storing / restoring errorno
#ifndef LIGHTBOX_NETWORK_NETWORK_HPP
#define LIGHTBOX_NETWORK_NETWORK_HPP
#include <filesystem>
#include <string>
namespace zebral
{
class Network
{
 public:
  Network();
  virtual ~Network();
};

}  // namespace zebral

#endif  // LIGHTBOX_NETWORK_NETWORK_HPP