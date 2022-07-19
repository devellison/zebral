#include "xml_factory.hpp"

#include "errors.hpp"
#include "log.hpp"
#include "platform.hpp"

#if _WIN32

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/base.h>
#include <winrt/windows.data.xml.dom.h>
#include <winrt/windows.foundation.collections.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Data::Xml::Dom;

namespace zebral
{
ConfigObject* ParseNode(IXmlNode node, XMLFactory& factory, ConfigObject* parent = nullptr)
{
  auto typeName = winrt::to_string(node.NodeName());

  // Get attributes
  KeyAttribs keyAttribs;

  auto attribs = node.Attributes();
  for (const auto& curAttrib : attribs)
  {
    auto key        = winrt::to_string(curAttrib.NodeName());
    auto value      = winrt::to_string(winrt::unbox_value<winrt::hstring>(curAttrib.NodeValue()));
    keyAttribs[key] = value;
  }

  ConfigObject* newObj = factory.Create(typeName, keyAttribs, parent);

  // Parse child elements
  for (IXmlNode child : node.ChildNodes())
  {
    if (child.NodeType() == NodeType::ElementNode)
    {
      /*auto childObj = */ ParseNode(child, factory, newObj);
    }
  }

  return newObj;
}

ConfigObject* ParseXml(const std::filesystem::path& path, XMLFactory& factory)
{
  if (!std::filesystem::exists(path))
  {
    ZBA_THROW("File does not exist: " + path.string(), Result::ZBA_FILE_DOES_NOT_EXIST);
  }

  std::string filename = std::filesystem::absolute(path).string();

  winrt::hstring winPath = winrt::to_hstring(filename);

  StorageFile xmlFile = StorageFile::GetFileFromPathAsync(winPath).get();
  XmlDocument doc     = XmlDocument::LoadFromFileAsync(xmlFile).get();
  /// {TODO} error handling
  auto node = doc.FirstChild().try_as<IXmlNode>();

  if (node)
  {
    ConfigObject* root = ParseNode(node, factory);
    return root;
  }
  return nullptr;
}

void WriteXml(const std::filesystem::path& path, ConfigObject* root)
{
  (void)path;
  (void)root;
  ZBA_THROW("Todo", Result::ZBA_ERROR);
}

}  // namespace zebral

#endif  // _WIN32
