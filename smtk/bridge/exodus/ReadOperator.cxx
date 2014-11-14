//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/exodus/ReadOperator.h"

#include "smtk/bridge/exodus/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"

#include "vtkExodusIIReader.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

bool ReadOperator::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult ReadOperator::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

  vtkNew<vtkExodusIIReader> rdr;
  rdr->SetFileName(filenameItem->value(0));
  rdr->UpdateInformation();
  // Turn on all side and node sets.
  // Turn off all element blocks?
  rdr->Update();
  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    vtkSmartPointer<vtkMultiBlockDataSet>::New();
  modelOut->ShallowCopy(
    vtkMultiBlockDataSet::SafeDownCast(
      rdr->GetOutputDataObject()));
  Bridge* brdg = this->exodusBridge();
  smtk::model::ModelEntity smtkModelOut =
    brdg->addModel(modelOut);

  // Now set model for bridge and transcribe everything.

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");
  resultModels->setValue(smtkModelOut);

  return result;
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/exodus/ReadOperator_xml.h"

smtkImplementsModelOperator(
  smtk::bridge::exodus::ReadOperator,
  exodus_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::exodus::Bridge);
