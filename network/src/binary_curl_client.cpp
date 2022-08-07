#include "binary_curl_client.hpp"
/// \file binary_curl_client.cpp
/// Base class for CURL-based network clients that grab binary files.
#include <curl/curl.h>
#include <curl/easy.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <vector>

#include "log.hpp"
#include "platform.hpp"

namespace zebral
{
BinaryCurlClient::BinaryCurlClient(const std::string &user, const std::string &pwd)
    : curl_(curl_easy_init()),
      user_(user),
      pwd_(pwd),
      response_code_(0),
      max_length_(0),
      custom_headers_list_(nullptr)
{
}

BinaryCurlClient::~BinaryCurlClient()
{
  if (curl_)
  {
    ClearCustomHeaders();
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
  }
}

CURLcode BinaryCurlClient::CurlSetup(const std::string &uri)
{
  CURLcode res;
  if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_URL, uri.c_str()))) return res;
  if (CURLE_OK !=
      (res = curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, BinaryCurlClient::HeaderCallback)))
    return res;
  if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_HEADERDATA, this))) return res;
  if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_VERBOSE, 0L))) return res;
  if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1L))) return res;
  if (!user_.empty() && !(pwd_.empty()))
  {
    if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST))) return res;
    if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_USERNAME, user_.c_str()))) return res;
    if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_PASSWORD, pwd_.c_str()))) return res;
  }
  if (CURLE_OK !=
      (res = curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, BinaryCurlClient::WriteCallback)))
    return res;
  if (CURLE_OK != (res = curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this))) return res;

  return res;
}

void BinaryCurlClient::AddCustomHeader(const std::string &header)
{
  custom_headers_.emplace_back(header);
}

int BinaryCurlClient::Get(const std::string &uri, FileReceivedCallback callback, size_t maxLength)
{
  if (!curl_)
  {
    ZBA_ERR("CURL not initialized!");
    return 0;
  }
  callback_      = callback;
  response_code_ = 0;
  max_length_    = maxLength;
  target_.clear();

  // Free up header list if it exists (shouldn't?)
  if (custom_headers_list_)
  {
    curl_slist_free_all(custom_headers_list_);
    custom_headers_list_ = nullptr;
  }

  // Now generate a list from our headers_
  if (custom_headers_.size())
  {
    for (const auto &curHeader : custom_headers_)
    {
      ZBA_LOG("Adding header: {}", curHeader);
      custom_headers_list_ = curl_slist_append(custom_headers_list_, curHeader.c_str());
    }
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, custom_headers_list_);
  }

  CurlSetup(uri);
  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

  CURLcode res = curl_easy_perform(curl_);

  if (0 == response_code_)
  {
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);
  }

  if (custom_headers_list_)
  {
    curl_slist_free_all(custom_headers_list_);
    custom_headers_list_ = nullptr;
  }

  if (res != CURLE_OK)
  {
    // So... this could be an error, but it also happens when we stream for a while, then cancel.
    // Handle specific cases as needed.
    ZBA_LOG("curl_easy_perform returned {}", static_cast<int>(res));
  }

  if (200 != response_code_)
  {
    ZBA_LOG("Response {} for {}", response_code_, uri.c_str());
  }

  ClearCustomHeaders();
  return response_code_;
}

void BinaryCurlClient::ClearCustomHeaders()
{
  custom_headers_.clear();
  if (custom_headers_list_)
  {
    curl_slist_free_all(custom_headers_list_);
    custom_headers_list_ = nullptr;
  }
}

void BinaryCurlClient::SetAuthInfo(const std::string &usr, const std::string &pwd)
{
  user_ = usr;
  pwd_  = pwd;
}

bool BinaryCurlClient::OnGotPart(uint8_t *data, size_t length,
                                 const std::vector<std::string> &headers)
{
  if (callback_)
  {
    return callback_(data, length, headers);
  }
  return true;
}

size_t BinaryCurlClient::WriteCallback(void *contents, size_t size, size_t nmemb, void *userdata)
{
  BinaryCurlClient *client = reinterpret_cast<BinaryCurlClient *>(userdata);
  return client->OnWrite(reinterpret_cast<uint8_t *>(contents), size * nmemb);
}
size_t BinaryCurlClient::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  BinaryCurlClient *client = reinterpret_cast<BinaryCurlClient *>(userdata);
  std::string header(buffer, size * nitems);
  client->OnHeader(header);

  return size * nitems;
}

void BinaryCurlClient::OnHeader(const std::string &header)
{
  headers_.emplace_back(header);
}

}  // namespace zebral
