/// \file http_client.cpp
/// Hopefully simple http client
#include "http_client.hpp"
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
static constexpr const char *kReContent = "^multipart/x-mixed-replace;boundary=(.*)";

HttpClient::HttpClient(const std::string &user, const std::string &pwd)
    : BinaryCurlClient(user, pwd),
      content_regex_(kReContent, std::regex_constants::ECMAScript),
      streaming_(false),
      part_index_(0)
{
}

HttpClient::~HttpClient() {}

size_t HttpClient::OnWrite(uint8_t *data, size_t length)
{
  // Don't attempt to parse server responses other than 200 OK
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);
  if (response_code_ != 200)
  {
    ZBA_LOG("Response code: {}", response_code_);
    return 0;
  }

  if (content_type_.empty())
  {
    char *ctype = nullptr;
    curl_easy_getinfo(curl_, CURLINFO_CONTENT_TYPE, &ctype);
    if (ctype)
    {
      content_type_ = ctype;
    }
    boundary_.clear();
    streaming_ = false;
    std::smatch matches;

    if ((std::regex_match(content_type_, matches, content_regex_)) && matches.size() > 1)
    {
      std::string bstr = matches[1].str();
      boundary_.insert(boundary_.begin(), bstr.begin(), bstr.end());
      ZBA_LOG("Boundary: {}", bstr);
      streaming_  = true;
      part_index_ = 0;
    }
  }

  bool cancel_stream = false;

  if (length)
  {
    target_.insert(target_.end(), data, data + length);

    if (!boundary_.empty())
    {
      bool gotPart = false;

      // Possible we might get more that one part at a time, so loop through
      // any we have
      do
      {
        gotPart = false;
        std::vector<std::uint8_t>::iterator iter =
            std::search(target_.begin(), target_.end(),
                        std::boyer_moore_searcher(boundary_.begin(), boundary_.end()));
        if (iter != target_.end())
        {
          gotPart = true;

          size_t chunk_size = std::distance(target_.begin(), iter);

          // Remove possible garbage bytes at end from boundary marker
          if ((target_[chunk_size - 1] == '-') && (target_[chunk_size - 2] == '-'))
          {
            chunk_size -= 2;
            if ((target_[chunk_size - 1] == 0x0a) || (target_[chunk_size - 1] == 0x0d))
              chunk_size--;
            if ((target_[chunk_size - 1] == 0x0a) || (target_[chunk_size - 1] == 0x0d))
              chunk_size--;
          }

          // Collect part headers and filter out of data
          std::vector<std::string> headers;
          size_t headerLen = collect_headers(target_.data(), chunk_size, headers);
          if (headerLen && (chunk_size > 3) && (chunk_size > headerLen))
          {
            cancel_stream = !OnGotPart(target_.data() + headerLen, chunk_size - headerLen, headers);
            part_index_++;
          }

          // Erase the chunk of memory we sent out plus the first chunk header...
          target_.erase(target_.begin(), iter + boundary_.size());
        }
      } while (gotPart && (!cancel_stream));
    }
  }

  // If callback cancelled, then return 0 length.
  return cancel_stream ? 0 : length;
}

size_t HttpClient::collect_headers(uint8_t *data, size_t length, std::vector<std::string> &headers)
{
  std::string curString;
  char *curPtr        = reinterpret_cast<char *>(data);
  bool last_was_empty = false;

  while (curPtr < reinterpret_cast<const char *>(data + length))
  {
    switch (*curPtr)
    {
      case 0:
      case 0x0d:
        break;
      case 0x0a:
        if (!curString.empty())
        {
          headers.emplace_back(curString);
          curString.clear();
        }
        else
        {
          if (last_was_empty)
          {
            // done.
            curPtr++;
            size_t size = static_cast<size_t>(curPtr - reinterpret_cast<const char *>(data));
            return size;
          }
          last_was_empty = true;
        }
        break;
      default:
        curString += *curPtr;
        break;
    }
    curPtr++;
  }
  ZBA_ERR("Didn't find end of headers in part?");
  return 0;
}

int HttpClient::Get(const std::string &uri, FileReceivedCallback callback, size_t maxLength)
{
  streaming_ = false;
  boundary_.clear();
  content_type_.clear();
  part_index_ = 0;

  response_code_ = BinaryCurlClient::Get(uri, callback, maxLength);

  if (streaming_ && (response_code_ == 200))
  {
    ZBA_LOG("Streaming ended after {} parts", part_index_);
  }

  // If we didn't stream a multipart, send a callback with the data now.
  if (!streaming_ && (response_code_ == 200))
  {
    std::vector<std::string> headers;

    // Add content-type header
    char *ctype = nullptr;
    curl_easy_getinfo(curl_, CURLINFO_CONTENT_TYPE, &ctype);
    if (ctype)
    {
      std::string header = "Content Type: ";
      header += ctype;
      headers.emplace_back(header);
    }
    if (callback_)
    {
      callback_(target_.data(), target_.size(), headers);
    }
  }

  return response_code_;
}

}  // namespace zebral