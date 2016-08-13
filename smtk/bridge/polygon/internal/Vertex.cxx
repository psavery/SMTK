//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/internal/Vertex.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/Session.h"

#include "smtk/io/Logger.h"

#include "boost/polygon/polygon.hpp"

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

/**\brief Return true if the edge can be inserted.
  *
  * FIXME: This will have precision issues with small angles because
  *        sqrt() is not calculated with intervals and overflow may
  *        occur when edge neighbor-points are far from each other.
  */
bool vertex::canInsertEdge(const Point& neighborhood, incident_edges::iterator* where)
{
  // Early termination... 0 or 1 existing vertices are always in CCW order no
  // matter where we insert
  if (this->m_edges.size() < 2)
    {
    // A vertex with 1 incident edge that is part of a face has face completely
    // surrounding the vertex; it will never be valid to insert another edge
    // there without removing the face first.
    if (!this->m_edges.empty() && this->m_edges.front().m_adjacentFace)
      return false;
    // Otherwise, it is always valid to insert a new edge anywhere.
    if (where)
      *where = this->m_edges.begin();
    return true;
    }

  pmodel* model = this->parentAs<pmodel>();
  Point pt(
    neighborhood.x() - this->m_coords.x(),
    neighborhood.y() - this->m_coords.y()
  );

  Point prevPt(
    model->edgeTestPoint(
      this->m_edges.back().m_edgeId, !this->m_edges.back().m_edgeOut));
  Point pa(
    prevPt.x() - this->m_coords.x(),
    prevPt.y() - this->m_coords.y()
  );
  incident_edges::iterator it;
  for (it = this->m_edges.begin(); it != this->m_edges.end(); ++it)
    {
    Point currPt = model->edgeTestPoint(it->m_edgeId, !it->m_edgeOut);
    Point pb(
      currPt.x() - this->m_coords.x(),
      currPt.y() - this->m_coords.y()
    );

    // Test whether "t" is in the CCW range between "a" and "b":
    // (Done using signs of cross-products to check angle relationships.)
    HighPrecisionCoord axb = HighPrecisionCoord(pa.x()) * pb.y() - HighPrecisionCoord(pb.x()) * pa.y();
    HighPrecisionCoord axt = HighPrecisionCoord(pa.x()) * pt.y() - HighPrecisionCoord(pt.x()) * pa.y();
    HighPrecisionCoord txb = HighPrecisionCoord(pt.x()) * pb.y() - HighPrecisionCoord(pb.x()) * pt.y();
    bool inside =
      (axb > 0 && axt > 0 && txb > 0) ||    // A->B < 180 degrees => A->T and T->B also < 180 degrees
      (axb < 0 && !(axt < 0 && txb < 0)) || // A->B > 180 degrees => if B->T and T->A < 180 degrees, T outside A->B
      (axb == 0 && axt < 0 && txb < 0);     // A->B = 180 degrees => A->T and T->B also < 180 degrees

    if (inside)
      {
      if (!it->m_adjacentFace)
        { // There is no face; it's OK to add the edge here.
        if (where)
          *where = it;
        return true;
        }
      else
        {
        smtkErrorMacro(model->session()->log(),
          "Edge would split face " << it->m_adjacentFace);
        return false;
        }
      }
    pa = pb;
    }
  smtkErrorMacro(model->session()->log(), "Collinear edges");
  return false;
}

/**\brief Insert the edge \a where told (by canInsertEdge).
  *
  * \a edgeOutwards indicates whether the forward-direction edge
  * is outward or inward-pointing (from/to this vertex).
  */
void vertex::insertEdgeAt(incident_edges::iterator where, const Id& edgeId, bool edgeOutwards)
{
  incident_edge_data edgeData;
  edgeData.m_edgeId = edgeId;
  edgeData.m_edgeOut = edgeOutwards;
  // NB: Should never insert edge where a face exists, so edgeData.m_adjacentFace should be left NULL.
  this->m_edges.insert(where, edgeData);
}

/**\brief Remove the edge incidence record at the given position.
  *
  * This does not perform any updates to other incidence records.
  */
void vertex::removeEdgeAt(incident_edges::iterator where)
{
  this->m_edges.erase(where);
}

bool vertex::setFaceAdjacency(const Id& incidentEdge, const Id& adjacentFace, bool isCCW, int edgeDir)
{
  incident_edges::iterator it;
  for (it = this->m_edges.begin(); it != this->m_edges.end(); ++it)
    {
    // This conditional is complex because we must handle the case when
    // an edge has both endpoints into the same vertex:
    if (
      it->edgeId() == incidentEdge && (                          // The edge ID matches and either:
        edgeDir == 0 ||                                          // we don't care about edge direction or
        (edgeDir != 0 && (edgeDir > 0) == it->isEdgeOutgoing())  // the edge direction also matches (i.e., head at vertex)
      ))
      {
      if (isCCW)
        {
        ++it;
        if (it == this->edgesEnd())
          {
          it = this->edgesBegin();
          }
        it->m_adjacentFace = adjacentFace;
        }
      else
        {
        it->m_adjacentFace = adjacentFace;
        }
      this->dump();
      return true;
      }
    }
  this->dump();
  return false;
}

void vertex::dump()
{
  std::cout << "    Vertex " << this->id() << "   (" << this->point().x() << " " << this->point().y() << ")" << "\n";
  for (incident_edges::iterator it = this->edgesBegin(); it != edgesEnd(); ++it)
    {
    std::cout
      << "      e: " << it->edgeId() << " " << (it->isEdgeOutgoing() ? "O" : "I")
      << "      f: " << it->clockwiseFaceId() << "\n";
    }
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
