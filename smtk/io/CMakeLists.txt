# set up sources to build
set(ioSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  Helpers.cxx
  SaveJSON.cxx
  json/jsonComponentSet.cxx
  json/jsonSelectionMap.cxx
  LoadJSON.cxx
  Logger.cxx
  ModelToMesh.cxx
  ResourceSetReader.cxx
  ResourceSetWriter.cxx
  XmlDocV1Parser.cxx
  XmlDocV2Parser.cxx
  XmlDocV3Parser.cxx
  XmlV2StringWriter.cxx
  XmlV3StringWriter.cxx
)

set(ioHeaders
  AttributeReader.h
  AttributeWriter.h
  Helpers.h
  json/jsonComponentSet.h
  json/jsonSelectionMap.h
  SaveJSON.h
  LoadJSON.h
  Logger.h
  ModelToMesh.h
  ResourceSetReader.h
  ResourceSetWriter.h
  #XmlDocV1Parser.h
  XmlDocV2Parser.h
  XmlStringWriter.h
  XmlV2StringWriter.h
  XmlV3StringWriter.h
)

# mesh-related I/O
list(APPEND ioSrcs
  ExportMesh.cxx
  ImportMesh.cxx
  ReadMesh.cxx
  WriteMesh.cxx
  mesh/Format.cxx
  mesh/MeshIOMoab.cxx
  mesh/MeshIOXMS.cxx
)

list(APPEND ioHeaders
  ExportMesh.h
  ImportMesh.h
  ReadMesh.h
  WriteMesh.h
  mesh/Format.h
  mesh/MeshIO.h
  mesh/MeshIOMoab.h
  mesh/MeshIOXMS.h
)

#install the headers
smtk_public_headers(${ioHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  target_link_libraries(TestBuild_smtk_io
    PUBLIC
    nlohmann_json)
endif()
