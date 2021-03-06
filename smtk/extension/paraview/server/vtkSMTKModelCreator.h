//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKModelCreator_h
#define smtk_extension_paraview_server_vtkSMTKModelCreator_h

#include "smtk/extension/paraview/server/Exports.h"

#include "vtkSMTKResourceSource.h"

class vtkModelMultiBlockSource;
class vtkSMTKWrapper;

/**\brief A class for SMTK-based model sources.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKModelCreator : public vtkSMTKResourceSource
{
public:
  vtkTypeMacro(vtkSMTKModelCreator, vtkSMTKResourceSource);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKModelCreator* New();

  /// Set/get the create operation type name.
  vtkGetStringMacro(TypeName);
  vtkSetStringMacro(TypeName);

  /// Set/get the json-formatted input specification for the create operation.
  vtkGetStringMacro(Parameters);
  vtkSetStringMacro(Parameters);

  /// Return the VTK algorithm used to read the SMTK file.
  vtkModelMultiBlockSource* GetModelSource() { return this->ModelSource.GetPointer(); }

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GetResource() const override;

  /// Return the SMTK model resource that holds data read from \a FileName.
  smtk::model::ResourcePtr GetSMTKResource() const;

protected:
  vtkSMTKModelCreator();
  ~vtkSMTKModelCreator() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool CreateModel();

  char* TypeName;
  char* Parameters;
  vtkNew<vtkModelMultiBlockSource> ModelSource;

private:
  vtkSMTKModelCreator(const vtkSMTKModelCreator&) = delete;
  void operator=(const vtkSMTKModelCreator&) = delete;
};

#endif
