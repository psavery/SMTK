//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_Session_h
#define pybind_smtk_bridge_multiscale_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/Session.h"

#include "smtk/model/Session.h"
#include "smtk/bridge/mesh/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > pybind11_init_smtk_bridge_multiscale_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > instance(m, "Session");
  instance
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::Session> (smtk::bridge::multiscale::Session::*)()) &smtk::bridge::multiscale::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::Session> (smtk::bridge::multiscale::Session::*)() const) &smtk::bridge::multiscale::Session::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Session> (*)()) &smtk::bridge::multiscale::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Session> (*)(::std::shared_ptr<smtk::bridge::multiscale::Session> &)) &smtk::bridge::multiscale::Session::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::Session::name)
    .def_static("CastTo", [](const std::shared_ptr<smtk::model::Session> i) {
        return std::dynamic_pointer_cast<smtk::bridge::multiscale::Session>(i);
      })
    ;
  return instance;
}

#endif
