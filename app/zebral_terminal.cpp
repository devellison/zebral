#include "args.hpp"
#include "log.hpp"
#include "serial_info.hpp"
#include "serial_line_channel.hpp"

using namespace zebral;

void on_read(const std::string& incoming)
{
  std::cout << " " << incoming << std::endl;
}

int start_terminal(const SerialInfo& info, unsigned int baud)
{
  SerialLineChannel channel(info, on_read, baud);
  std::string inputLine;
  while (std::getline(std::cin, inputLine))
  {
    if (0 == zba_strcasecmp(inputLine.c_str(), "exit"))
    {
      break;
    }
    // For now, exit the terminal when "exit" is typed alone on a line.
    channel.Write(inputLine);
  }
  return 0;
}

static const std::vector<ArgsConfigEntry> kArgTable = {
    {"baud", 'b', "BAUD", "Speed of serial port (e.g. 115200, 9600, etc.)"},
    {"help", '?', nullptr, "Show help"},
    {"list", 'l', nullptr, "List serial devices"}};

int main(int argc, char** argv)
{
  Platform p;

  Args args(argc, argv, kArgTable);
  if (args.has_errors())
  {
    args.display_errors();
  }

  auto devices = SerialLineChannel::Enumerate();
  if (args.has_flag("list"))
  {
    std::cout << "Known devices:" << std::endl;
    for (const auto& curDevice : devices)
    {
      std::cout << "   " << curDevice << std::endl;
    }
    // Exit if there aren't arguments given.
    if (args.num_arguments() == 0)
    {
      return 0;
    }
    std::cout << std::endl;
  }

  if (args.has_errors() || args.has_flag("help") || (args.num_arguments() != 1))
  {
    args.display_help("Usage: zebral_terminal [OPTIONS] SERIAL_PORT");
    std::cerr << std::endl;
    std::cerr << "Examples: zebral_terminal COM3 --baud 115200" << std::endl;
    std::cerr << "          zebral_terminal /dev/ttyACM0" << std::endl;
    return 1;
  }

  unsigned long baud = 115200;
  args.get("baud", baud);

  std::string port = args.get_argument(0);

  bool foundDevice = false;
  int result       = 0;

  for (auto& curDevice : devices)
  {
    if (0 == zba_strcasecmp(curDevice.path.c_str(), port.c_str()))
    {
      result      = start_terminal(curDevice, baud);
      foundDevice = true;
      break;
    }
  }

  if (!foundDevice)
  {
    std::cerr << "Could not find device: " << port << std::endl;
    return 2;
  }

  return result;
}