//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Container_h
#define smtk_resource_Container_h

#include "smtk/resource/Metadata.h"
#include "smtk/resource/Resource.h"

#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace smtk
{
namespace resource
{

/// Tags used to access Container data.
struct IdTag
{
};
struct IndexTag
{
};
struct LocationTag
{
};
struct NameTag
{
};
struct AliasTag
{
};

/// Global access methods used to sort Container data.
namespace detail
{
inline const smtk::common::UUID& id(const ResourcePtr& r)
{
  return r->id();
}
inline smtk::resource::Resource::Index index(const ResourcePtr& r)
{
  return r->index();
}
inline const std::string& location(const ResourcePtr& r)
{
  return r->location();
}
}

using namespace boost::multi_index;

/// A multi-index container for accessing resources. This class is primarily
/// intended to be used in the implementation of smtk::resource::Manager only.
typedef boost::multi_index_container<
  ResourcePtr,
  indexed_by<ordered_unique<tag<IdTag>,
               global_fun<const ResourcePtr&, const smtk::common::UUID&, &detail::id> >,
    ordered_non_unique<tag<IndexTag>,
               global_fun<const ResourcePtr&, smtk::resource::Resource::Index, &detail::index> >,
    ordered_non_unique<tag<LocationTag>,
               global_fun<const ResourcePtr&, const std::string&, &detail::location> > > >
  Container;

/// A multi-index container for accessing resource metadata. This class is
/// primarily intended to be used in the implementation of
/// smtk::resource::Manager only.
typedef boost::multi_index_container<
  Metadata,
  indexed_by<
    ordered_unique<tag<NameTag>, const_mem_fun<Metadata, const std::string&, &Metadata::typeName> >,
    ordered_unique<tag<IndexTag>,
      const_mem_fun<Metadata, const smtk::resource::Resource::Index&, &Metadata::index> > > >
  MetadataContainer;
}
}

#endif // smtk_resource_Resources_h
