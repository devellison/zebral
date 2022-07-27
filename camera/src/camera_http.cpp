#include "camera_http.hpp"
#include "convert.hpp"
#include "errors.hpp"
#include "log.hpp"

// Maybe 5MP? Reserve a big chunk
static constexpr size_t kExpectedImageSize = 1024 * 1024 * 5;

namespace zebral
{
CameraHttp::CameraHttp(const std::string& name, const std::string& uri, const std::string& user,
                       const std::string& pwd)
    : Camera(CameraInfo(name, uri)),
      user_(user),
      pwd_(pwd)
{
}

CameraHttp::~CameraHttp()
{
  Stop();
}

// Enumerate devices
std::vector<CameraInfo> CameraHttp::Enumerate()
{
  // maybe upnp or similar?  or just keep a history and return those?
  ZBA_LOG("No HTTP discovery implemented at this time.");
  std::vector<CameraInfo> info;
  return info;
}

bool CameraHttp::OnBufferReceived(uint8_t* data, size_t length,
                                  const std::vector<std::string>& headers)
{
  if ((!data) || (length == 0))
  {
    ZBA_ERR("Got empty frame?");
    return true;
  }

  (void)headers;
  /// {TODO} Need to process the headers to verify jpeg and get hardware timestamp
  /// BUT, before that, we need to add a way to sync times between camera and system.
  JPEGToBGRFrame(data, length, cur_frame_, cur_frame_.width() * 3);
  cur_frame_.set_timestamp(TimeStampNow());
  OnFrameReceived(cur_frame_);
  return !exiting_;
}

void CameraHttp::ReadThread()
{
  client_ = std::unique_ptr<HttpClient>(new HttpClient(user_, pwd_));

  std::vector<uint8_t> data;
  data_.clear();
  data_.reserve(kExpectedImageSize);
  CameraFrame frame;
  int responseCode = 0;

  while (!exiting_)
  {
    // So, one of two things can happen on the Get() on success...
    // either we get a still image, and then it comes back, or
    // we're in a multipart and we get callbacks with the frames
    // and it doesn't come back until the connection is broken.
    auto receivedCb = std::bind(&CameraHttp::OnBufferReceived, this, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3);

        responseCode = client_->Get(info_.path, receivedCb, kExpectedImageSize);

    if (responseCode != 200)
    {
      ZBA_ERR("Failed to connect to camera: {}.", responseCode);
      break;
    }
  }
  /// {TODO} Add notification that the thread is now deaded.
  client_.reset();
}

/// Creates the capture_ object before thread is started
void CameraHttp::OnStart()
{
  OnStop();
  exiting_ = false;
  thread_  = std::thread(&CameraHttp::ReadThread, this);
}

/// Called once the thread stops to clean up the capture_ object
void CameraHttp::OnStop()
{
  exiting_ = true;
  if (thread_.joinable())
  {
    thread_.join();
  }
}

/// Called to set camera mode. Should throw on failure.
FormatInfo CameraHttp::OnSetFormat(const FormatInfo& info)
{
  ZBA_ERR("WARNING: Set format not supported on base HTTP camera");
  return info;
}

}  // namespace zebral
