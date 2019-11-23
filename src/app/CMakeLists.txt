include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${CGAL_LIBRARY} ${Boost_LIBRARIES})

add_executable(wtt_add_noise add_noise_to_mesh.cpp)
list(APPEND apps wtt_add_noise)

add_executable(wtt_filter wavelet_filter.cpp)
list(APPEND apps wtt_filter)

add_executable(wtt_fwt wavelet_analyze.cpp)
list(APPEND apps wtt_fwt)

add_executable(wtt_iwt wavelet_synthesize.cpp)
list(APPEND apps wtt_iwt)

add_executable(wtt_sort_mesh sort_mesh.cpp)

add_executable(wtt_l2_error l2_error.cpp)

install(TARGETS ${apps}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
