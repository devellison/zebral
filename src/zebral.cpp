#define ZEBRAL_EXPORTS
#include "zebral.hpp"
#include "platform.hpp"

static const char* kZEBRAL_VERSION = ZEBRAL_VERSION;

const char* ZebralVersion()
{
  return kZEBRAL_VERSION;
}