//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/Manager.h"
#include "smtk/operation/ResourceManagerOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace operation
{

Manager::Manager()
  : m_observers()
  , m_resourceObserver(-1)
  , m_resourceMetadataObserver(-1)
{
}

Manager::~Manager()
{
}

bool Manager::registerOperation(Metadata&& metadata)
{
  auto alreadyRegisteredMetadata = m_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == m_metadata.get<IndexTag>().end())
  {
    auto inserted = m_metadata.get<IndexTag>().insert(metadata);
    if (inserted.second)
    {
      m_metadataObservers(*inserted.first, true);
      return true;
    }
  }

  return false;
}

bool Manager::unregisterOperation(const std::string& typeName)
{
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    m_metadata.get<NameTag>().erase(metadata);
    m_metadataObservers(*metadata, false);
    return true;
  }

  return false;
}

bool Manager::unregisterOperation(const Operation::Index& index)
{
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    m_metadata.get<IndexTag>().erase(metadata);
    return true;
  }

  return false;
}

std::shared_ptr<Operation> Manager::create(const std::string& typeName)
{
  std::shared_ptr<Operation> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    op = metadata->create();
    op->m_manager = shared_from_this();
    m_observers(op, smtk::operation::EventType::CREATED, nullptr);
  }

  return op;
}

std::shared_ptr<Operation> Manager::create(const Operation::Index& index)
{
  std::shared_ptr<Operation> op;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    // Create the resource with the appropriate UUID
    op = metadata->create();
    op->m_manager = shared_from_this();
    m_observers(op, smtk::operation::EventType::CREATED, nullptr);
  }

  return op;
}

Metadata::Observers::Key Manager::observeMetadata(MetadataObserver fn, bool invokeImmediately)
{
  auto key = this->metadataObservers().insert(fn);
  if (invokeImmediately)
  {
    for (auto& md : this->metadata())
    {
      fn(md, true);
    }
  }
  return key;
}

bool Manager::unobserveMetadata(Metadata::Observers::Key key)
{
  return this->metadataObservers().erase(key) != 0;
}

bool Manager::registerResourceManager(smtk::resource::ManagerPtr& resourceManager)
{
  // Only allow one resource manager to manage created resources.
  if (m_resourceObserver != -1)
  {
    this->observers().erase(m_resourceObserver);
  }

  // Use this resource manager to conduct resource manager-related operations
  // (e.g. SaveResource, LoadResource, CreateResource).
  if (m_resourceMetadataObserver != -1)
  {
    this->metadataObservers().erase(m_resourceMetadataObserver);
  }

  std::weak_ptr<smtk::resource::Manager> weakRMPtr = resourceManager;

  // Define a metadata observer that appends the assignment of the resource
  // manager to the create functor for operations that inherit from
  // ResourceManagerOperation.
  auto resourceMetadataObserver = [&, weakRMPtr](const smtk::operation::Metadata& md, bool adding) {
    if (!adding)
      return;
    auto rsrcManager = weakRMPtr.lock();
    if (!rsrcManager)
    {
      // The underlying resource manager has expired, so we can remove this
      // metadata observer.
      m_metadataObservers.erase(m_resourceMetadataObserver);
      m_resourceMetadataObserver = -1;
      return;
    }

    // We are only interested in operations that inherit from
    // ResourceManagerOperation.
    if (std::dynamic_pointer_cast<ResourceManagerOperation>(md.create()) == nullptr)
    {
      return;
    }

    // This metadata observer actually manipulates the metadata, so we need a
    // const cast. This is an exception to the rule of metadata observers.
    smtk::operation::Metadata& metadata = const_cast<smtk::operation::Metadata&>(md);

    auto create = metadata.create;
    metadata.create = [=]() {
      auto op = create();
      std::dynamic_pointer_cast<ResourceManagerOperation>(op)->setResourceManager(weakRMPtr);
      return op;
    };
  };

  // Add this metadata observer to the set of metadata observers, invoking it
  // immediately on all extant metadata.
  m_resourceMetadataObserver = this->observeMetadata(resourceMetadataObserver);

  // Define an observer that adds all created resources to the resource manager.
  m_resourceObserver =
    this->observers().insert([&, weakRMPtr](std::shared_ptr<smtk::operation::Operation>,
      smtk::operation::EventType event, smtk::operation::Operation::Result result) {
      auto rsrcManager = weakRMPtr.lock();
      if (!rsrcManager)
      {
        // The underlying resource manager has expired, so we can remove this
        // observer.
        m_observers.erase(m_resourceObserver);
        m_resourceObserver = -1;
        return 0;
      }

      // We are only interested in collecting resources post-operation
      if (event != smtk::operation::EventType::DID_OPERATE)
      {
        return 0;
      }

      // Gather all resource items
      std::vector<smtk::attribute::ResourceItemPtr> resourceItems;
      std::function<bool(smtk::attribute::ResourceItemPtr)> filter = [](
        smtk::attribute::ResourceItemPtr) { return true; };
      result->filterItems(resourceItems, filter);

      // For each resource item found...
      for (auto& resourceItem : resourceItems)
      {
        // ...for each resource in a resource item...
        for (std::size_t i = 0; i < resourceItem->numberOfValues(); i++)
        {
          // (no need to look at resources that cannot be resolved)
          if (!resourceItem->isValid() || resourceItem->value(i) == nullptr)
          {
            continue;
          }

          // ...add the resource to the manager.
          rsrcManager->add(resourceItem->value(i));
        }
      }
      return 0;
    });

  return m_resourceObserver != -1;
}

std::set<std::string> Manager::availableOperations() const
{
  std::set<std::string> availableOperations;
  for (auto& md : m_metadata)
  {
    availableOperations.insert(md.typeName());
  }
  return availableOperations;
}

std::set<Operation::Index> Manager::availableOperations(
  const smtk::resource::ComponentPtr& component) const
{
  std::set<Operation::Index> availableOperations;
  for (auto& md : m_metadata)
  {
    if (md.acceptsComponent(component))
    {
      availableOperations.insert(md.index());
    }
  }
  return availableOperations;
}

std::set<std::string> Manager::availableGroups() const
{
  std::set<std::string> available;
  for (auto& md : m_metadata)
  {
    std::set<std::string> operatorGroups = md.groups();
    available.insert(operatorGroups.begin(), operatorGroups.end());
  }
  return available;
}
}
}
