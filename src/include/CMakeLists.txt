install(DIRECTORY ./ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/wtlib/version.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/wtlib/version.hpp"
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wtlib/version.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/wtlib)