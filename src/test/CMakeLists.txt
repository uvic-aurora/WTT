find_package(Boost REQUIRED
  COMPONENTS filesystem
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES} ${CGAL_LIBRARY})


add_executable(ptq_classify_vertices_test
  ptq_classify_vertices_test.cpp
)
target_compile_definitions(ptq_classify_vertices_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/"
          IS_RUNNING_TESTS=1)


add_executable(ptq_subdivision_modifier_test
  ptq_subdivision_modifier_test.cpp
)
target_compile_definitions(ptq_subdivision_modifier_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")



add_executable(wavelet_mesh_ops_test
  wavelet_mesh_ops_test.cpp
)
target_compile_definitions(wavelet_mesh_ops_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")


add_executable(loop_math_utils_test
  loop_math_utils_test.cpp
)

add_executable(loop_analysis_ops_test
  loop_analysis_ops_test.cpp
)
target_link_libraries(loop_analysis_ops_test
                      ${Boost_LIBRARIES}
                      ${CGAL_LIBRARY})
target_compile_definitions(loop_analysis_ops_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")


add_executable(loop_synthesis_ops_test
  loop_synthesis_ops_test.cpp
)
target_compile_definitions(loop_synthesis_ops_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")


add_executable(butterfly_analysis_operations_test
  butterfly_analysis_operations_test.cpp
)
target_compile_definitions(butterfly_analysis_operations_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")



add_executable(butterfly_synthesis_operations_test
  butterfly_synthesis_operations_test.cpp
)
target_compile_definitions(butterfly_synthesis_operations_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/"
          IS_RUNNING_TESTS=1)


add_executable(wavelet_operations_test
  wavelet_operations_test.cpp
)
target_compile_definitions(wavelet_operations_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/"
          IS_RUNNING_TESTS=1)


add_executable(ptq_wavelet_transforms_test
  ptq_wavelet_transforms_test.cpp
)
target_compile_definitions(ptq_wavelet_transforms_test
  PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data/"
          IS_RUNNING_TESTS=1)

set(TEST_SUITES ptq_classify_vertices_test
                ptq_subdivision_modifier_test
                loop_math_utils_test
                loop_analysis_ops_test
                loop_synthesis_ops_test
                butterfly_analysis_operations_test
                butterfly_synthesis_operations_test
                wavelet_operations_test
                ptq_wavelet_transforms_test
                wavelet_mesh_ops_test
                ${TEST_SUITES})

set(CODE_COVERAGE_DEPENDENCY ${TEST_SUITES} PARENT_SCOPE)

add_custom_target(run_tests
  COMMAND ${CMAKE_SOURCE_DIR}/bin/wtt_run_tests ${CMAKE_BINARY_DIR}/src ${TEST_SUITES}
  DEPENDS ${TEST_SUITES})
  
