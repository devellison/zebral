/// \file camera_frame.hpp
/// CameraFrame class for holding a camera image
#ifndef LIGHTBOX_CAMERA_CAMERA_FRAME_HPP_
#define LIGHTBOX_CAMERA_CAMERA_FRAME_HPP_

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace zebral
{
// Select clock for timestamps
using Clock = std::chrono::system_clock;

/// TimeStamp used by camera calls
using TimeStamp = zebral::Clock::time_point;
/// Get the timestamp right now
/// \return TimeStamp - current time_point on the chosen clock
TimeStamp TimeStampNow();

/// Simple image class.
/// This is meant as a very simple wrapper for an image grabbed from a camera.
/// The intention is to not have the camera API depend on OpenCV, but to allow
/// easy integration with OpenCV or other libraries through a separate file.
///
/// It is NOT meant to be used for image processing - just holding the image from capture
/// until it's out of the library.
///
/// This will ONLY work for images where each pixel channel is 1 T currently,
/// If we decide to support packed formats this will need to get a lot more complex.
class CameraFrame
{
 public:
  /// Default ctor, just zeros everything.
  /// empty() will return true.
  CameraFrame()
      : width_(0),
        height_(0),
        channels_(0),
        bytes_per_channel_(0),
        is_signed_(false),
        is_floating_(false),
        timestamp_(TimeStampNow())
  {
  }

  /// Normal ctor - if data is provided, copies it into the vector.
  /// Otherwise, reserves space for it.
  CameraFrame(int width, int height, int channels, int bytesPerChannel, bool is_signed,
              bool is_floating_point, TimeStamp timestamp = TimeStampNow(),
              const uint8_t* data = nullptr)
      : width_(width),
        height_(height),
        channels_(channels),
        bytes_per_channel_(bytesPerChannel),
        is_signed_(is_signed),
        is_floating_(is_floating_point),
        timestamp_(timestamp)
  {
    /// {TODO} won't work with stepped/padded data.
    size_t dataSize = static_cast<size_t>(width_) * height_ * channels_ * bytes_per_channel_;
    if (data)
    {
      data_.assign((data), (data + dataSize));
    }
    else
    {
      data_.resize(dataSize);
    }
  }

  void clear()
  {
    reset(0, 0, 0, 0, 0, 0);
  }

  void reset(int width, int height, int channels, int bytesPerChannel, bool is_signed,
             bool is_floating_point, TimeStamp timestamp = TimeStampNow(),
             const uint8_t* data = nullptr)
  {
    width_             = width;
    height_            = height;
    channels_          = channels;
    bytes_per_channel_ = bytesPerChannel;
    is_signed_         = is_signed;
    is_floating_       = is_floating_point;
    timestamp_         = timestamp;

    /// {TODO} won't work with stepped/padded data.
    size_t dataSize = static_cast<size_t>(width_) * height_ * channels_ * bytes_per_channel_;
    if (data)
    {
      data_.assign((data), (data + dataSize));
    }
    else
    {
      data_.resize(dataSize);
    }
  }

  /// Returns true if the data buffer has no data.
  /// \return true if frame is empty
  bool empty() const
  {
    return data_.size() == 0;
  }

  /// Width of the image (pixels)
  /// \return int image width in pixels
  int width() const
  {
    return width_;
  }

  /// Height of the image (pixels)
  /// \return int image height in pixels
  int height() const
  {
    return height_;
  }

  /// Number of channels (e.g. RGBA = 4)
  /// \return int number of channels
  int channels() const
  {
    return channels_;
  }

  /// Bytes per pixel per channel, so in RGBA with 8-bits per channel per pixel, is 1
  /// \return int - bytes per pixel per channel
  int bytes_per_channel() const
  {
    return bytes_per_channel_;
  }

  /// Is the data a signed type?
  /// \return true if is signed
  bool is_signed() const
  {
    return is_signed_;
  }

  /// Is the data a floating type?
  /// \return - true if is floating point
  bool is_floating() const
  {
    return is_floating_;
  }

  /// Size of image data in bytes
  /// \return size_t - size of data in bytes
  size_t data_size() const
  {
    return data_.size();
  }

  /// Const Pointer to start of image data
  /// \return const uint8_t* ptr to raw image data
  const uint8_t* data() const
  {
    return data_.data();
  }

  const uint8_t* const_data() const
  {
    return data_.data();
  }

  /// Pointer to start of image data
  /// \return uint8_t* ptr to raw image data
  uint8_t* data()
  {
    return data_.data();
  }

  /// Write the frame to a text-based image for debugging
  /// returns false if not supported for frame type.
  bool write_ppm(std::ostream& out)
  {
    if ((bytes_per_channel_ == 1) && (channels_ == 3))
    {
      // P6 ASCII PPM format
      out << "P6" << std::endl;
      out << width_ << " " << height_ << std::endl;
      out << "255" << std::endl;
      out.write(reinterpret_cast<char*>(data_.data()), data_.size());
      return true;
    }
    else if ((channels_ == 1) && (bytes_per_channel_ <= 2))
    {
      // P5 ASCII PPM format
      out << "P5" << std::endl;
      out << width_ << " " << height_ << std::endl;

      if (bytes_per_channel_ == 1)
      {
        out << "255" << std::endl;
      }
      else if (bytes_per_channel_ == 2)
      {
        out << "65535" << std::endl;
      }
      out.write(reinterpret_cast<char*>(data_.data()), data_.size());
      return true;
    }
    return false;
  }

  TimeStamp get_timestamp() const
  {
    return timestamp_;
  }

  void set_timestamp(TimeStamp timestamp)
  {
    timestamp_ = timestamp;
  }

 protected:
  int width_;                  ///< width of image in pixels
  int height_;                 ///< height of image in pixels
  int channels_;               ///< number of channels (expect interleaved channels in data)
  int bytes_per_channel_;      ///< bytes per pixel per channel
  bool is_signed_;             ///< is data a signed type?
  bool is_floating_;           ///< is data a floating type?
  std::vector<uint8_t> data_;  ///< Vector to store data
  TimeStamp timestamp_;
};

}  // namespace zebral
#endif  // LIGHTBOX_CAMERA_CAMERA_FRAME_HPP_