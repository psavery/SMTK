//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_Session_h
#define pybind_smtk_bridge_polygon_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/Session.h"

#include "smtk/model/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::Session, smtk::model::Session > pybind11_init_smtk_bridge_polygon_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::polygon::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::Session> (smtk::bridge::polygon::Session::*)()) &smtk::bridge::polygon::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::Session> (smtk::bridge::polygon::Session::*)() const) &smtk::bridge::polygon::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Session> (*)()) &smtk::bridge::polygon::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::Session> (*)(::std::shared_ptr<smtk::bridge::polygon::Session> &)) &smtk::bridge::polygon::Session::create, py::arg("ref"))
    .def_static("staticClassName", &smtk::bridge::polygon::Session::staticClassName)
    .def("name", &smtk::bridge::polygon::Session::name)
    .def("allSupportedInformation", &smtk::bridge::polygon::Session::allSupportedInformation)
    ;
  return instance;
}

#endif
