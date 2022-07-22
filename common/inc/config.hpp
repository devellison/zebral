#ifndef LIGHTBOX_ZEBRAL_COMMON_CONFIG_HPP_
#define LIGHTBOX_ZEBRAL_COMMON_CONFIG_HPP_

#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "log.hpp"

namespace zebral
{
/// Configurable objects base class.
typedef std::map<std::string, std::string> KeyAttribs;

/// This base class is for a tree of objects with key/value pairs for attributes that can be
/// constructed from XML or a similar tree structure.
///
/// See XmlFactory for the base factory class.
///
/// For simple usage, it can handle config files and setting up a tree of objects from them
/// with configurable parameters.
class ConfigObject
{
 public:
  ConfigObject(const std::string& typeName, KeyAttribs* attribs = nullptr,
               ConfigObject* parent = nullptr)
      : typeName_(typeName),
        attribs_{(attribs ? (*attribs) : KeyAttribs())},
        parent_{parent}
  {
    if (parent_)
    {
      parent_->AddChild(this);
    }
  }

  virtual ~ConfigObject()
  {
    while (children_.size())
    {
      delete children_.front();
      children_.erase(children_.begin());
    }
  }

  void AddChild(ConfigObject* child)
  {
    child->parent_ = this;
    children_.emplace_back(child);
  }

  /// Get an attrib as a value
  template <class T>
  bool GetAttrib(const std::string& name, T& value, bool useAncestors) const
  {
    auto iter = attribs_.find(name);
    if (iter == attribs_.end())
    {
      if (useAncestors && parent_)
      {
        return parent_->GetAttrib(name, value, useAncestors);
      }
      return false;
    }
    std::stringstream ss(iter->second);
    ss >> value;
    return true;
  }

  /// Specialization to get an attrib as a string value
  bool GetAttrib(const std::string& name, std::string& value, bool useAncestors) const
  {
    auto iter = attribs_.find(name);
    if (iter == attribs_.end())
    {
      if (useAncestors && parent_)
      {
        return parent_->GetAttrib(name, value, useAncestors);
      }
      return false;
    }
    value = iter->second;
    return true;
  }

  template <class T>
  void SetAttrib(const std::string& name, T& value)
  {
    std::stringstream ss;
    ss << value;
    attribs_[name] = ss.str();
  }

  void Dump(size_t depth = 0) const
  {
    std::string indent((depth * 4), ' ');
    ZBA_LOG("{}{}:", indent, typeName_);
    ZBA_LOG("{}  Attributes:", indent);
    for (auto curPair : attribs_)
    {
      ZBA_LOG("{}  {}:{}", indent, curPair.first, curPair.second);
    }
    ZBA_LOG("{}  Children:", indent);
    for (auto curChild : children_)
    {
      curChild->Dump(depth + 1);
    }
  }

  virtual void ToAttribs() {}
  virtual void FromAttribs() {}

  std::string GetTypeName() const
  {
    return typeName_;
  }

 protected:
  std::string typeName_;
  KeyAttribs attribs_;
  ConfigObject* parent_;
  std::vector<ConfigObject*> children_;
};

}  // namespace zebral
#endif  // LIGHTBOX_ZEBRAL_COMMON_CONFIG_HPP_