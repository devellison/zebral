/// \file camera_http.hpp
/// Header for HTTP cameras
#ifndef LIGHTBOX_CAMERA_CAMERA_HTTP_HPP_
#define LIGHTBOX_CAMERA_CAMERA_HTTP_HPP_

#include "camera.hpp"
#include "http_client.hpp"

namespace zebral
{
struct CameraInfo;

/// Baseline HTTP camera - this is intended more to derive from
/// than to use by itself, and is geared mostly towards the Zebral ESP32-CAM
/// firmware.  However, it should be able to take mostly any http:// image stream
/// that's either:
///
/// 1.) Just an image/jpeg serving URI, or...
/// 2.) A multipart image/jpeg video URI
///
/// #2 is quite a bit faster as there's no re-negotiation of the connection.
class CameraHttp : public Camera
{
 public:
  CameraHttp(const std::string& name, const std::string& uri, const std::string& user,
             const std::string& pwd);
  virtual ~CameraHttp();

  // Enumerate devices
  static std::vector<CameraInfo> Enumerate();

 protected:
  /// Creates the capture_ object before thread is started
  void OnStart() override;

  /// Called once the thread stops to clean up the capture_ object
  void OnStop() override;

  /// Called to set camera mode. Should throw on failure.
  FormatInfo OnSetFormat(const FormatInfo& info) override;

  void ReadThread();

 protected:
  bool OnBufferReceived(uint8_t* data, size_t length, const std::vector<std::string>& headers);

  std::string user_;
  std::string pwd_;
  std::unique_ptr<HttpClient> client_;
  std::thread thread_;
  std::vector<uint8_t> data_;
};
}  // namespace zebral
#endif  // LIGHTBOX_CAMERA_CAMERA_HTTP_HPP_
