#if _WIN32
#include "serial_line_channel.hpp"

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Serialcommunication.h>
#include <winrt/Windows.Devices.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/base.h>

#include "errors.hpp"
#include "log.hpp"
#include "platform.hpp"

using namespace winrt;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace zebral
{
/// Windows C++/RT implementation of SerialLineChannel
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
  SerialDevice device_;        ///< Windows SerialDevice
  volatile bool exiting_;      ///< Exiting flag
};

SerialLineChannel::Impl::Impl(SerialLineChannel* parent)
    : parent_(*parent),
      device_(SerialDevice::FromIdAsync(winrt::to_hstring(parent_.info_.bus)).get()),
      exiting_(false)

{
  ZBA_LOG("Starting serial: {} @ {}", parent_.info_.bus, parent_.baud_);
  device_.BaudRate(parent_.baud_);
  device_.Parity(SerialParity::None);
  device_.DataBits(8);
  device_.StopBits(SerialStopBitCount::One);
  device_.Handshake(SerialHandshake::None);
  device_.IsDataTerminalReadyEnabled(false);
  device_.IsRequestToSendEnabled(false);
}

SerialLineChannel::Impl::~Impl() {}

void SerialLineChannel::Impl::Write(const std::string& outputLine)
{
  std::string out = outputLine;
  DataWriter dataWriter{device_.OutputStream()};

  std::vector<uint8_t> output(reinterpret_cast<const uint8_t*>(out.c_str()),
                              reinterpret_cast<const uint8_t*>(out.c_str() + out.length()));

  dataWriter.WriteBytes(output);
  dataWriter.StoreAsync().get();
  dataWriter.DetachStream();
}

void SerialLineChannel::Impl::ReadThread()
{
  using namespace std::chrono_literals;
  Platform p;

  device_.ReadTimeout(100ms);
  auto dataReader = DataReader(device_.InputStream());
  dataReader.InputStreamOptions(InputStreamOptions::Partial | InputStreamOptions::ReadAhead);
  while (!exiting_)
  {
    try
    {
      auto async = dataReader.LoadAsync(1);

      switch (async.wait_for(50ms))
      {
        case AsyncStatus::Started:
          // Nothing received
          async.Cancel();
          break;
        case AsyncStatus::Completed:
        {
          auto length = async.GetResults();
          auto buffer = dataReader.ReadBuffer(length);
          parent_.OnReceivedData(buffer.data(), buffer.Length());
        }
        break;
      }
    }
    catch (const hresult_error&)
    {
    }
  }
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
  std::vector<SerialInfo> deviceList;
  winrt::hstring serial_selector = SerialDevice::GetDeviceSelector();

  auto devCol = DeviceInformation::FindAllAsync(serial_selector).get();

  for (const auto& device : devCol)
  {
    auto devInfo           = DeviceInformation::CreateFromIdAsync(device.Id()).get();
    std::string deviceName = winrt::to_string(devInfo.Name());

    auto serialDevice = SerialDevice::FromIdAsync(device.Id()).get();

    // If we can't get the port, it's most likely in use.
    if (!serialDevice)
    {
      ZBA_LOG("{} is currently in use and cannot be queried.", deviceName);
      continue;
    }

    deviceList.emplace_back(winrt::to_string(serialDevice.PortName()), deviceName,
                            winrt::to_string(device.Id()), serialDevice.UsbVendorId(),
                            serialDevice.UsbProductId());
  }

  return deviceList;
}

}  // namespace zebral
#endif  // _WIN32