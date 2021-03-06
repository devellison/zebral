#include "errors.hpp"
#include "gtest/gtest.h"
#include "log.hpp"
#include "platform.hpp"
#include "serial_line_channel.hpp"

using namespace zebral;

TEST(SerialTest, Enumeration)
{
  auto devices = SerialLineChannel::Enumerate();
  for (const auto& curDevice : devices)
  {
    std::cout << curDevice << std::endl;
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  Platform p;
  return RUN_ALL_TESTS();
}