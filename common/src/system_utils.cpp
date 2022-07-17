/// \file system_utils.cpp
/// System utility functions
#include "system_utils.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>

#include "errors.hpp"
#include "find_files.hpp"
#include "log.hpp"

namespace zebral
{
#if _WIN32
// Convert between c++ clock and Windows FILETIME.
// This is based on code by Billy O'Neal
// https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#FILETIME

// filetime_duration has the same layout as FILETIME; 100ns intervals
using filetime_duration = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;
// January 1, 1601 (NT epoch) - January 1, 1970 (Unix epoch):
constexpr std::chrono::duration<int64_t> nt_to_unix_epoch{INT64_C(-11644473600)};

std::chrono::system_clock::time_point FILETIME_to_system_clock(FILETIME fileTime)
{
  const filetime_duration asDuration{static_cast<int64_t>(
      (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime)};
  const auto withUnixEpoch = asDuration + nt_to_unix_epoch;

  return std::chrono::system_clock::time_point{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(withUnixEpoch)};
}

FILETIME system_clock_to_FILETIME(std::chrono::system_clock::time_point systemPoint)
{
  const auto asDuration =
      std::chrono::duration_cast<filetime_duration>(systemPoint.time_since_epoch());
  const auto withNtEpoch  = asDuration - nt_to_unix_epoch;
  const uint64_t rawCount = withNtEpoch.count();
  FILETIME result;
  result.dwLowDateTime  = static_cast<DWORD>(rawCount);  // discards upper bits
  result.dwHighDateTime = static_cast<DWORD>(rawCount >> 32);
  return result;
}
#endif

#if __linux__
// Linux USB info
bool GetUSBInfo(const std::string& device_file, const std::string& driverType,
                const std::string& deviceType, const std::string& devicePrefix, uint16_t& vid,
                uint16_t& pid, std::string& bus, std::string& name)
{
  // / sys / bus / usb / drivers / uvcvideo
  std::string driverSearch = zba_format("/sys/bus/usb/drivers/{}/", driverType);
  std::string devSearch    = zba_format("^{}([0-9]*)$", devicePrefix);

  auto usbDevAddrs = FindFiles(driverSearch, "^([0-9-.]*):([0-9-.]*)$");
  for (auto& curDevAddr : usbDevAddrs)
  {
    if (!std::filesystem::exists(curDevAddr.dir_entry.path() / deviceType))
    {
      continue;
    }

    auto usbVideoNames = FindFiles(curDevAddr.dir_entry.path() / deviceType, devSearch);
    for (auto curVideo : usbVideoNames)
    {
      if (device_file == curVideo.dir_entry.path().filename().string())
      {
        // Grab idProduct and idVendor files
        bus = curDevAddr.matches[1];

        std::ifstream nameFile(
            zba_format("/sys/bus/usb/drivers/usb/{}/product", curDevAddr.matches[1]));

        if (nameFile.is_open())
        {
          std::getline(nameFile, name);
        }

        std::ifstream prodFile(
            zba_format("/sys/bus/usb/drivers/usb/{}/idProduct", curDevAddr.matches[1]));
        std::ifstream vendFile(
            zba_format("/sys/bus/usb/drivers/usb/{}/idVendor", curDevAddr.matches[1]));
        if (prodFile.is_open() && vendFile.is_open())
        {
          std::string prodStr, vendStr;
          std::getline(prodFile, prodStr);
          std::getline(vendFile, vendStr);
          vid = std::stol(vendStr, 0, 16);
          pid = std::stol(prodStr, 0, 16);
          return true;
        }
        break;
      }
    }
  }
  return false;
}

#endif  // __linux__

}  // namespace zebral
