/// \file http_client.hpp
/// HTTP client - right now specifically for the Zebral_ESP32CAM
#ifndef LIGHTBOX_NETWORK_HTTP_CLIENT_HPP_
#define LIGHTBOX_NETWORK_HTTP_CLIENT_HPP_

#include <curl/curl.h>
#include <filesystem>
#include <functional>
#include <regex>
#include <string>
#include "binary_curl_client.hpp"

namespace zebral
{
class HttpClient : public BinaryCurlClient
{
 public:
  HttpClient(const std::string& usr = "", const std::string& pwd = "");
  virtual ~HttpClient();

  /// Retrieves the URI and calls callback synchronously.
  /// If there's just one file, it then returns.
  /// If it's a streaming multipart, keeps calling the callback until the connection drops
  /// or the callback returns false.
  ///
  /// Returns respose code (200 on success)
  int Get(const std::string& uri, FileReceivedCallback callback,
          size_t maxLength = kDefMaxFileLength) override;

 protected:
  // in-context callback from CURL
  size_t OnWrite(uint8_t* data, size_t length) override;

  // Collects headers from a multipart buffer
  size_t collect_headers(uint8_t* data, size_t length, std::vector<std::string>& headers);

  std::regex content_regex_;       ///< Regex for finding boundary specification
  bool streaming_;                 ///< If true, is multipart
  std::string content_type_;       ///< Type of file (e.g. image/jpg)
  std::vector<uint8_t> boundary_;  ///< boundary string
  size_t part_index_;              ///< Counter for incoming parts when streaming
};

}  // namespace zebral

#endif  // LIGHTBOX_NETWORK_HTTP_CLIENT_HPP_