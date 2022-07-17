/// \file string_utils.cpp
/// String utility functions
#include "string_utils.hpp"
#include <sstream>

namespace zebral
{
std::string remove_invalid_chars(const std::string& src, const std::string& invalid,
                                 char replacement)
{
  std::stringstream filtered;

  for (char curChar : src)
  {
    if (invalid.find(curChar) != std::string::npos)
    {
      if (replacement)
      {
        filtered << replacement;
      }
    }
    else
    {
      filtered << curChar;
    }
  }
  return filtered.str();
}
}  // namespace zebral