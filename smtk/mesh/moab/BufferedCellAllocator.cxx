//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/mesh/moab/BufferedCellAllocator.h"
#include "smtk/mesh/moab/CellTypeToType.h"

#include "smtk/mesh/core/CellTypes.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/Interface.hpp"
#include "moab/ReadUtilIface.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>

namespace smtk
{
namespace mesh
{
namespace moab
{

BufferedCellAllocator::BufferedCellAllocator(::moab::Interface* interface)
  : smtk::mesh::BufferedCellAllocator()
  , Allocator(interface)
  , m_firstCoordinate(0)
  , m_nCoordinates(0)
  , m_coordinateMemory()
  , m_activeCellType(smtk::mesh::CellType_MAX)
  , m_nCoords(0)
  , m_localConnectivity()
  , m_cells()
{
}

BufferedCellAllocator::~BufferedCellAllocator()
{
  this->flush();
}

bool BufferedCellAllocator::reserveNumberOfCoordinates(std::size_t nCoordinates)
{
  // Can only reserve coordinates once
  if (m_nCoordinates != 0)
  {
    return false;
  }

  m_validState = this->allocatePoints(nCoordinates, m_firstCoordinate, m_coordinateMemory);

  if (m_validState)
  {
    m_nCoordinates = nCoordinates;
  }

  return m_validState;
}

bool BufferedCellAllocator::setCoordinate(std::size_t coord, double* xyz)
{
  if (!m_validState)
  {
    return false;
  }
  assert(coord < m_nCoordinates);

  m_coordinateMemory[0][coord] = xyz[0];
  m_coordinateMemory[1][coord] = xyz[1];
  m_coordinateMemory[2][coord] = xyz[2];

  return m_validState;
}

bool BufferedCellAllocator::flush()
{
  if (!m_validState)
  {
    return false;
  }

  if (m_localConnectivity.empty())
  {
    return true;
  }

  if (m_activeCellType == smtk::mesh::CellType_MAX)
  {
    return false;
  }

  if (m_activeCellType == smtk::mesh::Vertex)
  {
    // In the moab/interface world vertices don't have explicit connectivity
    // so we can't allocate cells. Instead we just explicitly add those
    // points to the cells range
    for (auto&& ptCoordinate : m_localConnectivity)
    {
      m_cells.insert(m_firstCoordinate + ptCoordinate);
    }

    m_localConnectivity.clear();

    return m_validState;
  }

  // only convert cells smtk mesh supports
  ::moab::Range cellsCreatedForThisType;

  // need to convert from smtk cell type to moab cell type
  ::moab::EntityHandle* startOfConnectivityArray = 0;

  m_validState = this->allocateCells(m_activeCellType, m_localConnectivity.size() / m_nCoords,
    m_nCoords, cellsCreatedForThisType, startOfConnectivityArray);

  if (m_validState)
  {
    // now that we have the chunk allocated need to fill it
    // we do this by iterating the cells
    for (std::size_t i = 0; i < m_localConnectivity.size(); ++i)
    {
      startOfConnectivityArray[i] = m_firstCoordinate + m_localConnectivity[i];
    }

    // notify database that we have written to connectivity, that way
    // it can properly update adjacencies and other database info
    this->connectivityModified(m_cells, m_nCoords, startOfConnectivityArray);

    // insert these cells back into the range
    m_cells.insert(cellsCreatedForThisType.begin(), cellsCreatedForThisType.end());
  }

  m_localConnectivity.clear();

  return m_validState;
}
}
}
}
