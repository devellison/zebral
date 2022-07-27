/// \file binary_curl_client.hpp
/// Client based on CURL that expects 1 or more binary files as a result of a query
#ifndef LIGHTBOX_NETWORK_BINARY_CURL_CLIENT_HPP_
#define LIGHTBOX_NETWORK_BINARY_CURL_CLIENT_HPP_

#include <curl/curl.h>
#include <filesystem>
#include <functional>
#include <regex>
#include <string>

namespace zebral
{
/// Default max per file of 5MB.
/// If it gets much bigger, we really shouldn't be streaming straight to a RAM buffer.
static const size_t kDefMaxFileLength = 1024 * 1024 * 5;

class BinaryCurlClient
{
 public:
  BinaryCurlClient(const std::string& usr = "", const std::string& pwd = "");
  virtual ~BinaryCurlClient();

  /// Set authentication info. Right now just does Digest.
  void SetAuthInfo(const std::string& usr, const std::string& pwd);

  /// Called per file received. Return true to continue (if multipart),
  /// false to stop.
  typedef std::function<bool(uint8_t* data, size_t length, const std::vector<std::string>& headers)>
      FileReceivedCallback;

  /// Retrieves the URI and calls callback synchronously.
  /// Returns respose code (200 on success)
  virtual int Get(const std::string& uri, FileReceivedCallback callback,
                  size_t maxLength = kDefMaxFileLength);

 protected:
  // Add headers to be added to the next set..
  // Currently these are cleared by Get(), but may be used elsewhere...
  // If used elsewhere, please clear them after calling curl_easy_perform
  // by calling ClearCustomHeaders.
  void AddCustomHeader(const std::string& header);
  void ClearCustomHeaders();

  CURLcode CurlSetup(const std::string& uri);

  // in-context callback from CURL
  virtual size_t OnWrite(uint8_t* data, size_t length) = 0;
  virtual void OnHeader(const std::string& header);

  // Return true to continue / keep reading future parts.
  // Called for each "file" when we have a multipart stream.
  bool OnGotPart(uint8_t* data, size_t length, const std::vector<std::string>& headers);

  /// Raw CURL callback
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userdata);
  static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);

  CURL* curl_;        ///< libCUrl context
  std::string user_;  ///< username for http auth
  std::string pwd_;   ///< pwd for http auth

  long response_code_;                       ///< Response code (e.g. 200 success, 404 not found)
  size_t max_length_;                        ///< Max size of a file we'll take in.
  FileReceivedCallback callback_;            ///< User callback, called once per file
  std::vector<uint8_t> target_;              ///< Our streaming buffer
  std::vector<std::string> headers_;         ///< Received headers
  std::vector<std::string> custom_headers_;  ///< Our custom outgoing headers
  curl_slist* custom_headers_list_;
};

}  // namespace zebral

#endif  // LIGHTBOX_NETWORK_BINARY_CURL_CLIENT_HPP_
