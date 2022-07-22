/// \file serial_line_channel.hpp
/// Simple line-based serial communications.
#ifndef LIGHTBOX_SERIAL_SERIAL_LINE_CHANNEL_HPP_
#define LIGHTBOX_SERIAL_SERIAL_LINE_CHANNEL_HPP_

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "serial_info.hpp"

namespace zebral
{
/// Serial line communications class
/// This is a really simplified line-based serial class designed to
/// read and write a line at a time over a USB-Serial port.
class SerialLineChannel
{
 public:
  /// Retrieve a list of current devices
  /// \returns std::vector<SerialInfo> List of filled SerialInfo structs
  static std::vector<SerialInfo> Enumerate();

  /// Callback that receives one line of text at a time
  typedef std::function<void(const std::string& incoming)> ReadCallback;

  /// construct and open a serial device from an info.
  SerialLineChannel(const SerialInfo& info, ReadCallback = nullptr, unsigned int baud = 115200);

  /// close serial device in dtor
  ~SerialLineChannel();

  /// Writes a string (with CR added!) to the port
  void Write(const std::string& outLine);

 protected:
  /// Called when we receive a chunk of data.
  /// Manages buffer and calls OnReceivedLine for each line.
  void OnReceivedData(const uint8_t* data, size_t length);

  /// Called for each line of data we received. Calls callback_ if non-null.
  void OnReceivedLine(const std::string& incoming);

  /// Internal callback used if we aren't given one, just dumps the data to stdout
  static void StdioCallback(const std::string& incoming);

  SerialInfo info_;          ///< Info about the device
  ReadCallback callback_;    ///< Per-line callback
  std::thread read_thread_;  ///< Reader thread
  std::string read_buffer_;  ///< Accumulates strings until line ends
  unsigned int baud_;        ///< User-specified speed of port (115200 default)

  /// The implementation sits in the Impl object
  /// to avoid throwing all sorts of stuff into the header.
  class Impl;
  std::unique_ptr<Impl> impl_;  ///< Platform-specific implementation
};

}  // namespace zebral
#endif  // LIGHTBOX_SERIAL_SERIAL_LINE_CHANNEL_HPP_
