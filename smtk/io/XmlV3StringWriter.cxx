//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV3StringWriter.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/io/Logger.h"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

namespace smtk
{
namespace io
{

XmlV3StringWriter::XmlV3StringWriter(const attribute::ResourcePtr myResource)
  : XmlV2StringWriter(myResource)
{
}

XmlV3StringWriter::~XmlV3StringWriter()
{
}

std::string XmlV3StringWriter::className() const
{
  return std::string("XmlV3StringWriter");
}

unsigned int XmlV3StringWriter::fileVersion() const
{
  return 3;
}

std::string XmlV3StringWriter::rootNodeName() const
{
  return std::string("SMTK_AttributeResource");
}

void XmlV3StringWriter::processDefinitionInternal(
  xml_node& definition, smtk::attribute::DefinitionPtr def)
{
  if (!def->tags().empty())
  {
    xml_node tagsNode = definition.append_child();
    tagsNode.set_name("Tags");

    std::string sep; // TODO: The writer could accept a user-provided separator.
    for (auto& tag : def->tags())
    {
      xml_node tagNode = tagsNode.append_child();
      tagNode.set_name("Tag");
      tagNode.append_attribute("Name").set_value(tag.name().c_str());
      if (!tag.values().empty())
      {
        tagNode.text().set(concatenate(tag.values(), sep, &m_logger).c_str());
      }
    }
  }
  XmlV2StringWriter::processDefinitionInternal(definition, def);
}

void XmlV3StringWriter::processItemDefinitionType(
  xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  switch (idef->type())
  {
    case Item::DateTimeType:
      this->processDateTimeDef(node, smtk::dynamic_pointer_cast<DateTimeItemDefinition>(idef));
      break;

    case Item::ReferenceType:
      this->processReferenceDef(node, std::dynamic_pointer_cast<ReferenceItemDefinition>(idef));
      break;

    case Item::ResourceType:
      this->processResourceDef(node, std::dynamic_pointer_cast<ResourceItemDefinition>(idef));
      break;

    case Item::ComponentType:
      this->processComponentDef(node, std::dynamic_pointer_cast<ComponentItemDefinition>(idef));
      break;

    default:
      XmlV2StringWriter::processItemDefinitionType(node, idef);
      break;
  }
}

void XmlV3StringWriter::processDateTimeDef(
  pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());

  std::string format = idef->displayFormat();
  if (!format.empty())
  {
    node.append_attribute("DisplayFormat").set_value(format.c_str());
  }
  node.append_attribute("ShowTimeZone") = idef->useTimeZone();
  node.append_attribute("ShowCalendarPopup") = idef->useCalendarPopup();

  if (idef->hasDefault())
  {
    xml_node defnode = node.append_child("DefaultValue");
    ::smtk::common::DateTimeZonePair dtz = idef->defaultValue();
    defnode.text().set(dtz.serialize().c_str());
  }

  // if (idef->hasRange())
  //   {
  //   xml_node rnode = node.append_child("RangeInfo");
  //   xml_node r;
  //   bool inclusive;
  //   if (idef->hasMinRange())
  //     {
  //     r = rnode.append_child("Min");
  //     inclusive = idef->minRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->minRange()));
  //     }
  //   if (idef->hasMaxRange())
  //     {
  //     r = rnode.append_child("Max");
  //     inclusive = idef->maxRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->maxRange()));
  //     }
  //   }
}

void XmlV3StringWriter::processItemType(xml_node& node, smtk::attribute::ItemPtr item)
{
  switch (item->type())
  {
    case Item::DateTimeType:
      this->processDateTimeItem(node, smtk::dynamic_pointer_cast<DateTimeItem>(item));
      break;

    case Item::ReferenceType:
      this->processReferenceItem(node, smtk::dynamic_pointer_cast<ReferenceItem>(item));
      break;

    case Item::ResourceType:
      this->processResourceItem(node, smtk::dynamic_pointer_cast<ResourceItem>(item));
      break;

    case Item::ComponentType:
      this->processComponentItem(node, smtk::dynamic_pointer_cast<ComponentItem>(item));
      break;

    default:
      XmlV2StringWriter::processItemType(node, item);
      break;
  }
}

void XmlV3StringWriter::processDateTimeItem(pugi::xml_node& node, attribute::DateTimeItemPtr item)
{
  size_t numValues = item->numberOfValues();
  xml_node val;
  if (numValues == 0)
  {
    return;
  }

  // (else)
  if (numValues == 1)
  {
    if (item->isSet())
    {
      ::smtk::common::DateTimeZonePair dtz = item->value();
      node.text().set(dtz.serialize().c_str());
    }
    else
    {
      node.append_child("UnsetVal");
    }
    return;
  }

  // (else)
  xml_node values = node.append_child("Values");
  for (std::size_t i = 0; i < numValues; ++i)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      ::smtk::common::DateTimeZonePair dtz = item->value();
      val.text().set(dtz.serialize().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV3StringWriter::processReferenceDef(
  pugi::xml_node& node, smtk::attribute::ReferenceItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ReferenceLabels");
}

void XmlV3StringWriter::processReferenceItem(pugi::xml_node& node, attribute::ReferenceItemPtr item)
{
  size_t i = 0, n = item->numberOfValues();
  std::size_t numRequiredVals = item->numberOfRequiredValues();
  // we should always have "NumberOfValues" set
  node.append_attribute("NumberOfValues").set_value(static_cast<unsigned int>(n));

  xml_node val;
  if (!n)
  {
    return;
  }

  if ((numRequiredVals == 1) && (!item->isExtensible()))
  {
    if (item->isSet())
    {
      val = node.append_child("Val");

      // Because resource links are not serialized into XML, we add enough dta
      // to each reference item to recreate the link.
      auto objKey = item->objectKey(i);

      xml_node key = val.append_child("Key");
      auto key1 = key.append_child("_1_");
      key1.text().set(objKey.first.toString().c_str());
      auto key2 = key.append_child("_2_");
      key2.text().set(objKey.second.toString().c_str());

      const smtk::resource::Surrogate& surrogate =
        item->attribute()->resource()->links().data().value(objKey.first);

      xml_node rhs = val.append_child("RHS");
      auto rhs1 = rhs.append_child("_1_");
      rhs1.text().set(
        item->attribute()->resource()->links().data().at(objKey.first).right.toString().c_str());
      auto rhs2 = rhs.append_child("_2_");
      rhs2.text().set(item->attribute()
                        ->resource()
                        ->links()
                        .data()
                        .at(objKey.first)
                        .at(objKey.second)
                        .right.toString()
                        .c_str());

      val.append_attribute("Role").set_value(
        item->attribute()->resource()->links().data().at(objKey.first).at(objKey.second).role);

      xml_node surrogateNode = val.append_child("Surrogate");
      surrogateNode.append_attribute("Index").set_value(
        static_cast<unsigned int>(surrogate.index()));
      surrogateNode.append_attribute("TypeName").set_value(surrogate.typeName().c_str());
      surrogateNode.append_attribute("Id").set_value(surrogate.id().toString().c_str());
      surrogateNode.append_attribute("Location").set_value(surrogate.location().c_str());
    }
    return;
  }
  xml_node values = node.append_child("Values");
  for (i = 0; i < n; i++)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");

      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));

      // Because resource links are not serialized into XML, we add enough dta
      // to each reference item to recreate the link.
      auto objKey = item->objectKey(i);

      xml_node key = val.append_child("Key");
      auto key1 = key.append_child("_1_");
      key1.text().set(objKey.first.toString().c_str());
      auto key2 = key.append_child("_2_");
      key2.text().set(objKey.second.toString().c_str());

      const smtk::resource::Surrogate& surrogate =
        item->attribute()->resource()->links().data().value(objKey.first);

      xml_node rhs = val.append_child("RHS");
      auto rhs1 = rhs.append_child("_1_");
      rhs1.text().set(
        item->attribute()->resource()->links().data().at(objKey.first).right.toString().c_str());
      auto rhs2 = rhs.append_child("_2_");
      rhs2.text().set(item->attribute()
                        ->resource()
                        ->links()
                        .data()
                        .at(objKey.first)
                        .at(objKey.second)
                        .right.toString()
                        .c_str());

      val.append_attribute("Role").set_value(
        item->attribute()->resource()->links().data().at(objKey.first).at(objKey.second).role);

      xml_node surrogateNode = val.append_child("Surrogate");
      surrogateNode.append_attribute("Index").set_value(
        static_cast<unsigned int>(surrogate.index()));
      surrogateNode.append_attribute("TypeName").set_value(surrogate.typeName().c_str());
      surrogateNode.append_attribute("Id").set_value(surrogate.id().toString().c_str());
      surrogateNode.append_attribute("Location").set_value(surrogate.location().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

void XmlV3StringWriter::processResourceDef(
  pugi::xml_node& node, smtk::attribute::ResourceItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ResourceLabels");
}

void XmlV3StringWriter::processResourceItem(pugi::xml_node& node, attribute::ResourceItemPtr item)
{
  this->processReferenceItem(node, item);
}

void XmlV3StringWriter::processComponentDef(
  pugi::xml_node& node, smtk::attribute::ComponentItemDefinitionPtr idef)
{
  this->processReferenceDefCommon(node, idef, "ComponentLabels");
}

void XmlV3StringWriter::processComponentItem(pugi::xml_node& node, attribute::ComponentItemPtr item)
{
  this->processReferenceItem(node, item);
}

void XmlV3StringWriter::processReferenceDefCommon(pugi::xml_node& node,
  smtk::attribute::ReferenceItemDefinitionPtr idef, const std::string& labelName)
{
  auto acceptableEntries = idef->acceptableEntries();
  xml_node accnode = node.append_child("Accepts");
  for (auto entry : acceptableEntries)
  {
    xml_node rsrcnode = accnode.append_child("Resource");
    rsrcnode.append_attribute("Name").set_value(entry.first.c_str());
    if (!entry.second.empty())
    {
      rsrcnode.append_attribute("Filter").set_value(entry.second.c_str());
    }
  }

  if (idef->lockType() != smtk::resource::LockType::DoNotLock)
  {
    node.append_attribute("LockType").set_value(static_cast<unsigned int>(idef->lockType()));
  }

  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());
  if (idef->isExtensible())
  {
    node.append_attribute("Extensible") = true;

    if (idef->maxNumberOfValues())
      node.append_attribute("MaxNumberOfValues") =
        static_cast<unsigned int>(idef->maxNumberOfValues());
  }

  if (idef->hasValueLabels())
  {
    xml_node lnode = node.append_child();
    lnode.set_name(labelName.c_str());
    if (idef->usingCommonLabel())
    {
      lnode.append_attribute("CommonLabel") = idef->valueLabel(0).c_str();
    }
    else
    {
      size_t i, n = idef->numberOfRequiredValues();
      xml_node ln;
      for (i = 0; i < n; i++)
      {
        ln = lnode.append_child();
        ln.set_name("Label");
        ln.set_value(idef->valueLabel(i).c_str());
      }
    }
  }
}

} // namespace io
} // namespace smtk
