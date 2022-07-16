/// \file string_utils.hpp
/// String utility functions
#ifndef LIGHTBOX_ZEBRAL_COMMON_STRING_UTILS_HPP_
#define LIGHTBOX_ZEBRAL_COMMON_STRING_UTILS_HPP_
#include <string>
namespace zebral
{
std::string remove_invalid_chars(const std::string& src, const std::string& invalid,
                                 char replacement = 0);
}  // namespace zebral

#endif  // LIGHTBOX_ZEBRAL_COMMON_STRING_UTILS_HPP_