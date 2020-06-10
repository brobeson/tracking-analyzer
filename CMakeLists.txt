cmake_minimum_required(VERSION 3.12)

project(
  tracking-analyzer
  DESCRIPTION "A tool for real-time analysis of visual tracking results."
  HOMEPAGE_URL "https://github.com/brobeson/tracking-analyzer"
  LANGUAGES CXX
)

# include(cmake/compiler.cmake)

add_subdirectory(tracking-analyzer)
return()

option(fits_enable_tests "Enable or disable building FITS tests." on)
if(fits_enable_tests)
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
    add_subdirectory(catch2)
    add_subdirectory(unit_tests)
    # add_subdirectory(compilation_tests)
  endif()
endif()