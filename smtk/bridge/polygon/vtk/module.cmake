set (__dependencies)
# Test for targets that might be required or
# might not exist.
foreach(target
    vtkRenderingFreeType
    vtkRenderingMatplotlib
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingVolume${VTK_RENDERING_BACKEND}
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()


vtk_module(vtkPolygonOperatorsExt
  DEPENDS
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkInteractionWidgets
  PRIVATE_DEPENDS
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_WRAP_HIERARCHY
)