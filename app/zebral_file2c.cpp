/// \zebral_file2c.cpp
/// Converts files to C declarations useful for embedding
/// Binary files are converted to a const byte array in hex.
/// Text files are converted to a const char* that's readable.

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "string_utils.hpp"

using namespace zebral;

enum b2c_errors
{
  B2C_SUCCESS        = 0,
  B2C_INVALID_PARAMS = 1,
  B2C_FILE_NOT_FOUND,
  B2C_FILE_OPEN_ERROR,
  B2C_FILE_BINARY,
  B2C_FILE_TEXT
};

std::string get_variable_name(const std::filesystem::path& source_path)
{
  return remove_invalid_chars(source_path.stem().string(), "!@#$%^&*()-=+[]{}\\|;':\",.<>/?", '_');
}

std::string escape_line(const std::string& input)
{
  std::stringstream ss;
  for (auto curChar : input)
  {
    switch (curChar)
    {
      case '"':
        ss << "\\" << curChar;
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '\r':
      case '\n':
        break;
      default:
        ss << curChar;
        break;
    }
  }
  return ss.str();
}

int is_file_binary(const std::filesystem::path& source_path)
{
  int result = B2C_FILE_TEXT;

  std::ifstream source(source_path, std::ios::binary);
  if (!source.is_open())
  {
    std::cerr << "Could not open file: " << source_path.string() << std::endl;
    return B2C_FILE_OPEN_ERROR;
  }

  int curPos   = 0;
  char curChar = 0;
  while (source.get(curChar))
  {
    if (!std::isprint(curChar) && !std::isspace(curChar))
    {
      std::cerr << "Non-printable character '" << static_cast<char>(curChar) << "' (" << curChar
                << ") at " << curPos << std::endl;
      result = B2C_FILE_BINARY;
      break;
    }
    curPos++;
  }

  return result;
}

int binary2c(const std::filesystem::path& source_path)
{
  std::ifstream source(source_path, std::ios::binary);
  if (!source.is_open())
  {
    std::cerr << "Could not open file: " << source_path.string() << std::endl;
    return B2C_FILE_OPEN_ERROR;
  }

  uint8_t chr;

  std::string var_name = get_variable_name(source_path);

  bool eol                         = false;
  static constexpr int line_length = 16;
  int count                        = 0;
  size_t file_len                  = std::filesystem::file_size(source_path);
  std::cout << "extern const uint8_t " << var_name << "[" << file_len << "];" << std::endl;
  std::cout << "const uint8_t " << var_name << "[" << file_len << "] = {" << std::endl;
  std::cout << std::hex;

  while (source.read(reinterpret_cast<char*>(&chr), 1))
  {
    if (count)
    {
      std::cout << ", ";
      eol = (!(count % line_length));
      if (eol)
      {
        std::cout << std::endl;
      }
    }

    std::cout << "0x" << static_cast<unsigned int>(chr);
    ++count;
  }

  if (!eol)
  {
    std::cout << std::endl;
  }
  std::cout << std::dec << "};" << std::endl;
  return B2C_SUCCESS;
}

int text2c(const std::filesystem::path& source_path)
{
  std::string var_name = get_variable_name(source_path);
  std::ifstream source(source_path, std::ios::binary);

  if (!source.is_open())
  {
    std::cerr << "Could not open file: " << source_path.string() << std::endl;
    return B2C_FILE_OPEN_ERROR;
  }

  std::cout << "static const char* " << var_name << " = " << std::endl;

  std::string inputLine;
  int lineNum = 0;
  while (std::getline(source, inputLine))
  {
    std::cout << "  \"" << escape_line(inputLine) << "\\n\"" << std::endl;
    ++lineNum;
  }

  std::cout << ";" << std::endl;
  return B2C_SUCCESS;
}

int main(int argc, char** argv)
{
  int result = B2C_SUCCESS;
  if (argc != 2)
  {
    std::cerr << "Usage: zebral_file2c FILE" << std::endl;
    return B2C_INVALID_PARAMS;
  }

  if (!std::filesystem::exists(argv[1]))
  {
    std::cerr << "Cannot find file: " << argv[1];
    return B2C_FILE_NOT_FOUND;
  }

  result = is_file_binary(argv[1]);
  switch (result)
  {
    case B2C_FILE_BINARY:
      // Convert binary file to C bytes...
      result = binary2c(argv[1]);
      break;
    case B2C_FILE_TEXT:
      // Convert text file to C bytes
      result = text2c(argv[1]);
      break;
    default:
      break;
  }

  return result;
}