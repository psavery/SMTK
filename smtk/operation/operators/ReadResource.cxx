//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/ReadResource_xml.h"
#include "smtk/operation/groups/ReaderGroup.h"

#include "nlohmann/json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace smtk
{
namespace operation
{

ReadResource::ReadResource()
{
}

bool ReadResource::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To read a resource, we must have an operation manager from which we access
  // specific read operations.
  if (m_manager.expired())
  {
    return false;
  }

  return true;
}

ReadResource::Result ReadResource::operateInternal()
{
  auto manager = m_manager.lock();

  if (manager == nullptr)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto params = this->parameters();
  auto fileItem = params->findFile("filename");

  std::string type;

  smtk::operation::ReaderGroup readerGroup(manager);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  for (auto fileIt = fileItem->begin(); fileIt != fileItem->end(); ++fileIt)
  {
    std::string filename = *fileIt;

    // Scope so file is only open for a short time:
    {
      std::ifstream file(filename, std::ios::in);
      if (!file.good())
      {
        smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\" for reading.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }

      bool fileTypeKnown = false;
      json j;

      try
      {
        j = json::parse(file);
        type = j.at("type");
        fileTypeKnown = true;
      }
      catch (std::exception&)
      {
      }

      if (!fileTypeKnown)
      {
        try
        {
          for (json::iterator it = j.begin(); it != j.end(); ++it)
          {
            auto jtype = it->find("type");
            if (jtype != it->end() && jtype.value() == "session")
            {
              type = it->find("name").value();
              fileTypeKnown = true;
            }
          }
        }
        catch (std::exception&)
        {
        }
      }

      if (!fileTypeKnown)
      {
        smtkErrorMacro(
          this->log(), "Could not determine resource type for file \"" << filename << "\".");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    smtk::operation::Operation::Ptr readOperation = readerGroup.readerForResource(type);

    if (readOperation == nullptr)
    {
      smtkErrorMacro(this->log(), "Could not find reader for file \""
          << filename << "\" (type = " << type << ").");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    // Set the local reader's filename field.
    smtk::attribute::FileItem::Ptr readerFileItem = readOperation->parameters()->findFile(
      readerGroup.fileItemNameForOperation(readOperation->index()));
    readerFileItem->setValue(filename);

    smtk::operation::Operation::Result readOperationResult = readOperation->operate();
    if (readOperationResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      // An error message should already enter the logger from the local
      // operation.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::resource::ResourcePtr resource = readOperationResult->findResource("resource")->value();
    if (resource == nullptr)
    {
      smtkErrorMacro(
        this->log(), "Error reading file \"" << filename << "\" (type = " << type << ").");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->appendValue(resource);
  }

  return result;
}

const char* ReadResource::xmlDescription() const
{
  return ReadResource_xml;
}

void ReadResource::generateSummary(ReadResource::Result& res)
{
  int outcome = res->findInt("outcome")->value();
  smtk::attribute::FileItemPtr fitem = this->parameters()->findFile("filename");
  std::string label = this->parameters()->definition()->label();
  if (outcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), label << ": read \"" << fitem->value(0) << "\"");
  }
  else
  {
    smtkErrorMacro(this->log(), label << ": failed to read \"" << fitem->value(0) << "\"");
  }
}
}
}
