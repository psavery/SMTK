ADD_EXECUTABLE(DiscreteSerializationTest DiscreteSerializationTest.cxx)
TARGET_LINK_LIBRARIES(DiscreteSerializationTest smtkDiscreteSession vtkSMTKDiscreteModel)
add_test(NAME discreteModelSerializationTest COMMAND DiscreteSerializationTest)

ADD_EXECUTABLE(Discrete2DModelTopologyTest Discrete2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(Discrete2DModelTopologyTest smtkDiscreteSession vtkSMTKDiscreteModel)
add_test(NAME discreteModel2DModelTopologyTest COMMAND Discrete2DModelTopologyTest)
