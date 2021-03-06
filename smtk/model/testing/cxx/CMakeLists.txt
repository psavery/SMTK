################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(smtkCoreModelTesting STATIC helpers.cxx)
smtk_public_headers(helpers.h)
target_link_libraries(smtkCoreModelTesting LINK_PRIVATE smtkCore)
smtk_install_library(smtkCoreModelTesting)

################################################################################
# Tools and demos
################################################################################

add_executable(demoReportArrangements demoReportArrangements.cxx)
target_link_libraries(demoReportArrangements smtkCore smtkCoreModelTesting)

################################################################################
# Tests
################################################################################
add_executable(unitManager unitManager.cxx)
target_link_libraries(unitManager smtkCore smtkCoreModelTesting)
add_test(NAME unitManager COMMAND unitManager)

add_executable(unitIterators unitIterators.cxx)
target_link_libraries(unitIterators smtkCore smtkCoreModelTesting)
add_test(NAME unitIterators COMMAND unitIterators)

add_executable(unitTessellation unitTessellation.cxx)
target_link_libraries(unitTessellation smtkCore smtkCoreModelTesting)
add_test(NAME unitTessellation COMMAND unitTessellation)

#add_executable(unitOperation unitOperation.cxx)
#smtk_operation_xml( "${CMAKE_CURRENT_SOURCE_DIR}/unitOutcomeOperation.sbt" unitOperationXML)
#target_include_directories(unitOperation PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
#target_link_libraries(unitOperation smtkCore smtkCoreModelTesting)
#if (SMTK_ENABLE_CGM_SESSION)
#  target_link_libraries(unitOperation smtkCGMSession)
#endif (SMTK_ENABLE_CGM_SESSION)
#add_test(NAME unitOperation COMMAND unitOperation)

add_executable(unitEntityRef unitEntityRef.cxx)
target_link_libraries(unitEntityRef smtkCore smtkCoreModelTesting)
add_test(NAME unitEntityRef COMMAND unitEntityRef)

#add_executable(unitDefaultSession unitDefaultSession.cxx)
#smtk_operation_xml( "${CMAKE_CURRENT_SOURCE_DIR}/unitForwardingOperation.sbt" unitOperationXML)
#target_include_directories(unitDefaultSession PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
#target_link_libraries(unitDefaultSession smtkCore smtkCoreModelTesting)
#add_test(NAME unitDefaultSession COMMAND unitDefaultSession)

add_executable(unitArrangement unitArrangement.cxx)
target_link_libraries(unitArrangement smtkCore)
add_test(NAME unitArrangement COMMAND unitArrangement)

add_executable(unitEntity unitEntity.cxx)
target_link_libraries(unitEntity smtkCore smtkCoreModelTesting)
add_test(NAME unitEntity COMMAND unitEntity)

add_executable(unitExportMeshOperation unitExportMeshOperation.cxx)
target_compile_definitions(unitExportMeshOperation PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(unitExportMeshOperation smtkCore smtkCoreModelTesting
  ${Boost_LIBRARIES})

add_executable(benchmarkModel benchmarkModel.cxx)
target_link_libraries(benchmarkModel smtkCore smtkCoreModelTesting)
#add_test(NAME benchmarkModel COMMAND benchmarkModel)

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR)
  add_test(NAME unitExportMeshOperation
    COMMAND $<TARGET_FILE:unitExportMeshOperation>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json")
endif()
