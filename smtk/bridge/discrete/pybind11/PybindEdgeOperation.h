//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_EdgeOperation_h
#define pybind_smtk_bridge_discrete_operators_EdgeOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/EdgeOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::EdgeOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_EdgeOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::EdgeOperation, smtk::operation::Operation > instance(m, "EdgeOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::EdgeOperation> (*)()) &smtk::bridge::discrete::EdgeOperation::create)
    .def("ableToOperate", &smtk::bridge::discrete::EdgeOperation::ableToOperate)
    ;
  return instance;
}

#endif
