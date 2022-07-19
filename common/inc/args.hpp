/// \file args.hpp
/// Standalone command line processor
#ifndef LIGHTBOX_ZEBRAL_COMMON_ARGS_HPP_
#define LIGHTBOX_ZEBRAL_COMMON_ARGS_HPP_

#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "platform.hpp"

namespace zebral
{
/// Config entry for building args help and processing
/// Make a static vector of these to pass to Args contructor.
struct ArgsConfigEntry
{
  const char* shortname;    ///< switch name (--shortname)
  char switchchar;          ///< Switch character (-s)
  const char* valuename;    ///< If not null, has a value
                            ///< --shortname VALUENAME
                            ///< --shortname=VALUENAME
                            ///< --shortnameVALUENAME all work.
  const char* description;  ///< Description for help
};

/// Basic command line arguments. To be expanded on later.
///
/// Terminology - switch: An argument that takes a "-" or "--"
///               parameter: a switch that has an argument
///               flag: A switch without an argument, true by being present.
///               argument: unswitched (standalone) command line argument
///
/// To use, build a static table and pass it to the constructor along
/// with the current argc and argv.  It will be parsed.
///
/// If has_errors(), then something didn't match from the command line -
/// suggest calling display_errors(), display_help(), then exiting.
///
/// Otherwise, you can then call the functions to get the command line
/// properties.
///
/// For flags and parameters, you can use get() to automatically convert.
/// NOTE: type must have operator>> with istream for this to work.
///
/// For arguments, check the num_arguments(), then get them by index.
///
///   {"switch",'s',"VALUE","Example switch that takes a value"}
///
/// Multiple styles will work for parameters. With the above table
/// entry, all of these options should provide "switch" with a
/// value of "value":
///
///   --switch=value        -s=values
///   --switch value        -s value
///   --switchvalue         -svalue
///
/// Spaces are supported if quoted.
class Args
{
 public:
  /// Constructor processes arguments based on table.
  /// Can call has_errors() to see if errors were encountered.
  Args(int argc, char** argv, const std::vector<ArgsConfigEntry>& configTable)
      : configTable_(configTable)
  {
    for (int i = 0; i < argc; ++i)
    {
      raw_.emplace_back(argv[i]);
    }

    for (int i = 1; i < argc; ++i)
    {
      if (raw_[i][0] == '-')
      {
        if (raw_[i][1] == '-')
        {
          // shortname - "--shortname"
          i += process_shortname(i);
        }
        else
        {
          // char
          i += process_switch_char(i);
        }
      }
      else
      {
        arguments_.emplace_back(raw_[i]);
      }
    }
  }

  /// Dtor
  ~Args() {}

  /// Returns true if flag (switch with no argument) was found on command line
  bool has_flag(const std::string& shortname) const
  {
    return flags_.count(shortname) > 0;
  }

  /// Returns true if parameter was found
  bool has_parameter(const std::string& shortname) const
  {
    return parameters_.count(shortname) > 0;
  }

  /// Retrieves an optional parameter string given the name
  std::optional<std::string> get_parameter(const std::string& shortname) const
  {
    auto iter = parameters_.find(shortname);
    if (iter == parameters_.end()) return {};
    return iter->second;
  }

  /// Retrieves a ref to all arguments found
  const std::vector<std::string>& get_arguments() const
  {
    return arguments_;
  }

  /// Gets number of arguments found
  size_t num_arguments() const
  {
    return arguments_.size();
  }

  /// Gets an indexed argument (no switch/param)
  ///
  /// Currently returns an empty string if out of range.
  /// Debatable whether this or throwing is better.
  /// You can check num_arguments() first.
  std::string get_argument(size_t index) const
  {
    if (index < num_arguments())
    {
      return arguments_[index];
    }
    return "";
  }

  /// Get a list of unknown parameters
  const std::vector<std::string>& get_unknown() const
  {
    return unknown_;
  }

  /// Get number of unknown parameters
  size_t num_unknown() const
  {
    return unknown_.size();
  }

  /// Get list of missing parameters
  const std::vector<std::string>& get_missing() const
  {
    return missing_;
  }

  /// Get number missing parameters
  size_t num_missing() const
  {
    return missing_.size();
  }

  /// Display unknown options / missing arguments
  void display_errors(std::ostream& os = std::cerr) const
  {
    if (unknown_.size())
    {
      os << "Unknown options:" << std::endl;
      for (const auto& curEntry : unknown_)
      {
        os << "    " << curEntry << std::endl;
      }
    }

    if (missing_.size())
    {
      os << "Options missing arguments:" << std::endl;
      for (const auto& curEntry : missing_)
      {
        os << "    " << curEntry << std::endl;
      }
    }

    if (has_errors())
    {
      os << std::endl;
    }
  }

  /// Display help built from table
  void display_help(const std::string& usage_line, std::ostream& os = std::cerr) const
  {
    os << usage_line << std::endl << std::endl;
    os << zba_format("       {:12s} {}   {:20s} {:40s}", "OPTION_NAME", "CHAR", "PARAMETER",
                     "DESCRIPTION")
       << std::endl;
    ;
    for (const auto& curEntry : configTable_)
    {
      std::string helpString =
          zba_format("     --{:12s} (-{})   {:20s} {:40s}", curEntry.shortname, curEntry.switchchar,
                     (curEntry.valuename ? curEntry.valuename : ""), curEntry.description);
      os << helpString << std::endl;
    }
  }

  /// Returns true if missing/unknown arguments
  bool has_errors() const
  {
    return missing_.size() + unknown_.size() > 0;
  }

  // You can use any type stringstream can convert,
  // or add your own conversions.
  //
  // std::string is handled special so it doesn't
  // tokenize.
  //
  // Flags are set true if used on those and exists.
  //
  // Returns true if parameter/flag found, false otherwise.
  // If true (found) result is set. Otherwise it is left
  // at what it was.
  template <class T>
  bool get(const std::string& name, T& result)
  {
    auto p = get_parameter(name);
    if (!p)
    {
      if (has_flag(name))
      {
        result = 1;
        return true;
      }
      return false;
    }

    std::stringstream ss(*p);
    ss >> result;
    return true;
  }

  /// Special case for string to avoid tokenizing
  bool get(const std::string& name, std::string& result)
  {
    auto p = get_parameter(name);
    if (!p)
    {
      if (has_flag(name))
      {
        result = 1;
        return true;
      }
      return false;
    }
    result = *p;
    return true;
  }

 protected:
  /// Processes a short name (--shortname) switch
  int process_shortname(size_t index)
  {
    bool found    = false;
    int skip_args = 0;
    for (auto& entry : configTable_)
    {
      size_t shortLen = strlen(entry.shortname);
      if (0 != strncmp(entry.shortname, &raw_[index][2], shortLen))
      {
        continue;
      }

      found = true;
      if (!entry.valuename)
      {
        flags_.insert(entry.shortname);
        return skip_args;
      }

      std::string value;
      // Got a match
      if (raw_[index][2 + shortLen] == ' ')
      {
        // next argument
        skip_args = 1;
        if (index > raw_.size() - 1)
        {
          // past end but we require an argument
          missing_.emplace_back(raw_[index]);
          continue;
        }
        value = raw_[index + 1];
      }
      else if (raw_[index][2 + shortLen] == '=')
      {
        // after equals
        value = &raw_[index][2 + shortLen + 1];
      }
      else
      {
        // adjacent
        value = &raw_[index][2 + shortLen];
      }

      if (value.empty())
      {
        missing_.emplace_back(raw_[index]);
      }
      parameters_[entry.shortname] = value;
    }

    if (!found)
    {
      unknown_.emplace_back(raw_[index]);
    }
    return skip_args;
  }

  /// Processes a char switch (-s) switch
  int process_switch_char(size_t index)
  {
    int skip_args = 0;
    bool found    = false;

    for (auto& entry : configTable_)
    {
      if (entry.switchchar != raw_[index][1])
      {
        continue;
      }

      found = true;

      if (!entry.valuename)
      {
        flags_.insert(entry.shortname);
        return skip_args;
      }

      std::string value;
      // Got a match
      if (raw_[index][2] == 0)
      {
        // next argument
        skip_args = 1;
        if (index > raw_.size() - 1)
        {
          // past end but we require an argument
          missing_.emplace_back(raw_[index]);
          continue;
        }
        value = raw_[index + 1];
      }
      else if (raw_[index][2] == '=' || raw_[index][2] == ' ')
      {
        // after equals
        value = &raw_[index][2 + 1];
      }
      else
      {
        // adjacent
        value = &raw_[index][2];
      }

      if (value.empty())
      {
        missing_.emplace_back(raw_[index]);
      }
      parameters_[entry.shortname] = value;
    }
    if (!found)
    {
      unknown_.emplace_back(raw_[index]);
    }

    return skip_args;
  }

 protected:
  const std::vector<ArgsConfigEntry>& configTable_;  ///< Configuration table
  std::vector<std::string> raw_;                     ///< argv[] strings unprocessed
  std::vector<std::string> arguments_;               ///< Unswitched arguments
  std::map<std::string, std::string> parameters_;    ///< Specified parameters with values
  std::set<std::string> flags_;       ///< Switches/Flags with no values beyond being present
  std::vector<std::string> unknown_;  ///< unknown arguments
  std::vector<std::string> missing_;  ///< parameters missing their argument
};
}  // namespace zebral

#endif  // LIGHTBOX_ZEBRAL_COMMON_ARGS_HPP_