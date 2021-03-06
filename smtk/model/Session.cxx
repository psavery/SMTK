//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Session.h"

#include "smtk/model/ArrangementHelper.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionIOJSON.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

using smtk::attribute::Definition;
using smtk::attribute::DefinitionPtr;
using smtk::attribute::IntItemDefinition;
using smtk::attribute::ModelEntityItemDefinition;
using smtk::attribute::ComponentItemDefinition;
using smtk::attribute::StringItemDefinition;

namespace smtk
{
namespace model
{

/// Default constructor. This assigns a random session ID to each Session instance.
Session::Session()
  : m_sessionId(smtk::common::UUID::random())
  , m_resource(nullptr)
{
}

/// Destructor.
Session::~Session()
{
}

/**\brief Return the name of the session type (i.e., the name of the modeling kernel).
  *
  * Subclasses override this method by using the smtkDeclareModelingKernel
  * and smtkImplementsModelingKernel macros.
  */
std::string Session::name() const
{
  return "invalid";
}

/**\brief Return the session ID for this instance of the session.
  *
  * Sessions are ephemeral and tied to a particular machine so
  * they should generally not be serialized.
  */
smtk::common::UUID Session::sessionId() const
{
  return m_sessionId;
}

/**\brief Transcribe an entity from a foreign modeler into an SMTK storage Resource.
  *
  * On input, the \a entity will not be valid but if transcription is
  * successful, the \a requested records in the \a entity's Resource will
  * be valid. If \a requested includes SESSION_ENTITY_TYPE, then
  * \a entity.isValid() should return true after this call.
  *
  * Only honor requests for entity IDs listed as dangling unless
  * \a onlyDangling is false (default is true).
  * This prevents expensive requests by Resource instances over many Sessions.
  *
  * The return value is 0 upon failure and non-zero upon success.
  * Failure occurs when any \a requested bits of information that
  * are in Sessione::allSupportedInformation() are not transcribed,
  * or when \a requested is 0.
  */
int Session::transcribe(
  const EntityRef& entity, SessionInfoBits requested, bool onlyDangling, int depth)
{
  int retval = 0;
  if (requested)
  {
    // Check that the entity ID is dangling or we are forced to continue.
    DanglingEntities::iterator it = m_dangling.find(entity);
    if (onlyDangling && it == m_dangling.end())
    { // The session has not been told that this UUID exists.
      return retval;
    }
    // Ask the subclass to transcribe information.
    SessionInfoBits actual = this->transcribeInternal(entity, requested, depth);
    // Decide which bits of the request can possibly be honored...
    SessionInfoBits honorable = requested & this->allSupportedInformation();
    // ... and verify that all of those have been satisfied.
    retval = (honorable & actual) == honorable;
    // If transcription is complete, then remove the UUID from the dangling
    // entity set. Note that we must refresh the iterator since transcribeInternal
    // may have modified m_dangling.
    if (((actual & this->allSupportedInformation()) == this->allSupportedInformation()) &&
      ((it = m_dangling.find(entity)) != m_dangling.end()))
      m_dangling.erase(it);
  }
  return retval;
}

/**\brief Return a bit vector describing what types of information can be transcribed.
  *
  * This is used to determine when an entity has been fully transcribed into storage
  * and is no longer "dangling."
  */
SessionInfoBits Session::allSupportedInformation() const
{
  return SESSION_EVERYTHING;
}

/// Return the map from dangling entityrefs to bits describing their partial transcription state.
const DanglingEntities& Session::danglingEntities() const
{
  return m_dangling;
}

/**\brief Mark an entity, \a ent, as partially transcribed.
  *
  * Subclasses should call this method when a UUID has been assigned
  * to a model entity but ent.resource() has not yet been populated with
  * all of the information about the entity. The information which *is*
  * \a present in ent.resource() should be passed but will default to
  * zero (i.e., the UUID exists in some other entity's relations but
  * has no records in resource itself).
  *
  * The entity is added to the list of dangling entities and will be
  * removed from the list when a call to \a transcribeInternal indicates
  * that Session::allSupportedInformation() is now present in resource.
  */
void Session::declareDanglingEntity(const EntityRef& ent, SessionInfoBits present)
{
  if ((present & this->allSupportedInformation()) < this->allSupportedInformation())
    m_dangling[ent] = present;
  else
    m_dangling.erase(ent);
}

/**\brief Set configuration options on the session.
  *
  * Subclasses may override this method to accept configuration
  * options specific to their backends.
  * When \a optName and \a optVal are acceptable, the
  * method returns 1; otherwise a zero or negative value is returned.
  */
int Session::setup(const std::string& optName, const StringList& optVal)
{
  (void)optName;
  (void)optVal;
  return 0;
}

/// Return a reference to the resource that owns this Session.
Resource::Ptr Session::resource() const
{
  return m_resource ? m_resource->shared_from_this() : Resource::Ptr();
}

/// Return a reference to the mesh resource for this Session.
smtk::mesh::ManagerPtr Session::meshManager() const
{
  if (m_resource)
  {
    return m_resource->meshes();
  }
  else
  {
    return smtk::mesh::Manager::Ptr();
  }
}

/// Return the log (obtained from the model resource).
smtk::io::Logger& Session::log()
{
  return this->resource()->log();
}

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  *
  * Currently, it really only makes sense to call this method on a
  * Model (i.e., not an edge, face, etc.); entire models at a time
  * are retranscribed.
  *
  * Subclasses may override this method.
  * If they do not, they should implement the virtual relationship helper methods.
  */
SessionInfoBits Session::transcribeInternal(
  const EntityRef& entRef, SessionInfoBits flags, int depth)
{
  (void)depth;
  SessionInfoBits actual = SESSION_NOTHING;
  EntityPtr entRec = m_resource->findEntity(entRef.entity(), false);
  if (!entRec)
    entRec = this->addEntityRecord(entRef);

  // Get a subclass-specific helper for validating/repairing/creating arrangements
  ArrangementHelper* helper = this->createArrangementHelper();

  // Now recursively find all related entities.
  // This marks entRef, resets it (removing all relations), and stores state in the helper
  // as required to re-transcribe the state in a manner as consistent with the previous state
  // as possible. (For example the helper might store the sense number of an edge or vertex
  // with respect to its parent face or edge so that retranscription results in the same
  // senses if possible.)
  this->findOrAddRelatedEntities(entRef, flags, helper);
  helper->doneAddingEntities(this->shared_from_this(), flags);

  // We must re-find entRec because the addition of other entities may
  // have caused a reallocation (in hash-based storage):
  entRec = m_resource->findEntity(entRef.entity(), false);

  actual |= this->findOrAddArrangements(entRef, entRec, flags, helper);
  actual |= this->updateProperties(entRef, entRec, flags, helper);
  actual |= this->updateTessellation(entRef, flags, helper);
  delete helper;

  // Return what we actually transcribed so that parent can update
  // the dangling entity map.
  return actual;
}

/**\brief Set the session ID.
  *
  * Do not call this unless you are preparing the session
  * to be a remote mirror of a modeling session (for, e.g.,
  * client-server operation).
  */
void Session::setSessionId(const smtk::common::UUID& sessId)
{
  m_sessionId = sessId;
}

/// Inform this instance of the session that it is owned by \a resource.
void Session::setResource(Resource* resource)
{
  m_resource = resource;
}

/**\brief Called when an entity is being split so that attribute assignments can be updated.
  *
  * The default implementation removes all attributes from \a from and
  * assigns those same attributes to each entity of \a to.
  */
bool Session::splitAttributes(const EntityRef& from, const EntityRefs& to) const
{
  bool ok = true;
  smtk::model::Resource::Ptr resource = from.resource();
  if (!resource)
  {
    return ok;
  }

  // Fetch the attributes of the source entity.
  // If there are none, then return with success.
  std::set<smtk::attribute::AttributePtr> attrs;
  if (!resource->insertEntityAssociations(from, attrs))
  {
    return ok;
  }

  // If the output entities do not include the input,
  // remove attributes from the input as otherwise
  // adding them to the target entities might be disallowed
  // by the attribute resource.
  if (to.find(from) == to.end())
  {
    EntityRef mutableFrom(from);
    ok &= mutableFrom.disassociateAttributes(attrs);
  }

  for (auto attr : attrs)
  {
    for (auto ent : to)
    {
      if (ent != from && ent.isValid())
      {
        ok &= ent.associateAttribute(attr->attributeResource(), attr->id());
      }
    }
  }
  return ok;
}

/**\brief Called when an entity is being split so that attribute assignments can be updated.
  *
  * The default implementation removes all attributes from \a from and
  * assigns those same attributes to each entity of \a to.
  */
bool Session::mergeAttributes(const EntityRefs& from, EntityRef& to) const
{
  bool ok = true;
  if (from.empty())
  {
    return ok;
  }

  smtk::model::Resource::Ptr resource = from.begin()->resource();
  if (!resource)
  {
    resource = this->resource();
    if (!resource)
    {
      std::cerr << "Warning: No model resource when trying to merge attributes.\n";
      return ok;
    }
  }

  // Capture attributes in {from \ to} and disassociate them.
  // Be careful not to remove attributes from the target
  // entity as we might not be able to re-associate it later
  // (if the attribute may only exist on one entity, for example).
  std::set<smtk::attribute::AttributePtr> attrs;
  for (auto ent : from)
  {
    if (ent == to || !ent.isValid())
    {
      continue; // Leave the target entity's attributes alone. Ignore invalid entities.
    }
    resource->insertEntityAssociations(ent, attrs);
    ok &= ent.disassociateAttributes(attrs);
  }
  if (attrs.empty())
  {
    return ok; // No attributes on any entity in \a from == success unless we could not disassociate...
  }

  // Add attributes previously in {from \ to} to target (to).
  for (auto attr : attrs)
  {
    ok &= to.associateAttribute(attr->attributeResource(), attr->id());
  }
  return ok;
}

/**\brief This is used by the resource when erasing a model entity.
  *
  * Subclasses should implement this and erase all of the string, integer,
  * and floating-point properties (as specified by \a propFlags) that
  * their modeling kernel allows them to reproduce when transcribe() is
  * called.
  * The properties should only be erased from \a ent's model resource, not
  * from the underlying modeling kernel.
  *
  * Do *not* erase properties like name, color, and visibility unless
  * they are stored by the underlying modeling kernel in addition to
  * SMTK.
  *
  * Return true when at least one property was removed, false otherwise.
  */
bool Session::removeGeneratedProperties(const EntityRef& ent, SessionInfoBits propFlags)
{
  (void)ent;
  (void)propFlags;
  return false;
}

/**\brief Called when an entity is being split or cut into one or more descendants.
  *
  * Sessions are responsible for overriding this to handle propagation of any
  * property values.
  *
  * Return true on success and false on failure.
  * It is not a failure for an entity to be missing properties.
  * Depending on the context, it may be an error for a destination entity
  * to already have a property when it is inconsistent with the value on \a from.
  *
  * The base class implementation simply copies the "pedigree" property
  * to the descendants.
  */
bool Session::splitProperties(const EntityRef& from, const EntityRefs& to) const
{
  const char* intPropertyNamesToBroadcast[] = { "pedigree id" };
  unsigned npc = sizeof(intPropertyNamesToBroadcast) / sizeof(intPropertyNamesToBroadcast[0]);

  const char* stringPropertyNamesToBroadcast[] = { "name" };
  unsigned nps = sizeof(stringPropertyNamesToBroadcast) / sizeof(stringPropertyNamesToBroadcast[0]);

  const char* floatPropertyNamesToBroadcast[] = { "color" };
  unsigned npf = sizeof(floatPropertyNamesToBroadcast) / sizeof(floatPropertyNamesToBroadcast[0]);

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Split rule for integers: broadcast value to all.
  const IntegerData& idata(from.integerProperties());
  for (unsigned i = 0; i < npc; ++i)
  {
    IntegerData::const_iterator ipit;
    if ((ipit = idata.find(intPropertyNamesToBroadcast[i])) != idata.end())
    {
      for (EntityRefs::iterator oit = to.begin(); oit != to.end(); ++oit)
      {
        EntityRef mutableEnt(*oit);
        mutableEnt.setIntegerProperty(intPropertyNamesToBroadcast[i], ipit->second);
      }
    }
  }

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Split rule for strings: broadcast value to one (the first by UUID).
  for (unsigned int ii = 0; ii < nps; ++ii)
  {
    const StringData& sdata(from.stringProperties());
    StringData::const_iterator spit;
    if (!to.empty())
    { // Copy name only to the first entity.
      EntityRef mutableEnt(*to.begin());
      if ((spit = sdata.find(stringPropertyNamesToBroadcast[ii])) != sdata.end())
      {
        mutableEnt.setStringProperty(stringPropertyNamesToBroadcast[ii], spit->second);
      }
    }
  }

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Split rule for floats: broadcast value to all.
  const FloatData& fdata(from.floatProperties());
  for (unsigned i = 0; i < npf; ++i)
  {
    FloatData::const_iterator fpit;
    if ((fpit = fdata.find(floatPropertyNamesToBroadcast[i])) != fdata.end())
    {
      for (EntityRefs::iterator oit = to.begin(); oit != to.end(); ++oit)
      {
        EntityRef mutableEnt(*oit);
        mutableEnt.setFloatProperty(floatPropertyNamesToBroadcast[i], fpit->second);
      }
    }
  }
  return true;
}

/**\brief Called when one or more entities are being merged or joined into a descendant.
  *
  * Sessions are responsible for overriding this to handle propagation of any
  * property values.
  *
  * Return true on success and false on failure.
  *
  * The base class implementation simply copies the union of the "pedigree" properties
  * from the ancestors to the descendant.
  */
bool Session::mergeProperties(const EntityRefs& from, EntityRef& to) const
{
  const char* intPropertyNamesToReduce[] = { "pedigree id" };
  unsigned npc = sizeof(intPropertyNamesToReduce) / sizeof(intPropertyNamesToReduce[0]);

  const char* stringPropertyNamesToReduce[] = { "name" };
  unsigned nps = sizeof(stringPropertyNamesToReduce) / sizeof(stringPropertyNamesToReduce[0]);

  const char* floatPropertyNamesToReduce[] = { "color" };
  unsigned npf = sizeof(floatPropertyNamesToReduce) / sizeof(floatPropertyNamesToReduce[0]);

  std::cout << "Merging from:\n";
  for (auto fent : from)
  {
    std::cout << "  " << fent.name() << "\n";
  }
  std::cout << "to:\n  " << to.name() << "\n";

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Merge rule for integers: union all values.
  std::set<int> imerged;
  for (unsigned i = 0; i < npc; ++i)
  {
    EntityRefs::const_iterator eit;
    for (eit = from.begin(); eit != from.end(); ++eit)
    {
      const IntegerData& values(eit->integerProperties());
      IntegerData::const_iterator valueIt = values.find(intPropertyNamesToReduce[i]);
      if (valueIt != values.end())
      {
        imerged.insert(valueIt->second.begin(), valueIt->second.end());
      }
    }
    // Add values already present on the target:
    const IntegerData& toVals(to.integerProperties());
    IntegerData::const_iterator toValIt = toVals.find(intPropertyNamesToReduce[i]);
    if (toValIt != toVals.end())
    {
      imerged.insert(toValIt->second.begin(), toValIt->second.end());
    }
    if (!imerged.empty())
    {
      to.setIntegerProperty(
        intPropertyNamesToReduce[i], IntegerList(imerged.begin(), imerged.end()));
    }
  }

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Merge rule for strings: choose the first one, and only if not already present:
  StringList svalue;
  for (unsigned int ii = 0; ii < nps; ++ii)
  {
    bool haveString = false;
    EntityRefs::const_iterator eit;
    const StringData* toVals = to.hasStringProperties() ? &to.stringProperties() : nullptr;
    if (!toVals || toVals->find(stringPropertyNamesToReduce[ii]) == toVals->end())
    {
      for (eit = from.begin(); eit != from.end(); ++eit)
      {
        const StringData& values(eit->stringProperties());
        StringData::const_iterator valueIt = values.find(stringPropertyNamesToReduce[ii]);
        if (valueIt != values.end())
        {
          haveString = true;
          svalue.insert(svalue.end(), valueIt->second.begin(), valueIt->second.end());
          break;
        }
      }
      if (haveString)
      {
        to.setStringProperty(
          stringPropertyNamesToReduce[ii], StringList(svalue.begin(), svalue.end()));
      }
    }
  }

  // TODO: It should be possible to use different rules on a case-by-case basis:
  // Merge rule for floats: choose the first one, and only if not already present:
  FloatList fvalue;
  for (unsigned int ii = 0; ii < npf; ++ii)
  {
    bool haveFloat = false;
    EntityRefs::const_iterator eit;
    const FloatData* toVals = to.hasFloatProperties() ? &to.floatProperties() : nullptr;
    if (!toVals || toVals->find(floatPropertyNamesToReduce[ii]) == toVals->end())
    {
      std::cout << to.name() << " needs " << floatPropertyNamesToReduce[ii] << "\n";
      for (eit = from.begin(); eit != from.end(); ++eit)
      {
        const FloatData& values(eit->floatProperties());
        FloatData::const_iterator valueIt = values.find(floatPropertyNamesToReduce[ii]);
        if (valueIt != values.end())
        {
          haveFloat = true;
          fvalue.insert(fvalue.end(), valueIt->second.begin(), valueIt->second.end());
          break;
        }
      }
      if (haveFloat)
      {
        to.setFloatProperty(
          floatPropertyNamesToReduce[ii], FloatList(fvalue.begin(), fvalue.end()));
      }
    }
  }
  return true;
}

/**\brief Return a filename extension (including ".") appropriate for saving \a model.
  *
  * Subclasses of session should override this method.
  * If a method returns the empty string, it indicates that the model
  * requires no "native" model file. (The polygon session is an example
  * of this where all information is saved in the SMTK file.)
  */
std::string Session::defaultFileExtension(const Model& model) const
{
  (void)model;
  return ".native";
}

/// Subclasses implement this; it should add a record for \a entRef to the resource.
EntityPtr Session::addEntityRecord(const EntityRef& entRef)
{
  (void)entRef;
  return NULL;
}

/**\brief Subclasses implement this; it should return a new ArrangementHelper subclass instance.
  *
  * The caller is responsible for deleting it.
  */
ArrangementHelper* Session::createArrangementHelper()
{
  return new ArrangementHelper;
}

/**\brief Recursively called by transcribeInternal until no new entities are encountered.
  *
  */
int Session::findOrAddRelatedEntities(
  const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper)
{
  if (helper->isMarked(entRef))
    return 0;
  helper->mark(entRef, true);

  EntityTypeBits entType = static_cast<EntityTypeBits>(entRef.entityFlags() & ENTITY_MASK);
  // Ignore bits restricting group membership:
  if (entType & GROUP_ENTITY)
    entType = GROUP_ENTITY;

  int numAdded = 0;
  switch (entType)
  {
    case CELL_ENTITY:
      numAdded += this->findOrAddCellAdjacencies(entRef.as<CellEntity>(), flags, helper);
      numAdded += this->findOrAddCellUses(entRef.as<CellEntity>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    case USE_ENTITY:
      numAdded += this->findOrAddOwningCell(entRef.as<UseEntity>(), flags, helper);
      numAdded += this->findOrAddShellAdjacencies(entRef.as<UseEntity>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    case SHELL_ENTITY:
      numAdded += this->findOrAddUseAdjacencies(entRef.as<ShellEntity>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    case GROUP_ENTITY:
      numAdded += this->findOrAddGroupOwner(entRef.as<Group>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    case MODEL_ENTITY:
      numAdded += this->findOrAddFreeCells(entRef.as<Model>(), flags, helper);
      numAdded += this->findOrAddRelatedModels(entRef.as<Model>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    case INSTANCE_ENTITY:
      numAdded += this->findOrAddPrototype(entRef.as<Instance>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      break;
    case SESSION:
      numAdded += this->findOrAddRelatedModels(entRef.as<SessionRef>(), flags, helper);
      numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
      numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
      break;
    default:
      smtkInfoMacro(
        this->log(), "Unknown entity type " << entRef.entityFlags() << " being transcribed.");
      break;
  }

  helper->reset(
    entRef); // Remove all *generated* (not user-specified) arrangements, properties, etc.
  return numAdded;
}

/**\brief Subclasses implement this; it should add boundary, bounding, embedded, and embeddor cells of the current cell.
  *
  */
int Session::findOrAddCellAdjacencies(
  const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add use records of the current cell.
  *
  */
int Session::findOrAddCellUses(
  const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add the current use's owning cell.
  *
  */
int Session::findOrAddOwningCell(
  const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add shells bounded by or bounding the given use.
  *
  */
int Session::findOrAddShellAdjacencies(
  const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddUseAdjacencies(
  const ShellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddGroupOwner(
  const Group& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddFreeCells(
  const Model& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedModels(
  const Model& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddPrototype(
  const Instance& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedModels(
  const SessionRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedGroups(
  const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedInstances(
  const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this to finalize arrangement information for \a entRef using the \a helper.
  *
  */
SessionInfoBits Session::findOrAddArrangements(
  const EntityRef& entRef, EntityPtr entRec, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)entRec;
  (void)flags;
  (void)helper;
  return SESSION_ARRANGEMENTS;
}

/**\brief Subclasses implement this to update transcribed (not user-specified) properties of \a entRef.
  *
  */
SessionInfoBits Session::updateProperties(
  const EntityRef& entRef, EntityPtr entRec, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)entRec;
  (void)flags;
  (void)helper;
  return SESSION_FLOAT_PROPERTIES | SESSION_STRING_PROPERTIES | SESSION_INTEGER_PROPERTIES;
}

/**\brief Sublasses implement this to update the tessellation of the given \a entRef.
  *
  * This method will only be called when transcribe() is asked to include the tessellation.
  */
SessionInfoBits Session::updateTessellation(
  const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)flags;
  (void)helper;
  return SESSION_TESSELLATION;
}

/**\brief Subclasses may override this method to export additional state.
  *
  * Importers (e.g., LoadJSON) and exporters (e.g., SaveJSON) will
  * call this method to obtain a session I/O class instance specific to
  * the given \a format. If a valid SessionIO shared-pointer is
  * returned, it will be dynamically cast to a format-specific subclass
  * and given the opportunity to provide additional information to be
  * imported/exported to/from the session.
  *
  * This default implementation is provided since most sessions will
  * not need additional state.
  */
SessionIOPtr Session::createIODelegate(const std::string& format)
{
  if (format == "json")
  {
    return SessionIOJSON::create();
  }

  return SessionIOPtr();
}

} // namespace model
} // namespace smtk
