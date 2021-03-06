//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelEdgeMeshOperation - Set the model edge mesh parameters.
// .SECTION Description
// Operation to change meshing parameters and initiate meshing on
// the server of a model edge.

#ifndef __vtkCMBModelEdgeMeshOperation_h
#define __vtkCMBModelEdgeMeshOperation_h

#include "cmbSystemConfig.h"
#include <vtkObject.h>

class vtkCMBMeshWrapper;

class VTK_EXPORT vtkCMBModelEdgeMeshOperation : public vtkObject
{
public:
  static vtkCMBModelEdgeMeshOperation* New();
  vtkTypeMacro(vtkCMBModelEdgeMeshOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkCMBMeshWrapper* meshWrapper);

  // Description:
  // Set/get the model edge's unique persistent Id.
  vtkSetMacro(Id, vtkIdType);
  vtkGetMacro(Id, vtkIdType);

  // Description:
  // Set/get the length for the cells discretizing the model edge.
  vtkSetClampMacro(Length, double, 0, VTK_FLOAT_MAX);
  vtkGetMacro(Length, double);

  // Description:
  // Set/get whether or not to mesh this model edge.
  // 0 means do not and 1 means do.
  vtkSetClampMacro(BuildModelEntityMesh, int, 0, 1);
  vtkGetMacro(BuildModelEntityMesh, int);

  // Description:
  // Set/get whether or not to mesh higher dimensional entities.
  // 0 means do not and 1 means do.
  vtkSetClampMacro(MeshHigherDimensionalEntities, int, 0, 1);
  vtkGetMacro(MeshHigherDimensionalEntities, int);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelEdgeMeshOperation();
  virtual ~vtkCMBModelEdgeMeshOperation();

private:
  vtkCMBModelEdgeMeshOperation(const vtkCMBModelEdgeMeshOperation&); // Not implemented.
  void operator=(const vtkCMBModelEdgeMeshOperation&);               // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkIdType Id;
  double Length;
  int BuildModelEntityMesh;
  int MeshHigherDimensionalEntities;
};

#endif
