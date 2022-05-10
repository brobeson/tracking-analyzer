cmake_minimum_required(VERSION 3.12)

include(FetchContent)
FetchContent_Declare(
  cmake-tools
  GIT_REPOSITORY https://github.com/brobeson/cmake-tools.git
  GIT_TAG main
)
set(GSL_CXX_STANDARD 17)
FetchContent_Declare(
  gsl
  GIT_REPOSITORY https://github.com/microsoft/GSL
  GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(cmake-tools gsl)
list(APPEND CMAKE_MODULE_PATH "${cmake-tools_SOURCE_DIR}")

project(
  tracking-analyzer
  DESCRIPTION "A tool for real-time analysis of visual tracking results."
  HOMEPAGE_URL "https://github.com/brobeson/tracking-analyzer"
  LANGUAGES CXX
)

find_package(Qt5 5.9 REQUIRED COMPONENTS Charts Core Gui Widgets)
set(CMAKE_TOOLS_UB_SANITIZER on)
set(CMAKE_TOOLS_ADDRESS_SANITIZER on)
set(CMAKE_TOOLS_COVERAGE on)
include(CMakeToolsCompileOptions)
add_subdirectory(tracking-analyzer)
add_subdirectory(tracking-analyzer-gui)

string(
  CONCAT
    MEMORYCHECK_COMMAND_OPTIONS
    "--quiet"
    " --gen-suppressions=all"
    " --tool=memcheck"
    " --leak-check=yes"
    " --track-origins=yes"
    " --error-exitcode=1"
)
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(unit_tests)
endif()

include(dependencies.cmake)
cmake_tools_make_target_dependency_graphs(
  VERBOSE
  DEPENDENCY_EXCLUDES "asan;ubsan"
)
