//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_operators_ExportMesh_h
#define pybind_smtk_mesh_operators_ExportMesh_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/operators/ExportMesh.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::ExportMesh, smtk::operation::XMLOperation > pybind11_init_smtk_mesh_ExportMesh(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ExportMesh, smtk::operation::XMLOperation > instance(m, "ExportMesh");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::ExportMesh const &>())
    .def("deepcopy", (smtk::mesh::ExportMesh & (smtk::mesh::ExportMesh::*)(::smtk::mesh::ExportMesh const &)) &smtk::mesh::ExportMesh::operator=)
    .def("ableToOperate", &smtk::mesh::ExportMesh::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::mesh::ExportMesh> (*)()) &smtk::mesh::ExportMesh::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::ExportMesh> (*)(::std::shared_ptr<smtk::mesh::ExportMesh> &)) &smtk::mesh::ExportMesh::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::ExportMesh> (smtk::mesh::ExportMesh::*)() const) &smtk::mesh::ExportMesh::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::ExportMesh> (smtk::mesh::ExportMesh::*)()) &smtk::mesh::ExportMesh::shared_from_this)
    ;
  return instance;
}

#endif
