//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/operators/PartitionBoundaries.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/multiscale/PartitionBoundaries_xml.h"
#include "smtk/bridge/multiscale/Resource.h"
#include "smtk/bridge/multiscale/Session.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Vertex.h"

#include <cmath>

using namespace smtk::model;
using namespace smtk::common;

namespace
{
class Filter : public smtk::mesh::CellForEach
{
public:
  Filter()
    : smtk::mesh::CellForEach(true) // needs coordinates
  {
  }

  smtk::mesh::HandleRange validPoints;
};

class CoolingPlateFilter : public Filter
{
  double yvalue;
  double rvalue;
  double origin[3];
  bool lessThan;

public:
  CoolingPlateFilter(double yval, double rval, const double* o, bool less = true)
    : Filter()
    , yvalue(yval)
    , rvalue(rval)
    , lessThan(less)
  {
    for (int i = 0; i < 3; ++i)
    {
      this->origin[i] = o[i];
    }
  }

  void forCell(const smtk::mesh::Handle&, smtk::mesh::CellType, int numPts) override
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();
    for (int i = 0; i < numPts; ++i)
    {
      const double r =
        sqrt((coords[(i * 3)] - this->origin[0]) * (coords[(i * 3)] - this->origin[0]) +
          (coords[(i * 3) + 2] - this->origin[2]) * (coords[(i * 3) + 2] - this->origin[2]));
      const double currValue[2] = { coords[(i * 3) + 1], r };
      //add in a small tolerance
      if (currValue[0] >= (this->yvalue - 0.002) && currValue[0] <= (this->yvalue + 0.002))
      {
        if ((this->lessThan && (currValue[1] < this->rvalue)) ||
          ((!this->lessThan) && (currValue[1] >= this->rvalue)))
        {
          this->validPoints.insert(ptIds[i]);
        }
      }
    }
  }
};

class OuterEdgeFilter : public Filter
{
  double origin[3];
  double rmin;

public:
  OuterEdgeFilter(const double o[3], double r)
    : Filter()
    , rmin(r)
  {
    for (int i = 0; i < 3; i++)
    {
      this->origin[i] = o[i];
    }
  }

  void forCell(const smtk::mesh::Handle&, smtk::mesh::CellType, int numPts) override
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();

    for (int i = 0; i < numPts; ++i)
    {
      // reject any cells whose first coordinate is less than a distance <rmin>
      // from the axis of rotation
      const double r =
        sqrt((coords[(i * 3)] - this->origin[0]) * (coords[(i * 3)] - this->origin[0]) +
          (coords[(i * 3) + 2] - this->origin[2]) * (coords[(i * 3) + 2] - this->origin[2]));
      if (r > rmin)
      {
        this->validPoints.insert(ptIds[i]);
      }
    }
  }
};

bool labelIntersection(const smtk::mesh::CollectionPtr& collection,
  const smtk::mesh::MeshSet& shell, Filter& filter, smtk::model::EntityRefArray& created,
  smtk::bridge::mesh::Topology& topology)
{
  typedef smtk::bridge::mesh::Topology Topology;

  //need to remove the verts cells from the query for now
  //todo: filter needs to support vert cells
  smtk::mesh::CellSet shellCells = shell.cells();

  //extract the top cells
  smtk::mesh::for_each(shellCells, filter);
  smtk::mesh::CellSet filteredCells = smtk::mesh::CellSet(collection, filter.validPoints);

  //intersect the material and verts to find the verts of a given
  //material that passed the filter.
  //This verts than become a dirichlet set
  for (auto&& dom : collection->domains())
  {
    smtk::mesh::MeshSet domainMeshes = collection->meshes(dom);

    //find all cells on the top of shell that share a vert in common
    //with material volume
    smtk::mesh::CellSet domainCells = domainMeshes.cells();
    smtk::mesh::CellSet contactCells =
      smtk::mesh::point_intersect(domainCells, filteredCells, smtk::mesh::FullyContained);
    if (!contactCells.is_empty())
    {
      smtk::mesh::MeshSet contactD = collection->createMesh(contactCells);
      collection->setDirichletOnMeshes(contactD, smtk::mesh::Dirichlet(created.size()));

      // construct a new uuid
      smtk::common::UUID id = collection->modelResource()->unusedUUID();
      // construct a topology element for the vertex set (dimension 0)
      Topology::Element element(domainMeshes, 0);
      // insert the element into the topology under the parent level
      // (designating it as a "free" element)
      topology.m_elements.insert(std::make_pair(id, element));
      // store an entity ref associated with the vertex set id
      created.push_back(smtk::model::Vertex(collection->modelResource(), id));
    }
  }

  return true;
}
}

namespace smtk
{
namespace bridge
{
namespace multiscale
{

PartitionBoundaries::Result PartitionBoundaries::operateInternal()
{
  Result result;

  // Grab the datasets associated with the operator
  smtk::model::Models datasets = this->parameters()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
  {
    smtkErrorMacro(this->log(), "No models on which to partition boundaries.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::Model dataset = datasets[0];

  smtk::bridge::multiscale::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::multiscale::Resource>(dataset.component()->resource());
  smtk::bridge::multiscale::Session::Ptr session = resource->session();

  if (!session)
  {
    smtkErrorMacro(this->log(), "No session associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // The collection for this model has the same UUID as the model, so we can
  // access it using the model's UUID
  smtk::mesh::CollectionPtr collection =
    session->meshManager()->findCollection(dataset.entity())->second;

  if (!collection->isValid())
  {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::bridge::mesh::Topology* topology = session->topology(dataset);

  if (!topology)
  {
    smtkErrorMacro(this->log(), "No topology associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Set the origin from the values held in the parameters
  double origin[3];
  smtk::attribute::DoubleItemPtr originItem = this->parameters()->findDouble("origin");
  for (int i = 0; i < 3; i++)
  {
    origin[i] = originItem->value(i);
  }

  // Set the radius from the values held in the parameters
  double radius = this->parameters()->findDouble("radius")->value();

  //extract the exterior-shell for all meshes.
  smtk::mesh::MeshSet shell = collection->meshes().extractShell();

  // compute the shell's bounds
  std::array<double, 6> bounds = smtk::mesh::utility::extent(shell);

  // we're going to generate vertices, so we need to keep track of them
  smtk::model::EntityRefArray created;

  const double ymin = bounds[2];
  {
    CoolingPlateFilter filter(ymin, radius, origin);
    labelIntersection(collection, shell, filter, created, *topology);
  }

  const double ymax = bounds[3];
  {
    CoolingPlateFilter filter(ymax, radius, origin);
    labelIntersection(collection, shell, filter, created, *topology);
  }

  {
    OuterEdgeFilter filter(origin, radius * 2.);
    labelIntersection(collection, shell, filter, created, *topology);
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  createdItem->setIsEnabled(true);
  for (auto entity : created)
  {
    createdItem->appendValue(entity.component());
    session->declareDanglingEntity(entity);
  }

  smtk::attribute::ReferenceItem::Ptr modelItem = this->parameters()->associations();
  smtk::model::Model model = modelItem->valueAs<smtk::model::Entity>();
  model.addCells(created);

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  modifiedItem->appendValue(model.component());

  return result;
}

const char* PartitionBoundaries::xmlDescription() const
{
  return PartitionBoundaries_xml;
}
}
}
}
