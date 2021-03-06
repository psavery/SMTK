//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_LoadJSON_h
#define __smtk_io_LoadJSON_h

#include "smtk/CoreExports.h"       // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For Model ResourcePtr

#include "smtk/common/UUID.h"

#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/StringData.h"

struct cJSON;

namespace smtk
{
namespace io
{

class Logger;

/**\brief Import an SMTK model from JSON data.
  *
  * Methods are also provided for importing individual records
  * and groups of records directly from cJSON nodes.
  * These may be used to update relevant entities without
  * storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT LoadJSON
{
public:
  static int intoModelResource(const char* json, smtk::model::ResourcePtr resource);
  static int ofResource(cJSON* body, smtk::model::ResourcePtr resource);
  static int ofResourceEntityData(
    cJSON* body, smtk::model::ResourcePtr resource, smtk::model::BitFlags whatToImport);
  static int ofResourceEntity(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceArrangement(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceTessellation(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceAnalysis(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceFloatProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceStringProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  static int ofResourceIntegerProperties(
    const smtk::common::UUID& uid, cJSON*, smtk::model::ResourcePtr resource);
  // This function has no implementation!
  static int forResourceMeshes(
    smtk::mesh::ManagerPtr meshes, cJSON*, smtk::model::ResourcePtr modelResource);
  // static int ofOperationDefinitions(cJSON*, smtk::model::DefaultSessionPtr destSession);
  static int ofRemoteSession(cJSON*, smtk::model::DefaultSessionPtr destSession,
    smtk::model::ResourcePtr context, const std::string& refPath = std::string());
  static int ofLocalSession(cJSON*, smtk::model::ResourcePtr context, bool loadNativeModels = false,
    const std::string& referencePath = std::string());

  // static int ofOperation(cJSON* node, smtk::operation::OperationPtr& op, smtk::model::ResourcePtr context);
  // static int ofOperationResult(
  //   cJSON* node, Result& resOut, smtk::model::RemoteOperationPtr op);
  static int ofDanglingEntities(cJSON* node, smtk::model::ResourcePtr context);

  static int ofLog(const char* jsonStr, smtk::io::Logger& log);
  static int ofLog(cJSON* logrecordarray, smtk::io::Logger& log);

  //write all mesh collections that have associations to a model
  static int ofMeshesOfModel(cJSON* node, smtk::model::ResourcePtr modelResource,
    const std::string& refPath = std::string());
  //write all mesh properties for the collection
  static int ofMeshProperties(cJSON* node, smtk::mesh::CollectionPtr collection);
  // Mid-level helpers:
  static std::string sessionNameFromTagData(cJSON* tagData);
  static smtk::model::StringList sessionFileTypesFromTagData(cJSON* tagData);

  // Low-level helpers:
  static int getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::common::UUID>& uids);
  static int getStringArrayFromJSON(cJSON* arrayNode, std::vector<std::string>& text);
  static int getIntegerArrayFromJSON(cJSON* arrayNode, std::vector<long>& values);
  static int getRealArrayFromJSON(cJSON* arrayNode, std::vector<double>& values);
};

} // namespace model
} // namespace smtk

#endif // __smtk_io_LoadJSON_h
