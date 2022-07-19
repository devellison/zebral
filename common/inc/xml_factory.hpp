#ifndef LIGHTBOX_ZEBRAL_COMMON_XML_FACTORY_HPP_
#define LIGHTBOX_ZEBRAL_COMMON_XML_FACTORY_HPP_

#include <filesystem>
#include <memory>
#include <string>
#include "config.hpp"

namespace zebral
{
/// This is used to initialize underlying C libraries
/// when necessary.
class XMLFactoryInit
{
 public:
  XMLFactoryInit();
  ~XMLFactoryInit();
};

/// Base class for XML-based factories.
class XMLFactory
{
 public:
  XMLFactory() {}
  virtual ~XMLFactory() = default;

  /// Override to create objects of different types that are useful.
  virtual ConfigObject* Create(const std::string& typeName, KeyAttribs& attribs,
                               ConfigObject* parent)
  {
    auto newObj = new ConfigObject(typeName, &attribs, parent);
    return newObj;
  }

 private:
  static std::unique_ptr<XMLFactoryInit> sFactoryInit_;
};

ConfigObject* ParseXml(const std::filesystem::path& path, XMLFactory& factory);
void WriteXml(const std::filesystem::path& path, ConfigObject* root);
}  // namespace zebral

#endif  // LIGHTBOX_ZEBRAL_COMMON_XML_FACTORY_HPP_
