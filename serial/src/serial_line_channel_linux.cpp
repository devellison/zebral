#if __linux__
#include "serial_line_channel.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>

#include "errors.hpp"
#include "find_files.hpp"
#include "log.hpp"
#include "platform.hpp"
#include "results.hpp"
#include "system_utils.hpp"

namespace zebral
{
static constexpr size_t kBufferSize = 2048;

class SerialLineChannel::Impl
{
 public:
  Impl(SerialLineChannel* parent);
  ~Impl();

  /// Thread that waits for incoming data and reads it
  void ReadThread();

  /// Writes the string to the port
  void Write(const std::string& outputLine);

  SerialLineChannel& parent_;  ///< Parent SerialLineChannel object
  volatile bool exiting_;      ///< Exiting flag
  Result read_error_;          ///< Read error status, if any
  int serial_port_;            ///< Serial port file descriptor
  struct termios tty;
};

SerialLineChannel::Impl::Impl(SerialLineChannel* parent)
    : parent_(*parent),
      exiting_(false),
      read_error_(Result::ZBA_OK),
      serial_port_(-1)
{
  serial_port_ = ::open(parent_.info_.path.c_str(), O_RDWR | O_NONBLOCK);
  if (-1 == serial_port_)
  {
    ZBA_THROW("Failed to open " + parent_.info_.path, Result::ZBA_SERIAL_OPEN_FAILED);
  }

  if (flock(serial_port_, LOCK_EX | LOCK_NB) == -1)
  {
    ZBA_THROW("Serial port already locked.", Result::ZBA_SERIAL_OPEN_FAILED);
  }

  if (tcgetattr(serial_port_, &tty) != 0)
  {
    ZBA_THROW("Failed to configure serial port." + parent_.info_.path,
              Result::ZBA_SERIAL_OPEN_FAILED);
  }

  tty.c_cflag &= CSIZE;
  tty.c_cflag |= CS8;                      // 8-bit
  tty.c_cflag &= ~CSTOPB;                  // 1 stop bitr
  tty.c_cflag &= ~PARENB;                  // No parity
  tty.c_cflag &= ~CRTSCTS;                 // Disable flow control
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Disable Xon/Xoff flow ctrl
  tty.c_cflag |= CREAD | CLOCAL;           // Read and ignore ctrls
  tty.c_lflag &= ~ICANON;  // Hmmm.. revisit - this is the mode we're in, want termios to do it?
  tty.c_lflag &= ~ECHO;    // No echo for now
  tty.c_lflag &= ~ISIG;    // Disable signal chars
  tty.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // And the other ones
  tty.c_oflag &= ~OPOST;                                             // Disable output filters
  tty.c_oflag &= ~ONLCR;                                             // Disable conversion of CR/LF

  // Timing... revisit this
  tty.c_cc[VTIME] = 1;
  tty.c_cc[VMIN]  = 0;

  int speed = B115200;
  switch (parent_.baud_)
  {
    case 300:
      speed = B300;
      break;
    case 600:
      speed = B600;
      break;
    case 1200:
      speed = B1200;
      break;
    case 1800:
      speed = B1800;
      break;
    case 2400:
      speed = B2400;
      break;
    case 4800:
      speed = B4800;
      break;
    case 9600:
      speed = B9600;
      break;
    case 19200:
      speed = B19200;
      break;
    case 38400:
      speed = B38400;
      break;
    case 57600:
      speed = B57600;
      break;
    case 230400:
      speed = B230400;
      break;
    case 460800:
      speed = B460800;
      break;
    case 500000:
      speed = B500000;
      break;
    case 576000:
      speed = B576000;
      break;
    case 921600:
      speed = B921600;
      break;
    case 1000000:
      speed = B1000000;
      break;
    case 1152000:
      speed = B1152000;
      break;
    case 1500000:
      speed = B1500000;
      break;
    case 2000000:
      speed = B2000000;
      break;
    case 2500000:
      speed = B2500000;
      break;
    case 3000000:
      speed = B3000000;
      break;
    case 3500000:
      speed = B3500000;
      break;
    case 4000000:
      speed = B4000000;
      break;

    default:
      ZBA_LOG("Unknown baud {}. Defaulting to 115200", parent_.baud_);
      [[fallthrough]];
    case 115200:
      speed = B115200;
      break;
  }
  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  if (tcsetattr(serial_port_, TCSANOW, &tty) != 0)
  {
    ZBA_THROW("Failed to configure serial port." + parent_.info_.path,
              Result::ZBA_SERIAL_OPEN_FAILED);
  }
}

SerialLineChannel::Impl::~Impl()
{
  if (-1 != serial_port_)
  {
    ::close(serial_port_);
    serial_port_ = -1;
  }
}

void SerialLineChannel::Impl::Write(const std::string& outputLine)
{
  int amount_to_write = outputLine.length();

  int offset = 0;

  do
  {
    int amount_written = ::write(serial_port_, outputLine.c_str() + offset, amount_to_write);
    if (amount_written == -1)
    {
      if (EAGAIN == errno) continue;
      ZBA_THROW("Error writting to port!", Result::ZBA_SERIAL_WRITE_ERROR);
    }

    offset += amount_written;
    amount_to_write -= amount_written;

  } while (amount_to_write > 0);
}

void SerialLineChannel::Impl::ReadThread()
{
  Platform p;
  uint8_t buffer[kBufferSize];
  read_error_ = Result::ZBA_OK;
  while (!exiting_)
  {
    fd_set readfs;

    timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 10 * 1000;  // 10 ms

    // Wait until there's something on the line to read
    FD_ZERO(&readfs);
    FD_SET(serial_port_, &readfs);
    int result = ::select(serial_port_ + 1, &readfs, NULL, NULL, &tv);
    if (result == -1)
    {
      if (EAGAIN == errno) continue;

      ZBA_LOG("Error from select()");
      read_error_ = Result::ZBA_SERIAL_READ_ERROR;
      break;
    }
    else if (result == 0)
    {
      // timeout, just continue
      continue;
    }

    // Read what we can
    if (FD_ISSET(serial_port_, &readfs))
    {
      int r = ::read(serial_port_, buffer, kBufferSize);
      if (r == -1)
      {
        if (EAGAIN == errno) continue;
        read_error_ = Result::ZBA_SERIAL_READ_ERROR;
        ZBA_LOG("Error reading from serial");
        break;
      }
      parent_.OnReceivedData(buffer, r);
    }
  }

  // Mark that we're exiting, even if we exited on error.
  exiting_ = true;
}

SerialLineChannel::SerialLineChannel(const SerialInfo& info, ReadCallback callback,
                                     unsigned int baud)
    : info_(info),
      callback_(callback ? callback : StdioCallback),
      baud_(baud)
{
  impl_        = std::make_unique<Impl>(this);
  read_thread_ = std::thread(&SerialLineChannel::Impl::ReadThread, impl_.get());
}

SerialLineChannel::~SerialLineChannel()
{
  if (read_thread_.joinable())
  {
    impl_->exiting_ = true;
    read_thread_.join();
  }
  impl_.reset();
}

void SerialLineChannel::Write(const std::string& outputLine)
{
  impl_->Write(outputLine + "\n");
}

std::vector<SerialInfo> SerialLineChannel::Enumerate()
{
  std::vector<SerialInfo> serialList;

  /// {TODO} right now we're just enumerating USB CDC_ACM devices (modern USB serial devices)
  ///        and ttyUSB devices.
  ///        should probably extend this to other types of serial, but these do what I need.
  auto serial_devs = FindFiles("/dev/", "ttyACM([0-9]+)");
  for (auto curMatch : serial_devs)
  {
    std::string path      = curMatch.dir_entry.path().string();
    std::string path_file = curMatch.dir_entry.path().filename().string();

    std::string name;
    std::string bus;
    uint16_t vid = 0;
    uint16_t pid = 0;

    GetUSBInfo(path_file, "cdc_acm", "tty", "ttyACM", vid, pid, bus, name);
    serialList.emplace_back(path, name, bus, vid, pid);
  }

  auto usb_devs = FindFiles("/dev/", "ttyUSB([0-9]+)");
  for (auto curMatch : usb_devs)
  {
    std::string path      = curMatch.dir_entry.path().string();
    std::string path_file = curMatch.dir_entry.path().filename().string();

    std::string name;
    std::string bus;
    uint16_t vid = 0;
    uint16_t pid = 0;

    /// {TODO} These work just a little different, with no deviceType folder.
    /// Find a better way to do this that's fully generic.
    /// (These are other common USB->Serial adapters)
    if (!GetUSBInfo(path_file, "cp210x", "", "ttyUSB", vid, pid, bus, name))
    {
      GetUSBInfo(path_file, "ch341", "", "ttyUSB", vid, pid, bus, name);
    }
    serialList.emplace_back(path, name, bus, vid, pid);
  }

  return serialList;
}

}  // namespace zebral

#endif  //__linux__
