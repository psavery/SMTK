//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Read_h
#define pybind_smtk_bridge_cgm_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Read.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Read > pybind11_init_smtk_bridge_cgm_Read(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Read > instance(m, "Read", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Read const &>())
    .def("deepcopy", (smtk::bridge::cgm::Read & (smtk::bridge::cgm::Read::*)(::smtk::bridge::cgm::Read const &)) &smtk::bridge::cgm::Read::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Read> (*)()) &smtk::bridge::cgm::Read::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Read> (*)(::std::shared_ptr<smtk::bridge::cgm::Read> &)) &smtk::bridge::cgm::Read::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Read::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Read> (smtk::bridge::cgm::Read::*)() const) &smtk::bridge::cgm::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Read> (smtk::bridge::cgm::Read::*)()) &smtk::bridge::cgm::Read::shared_from_this)
    ;
  return instance;
}

#endif
