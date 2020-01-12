cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_MODULE_PATH
   "${CMAKE_SOURCE_DIR}/cmake"
   ${CMAKE_MODULE_PATH})
set(CMAKE_VERBOSE_MAKEFILE true)

project(wtlib LANGUAGES CXX)

set(wtlib_VERSION_MAJOR 0)
set(wtlib_VERSION_MINOR 1)
set(wtlib_VERSION_PATCH 0)
set(wtlib_VERSION "${wtlib_VERSION_MAJOR}.${wtlib_VERSION_MINOR}.${wtlib_VERSION_PATCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_definitions("-frounding-math")
endif()

include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

# Find CGAL lib
find_package(CGAL REQUIRED
             COMPONENTS Core)
# Find Boost program_options lib
find_package(Boost REQUIRED
             COMPONENTS program_options)
# Resolve CGAL incompatibility issues.
include(CheckCGALAPI)

include_directories(
  src/include
  ${CGAL_INCLUDE_DIRS}
)

option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_COVERAGE "Enable code coverage tests" OFF)
option(ENABLE_CUSTOM_MESH "Enable custom mesh" ON)
option(ENABLE_TEST "Enable test" OFF)
option(ENABLE_UNORDERED_MAP "Enable unordered map" ON)
option(DISABLE_ROUNDING_CHECK "Disable rounding check" OFF)

if (ENABLE_COVERAGE)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_CXX_FLAGS_DEBUG "-fno-inline -fno-elide-constructors ${CMAKE_CXX_FLAGS_DEBUG}")
  set(ENABLE_TEST ON)
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  message(STATUS "Check code coverage: ON")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif ()

if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "release")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")

if (ENABLE_ASAN)
  message(STATUS "Address Sanitizer: ON")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
else ()
  message(STATUS "Address Sanitizer: OFF")
endif (ENABLE_ASAN)

if (ENABLE_CUSTOM_MESH)
  add_definitions(-DWTLIB_USE_CUSTOM_MESH)
  message(STATUS "Custom mesh: ON")
else ()
  remove_definitions(-DWTLIB_USE_CUSTOM_MESH)
  message(STATUS "Custom mesh: OFF")
endif()

if (DISABLE_ROUNDING_CHECK)
  add_definitions(-DCGAL_DISABLE_ROUNDING_MATH_CHECK)
  message(STATUS "Rounding math check: OFF")
else()
  message(STATUS "Rounding math check: ON")
endif()

if (ENABLE_UNORDERED_MAP)
  add_definitions(-DWTLIB_USE_UNORDERED_MAP)
  message(STATUS "Lookup table implementation: std::unordered_map")
else ()
  remove_definitions(-DWTLIB_USE_UNORDERED_MAP)
  message(STATUS "Lookup table implementation: std::map")
endif()

if (ENABLE_TEST)
  message(STATUS "Build tests: ON")
  add_subdirectory(src/test)
endif (ENABLE_TEST)

if (ENABLE_COVERAGE)
  set(COVERAGE_LCOV_EXCLUDES '${CMAKE_CURRENT_BINARY_DIR}/*')
  setup_target_for_coverage_lcov(
    NAME coverage
    EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/bin/wtt_run_tests
    EXECUTABLE_ARGS ${CMAKE_CURRENT_BINARY_DIR}/src ${CODE_COVERAGE_DEPENDENCY}
    LCOV_ARGS -q -rc lcov_branch_coverage=1 --no-external -d ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    GENHTML_ARGS  --legend --branch-coverage --no-prefix
    DEPENDENCIES ${CODE_COVERAGE_DEPENDENCY})
else()
  message(STATUS "Check code coverage: OFF")
endif()

add_subdirectory(src/include)
add_subdirectory(src/app)
add_subdirectory(bin)

# Install CMake config
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/wtlib-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/wtlib-config.cmake"
  @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/wtlib-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/wtlib
        )

# Install data files
install(FILES ${CMAKE_SOURCE_DIR}/data/sorted_subdivision_meshes/bunny.off
              ${CMAKE_SOURCE_DIR}/data/sorted_subdivision_meshes/dragon.off
              ${CMAKE_SOURCE_DIR}/data/sorted_subdivision_meshes/gargoyle-7.off
              ${CMAKE_SOURCE_DIR}/data/sorted_subdivision_meshes/torusknot-3.off
              ${CMAKE_SOURCE_DIR}/data/sorted_subdivision_meshes/vase-8.off
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/wtlib/sample-meshes)