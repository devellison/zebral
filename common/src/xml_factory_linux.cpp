#include "xml_factory.hpp"

#include "errors.hpp"
#include "log.hpp"
#include "platform.hpp"

#if __linux__

#include "libxml/parser.h"
#include "libxml/tree.h"

namespace zebral
{
std::unique_ptr<XMLFactoryInit> sFactoryInit_ =
    std::unique_ptr<XMLFactoryInit>(new XMLFactoryInit());

XMLFactoryInit::XMLFactoryInit()
{
  xmlInitParser();
}

XMLFactoryInit::~XMLFactoryInit()
{
  xmlCleanupParser();
}

ConfigObject* ParseNode(xmlNode* node, XMLFactory& factory, ConfigObject* parent = nullptr)
{
  std::string typeName = reinterpret_cast<const char*>(node->name);

  // Get attributes
  KeyAttribs keyAttribs;

  _xmlAttr* curAttrib = node->properties;

  while (curAttrib != nullptr)
  {
    std::string key   = reinterpret_cast<const char*>(curAttrib->name);
    auto prop         = xmlGetProp(node, curAttrib->name);
    std::string value = reinterpret_cast<const char*>(prop);
    xmlFree(prop);
    keyAttribs[key] = value;
    curAttrib       = curAttrib->next;
  }

  ConfigObject* newObj = factory.Create(typeName, keyAttribs, parent);

  // Parse child elements
  xmlNode* child = node->children;
  while (child)
  {
    if (XML_ELEMENT_NODE == child->type)
    {
      /*auto childObj = */ ParseNode(child, factory, newObj);
    }
    child = child->next;
  }

  return newObj;
}

ConfigObject* ParseXml(const std::filesystem::path& path, XMLFactory& factory)
{
  if (!std::filesystem::exists(path))
  {
    ZBA_THROW("File does not exist: " + path.string(), Result::ZBA_FILE_DOES_NOT_EXIST);
  }

  xmlDoc* doc = xmlReadFile(path.string().c_str(), NULL, 0);
  if (!doc)
  {
    ZBA_THROW("Failed to parse XML file:" + path.string(), Result::ZBA_XML_PARSE_FAILED);
  }

  xmlNode* rootXml = xmlDocGetRootElement(doc);
  if (!rootXml)
  {
    xmlFree(doc);
    ZBA_THROW("XML file has no root element:" + path.string(), Result::ZBA_XML_PARSE_FAILED);
  }

  ConfigObject* root = ParseNode(rootXml, factory);

  xmlFree(rootXml);
  xmlFree(doc);

  return root;
}
void WriteXml(const std::filesystem::path& path, ConfigObject* root)
{
  (void)path;
  (void)root;

  ZBA_THROW("Todo", Result::ZBA_ERROR);
}

}  // namespace zebral

#endif  // __linux__