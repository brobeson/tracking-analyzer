cmake_minimum_required(VERSION 3.12)

project(
  tracking-analyzer
  DESCRIPTION "A tool for real-time analysis of visual tracking results."
  HOMEPAGE_URL "https://github.com/brobeson/tracking-analyzer"
  LANGUAGES CXX
)

find_package(Qt5 5.9 REQUIRED COMPONENTS Charts Core Gui Widgets)
include(cmake/compiler.cmake)
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
