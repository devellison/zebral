#include "serial_line_channel.hpp"

#include "log.hpp"
#include "platform.hpp"
namespace zebral
{
void SerialLineChannel::StdioCallback(const std::string& incoming)
{
  std::cout << incoming << std::endl;
}

void SerialLineChannel::OnReceivedData(const uint8_t* data, size_t length)
{
  if (length == 0) return;

  size_t start = 0;

  for (size_t i = 0; i < length; ++i)
  {
    switch (data[i])
    {
      case '\n':
      case '\r':
      case 0:
        // Got an end of line - append it to buffer and send if not empty
        // (we don't send blank lines right now, and we do strip crlf)
        read_buffer_ += std::string(reinterpret_cast<const char*>(data + start), i - start);
        start = i + 1;

        if (!read_buffer_.empty())
        {
          OnReceivedLine(read_buffer_);
          read_buffer_ = "";
        }
        break;

      default:
        break;
    }
  }

  // Store any unterminated data in the buffer for next time
  if (start < length)
  {
    read_buffer_ += std::string(reinterpret_cast<const char*>(data + start), length - start);
  }
}

/// Called for each line of data we received. Calls callback_ if non-null.
void SerialLineChannel::OnReceivedLine(const std::string& incoming)
{
  if (callback_)
  {
    callback_(incoming);
  }
}

}  // namespace zebral