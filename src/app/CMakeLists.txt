include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${CGAL_LIBRARY} ${Boost_LIBRARIES})

add_executable(wtl_add_noise_to_mesh add_noise_to_mesh.cpp)
list(APPEND apps wtl_add_noise_to_mesh)

add_executable(wtl_wavelet_filter wavelet_filter.cpp)
list(APPEND apps wtl_wavelet_filter)

add_executable(wtl_wavelet_analyze wavelet_analyze.cpp)
list(APPEND apps wtl_wavelet_analyze)

add_executable(wtl_wavelet_synthesize wavelet_synthesize.cpp)
list(APPEND apps wtl_wavelet_synthesize)

add_executable(wtl_sort_mesh sort_mesh.cpp)

add_executable(wtl_l2_error l2_error.cpp)

install(TARGETS ${apps}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
