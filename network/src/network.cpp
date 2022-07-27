#include "network.hpp"
#include <curl/curl.h>

namespace zebral
{
Network::Network()
{
  curl_global_init(CURL_GLOBAL_ALL);
}
Network::~Network()
{
  curl_global_cleanup();
}

}  // namespace zebral