cmake_minimum_required(VERSION 3.12)

project(
  tracking-analyzer
  DESCRIPTION "A tool for real-time analysis of visual tracking results."
  HOMEPAGE_URL "https://github.com/brobeson/tracking-analyzer"
  LANGUAGES CXX
)

# include(cmake/compiler.cmake)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(tracking-analyzer)

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
add_subdirectory(unit_tests)
