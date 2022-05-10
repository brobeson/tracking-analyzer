include(CMakePrintHelpers)

function(cmake_tools_make_target_dependency_graphs)
  cmake_parse_arguments(
    ct
    "VERBOSE"
    "NAMESPACE"
    ""
    ${ARGN}
  )
  if(NOT ct_SOURCE_DIRECTORY)
    set(ct_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}")
  endif()
  message(CHECK_START "Building target dependency graphs")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")
  _get_filtered_targets()
  cmake_print_variables(ct_TARGETS)
  _write_targets_to_json()
  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "done")
endfunction()


#===============================================================================
#                                                        Implementation Details
#===============================================================================
# Get the list of project targets allowed in the dependency graph.
# Return the list of targets in ct_TARGETS in the parent scope.
function(_get_filtered_targets)
  _log(CHECK_START "Searching for targets in ${CMAKE_SOURCE_DIR}")
  _get_defined_targets(ct_TARGETS "${ct_SOURCE_DIRECTORY}")
  list(LENGTH ct_TARGETS length)
  _log(STATUS "Found ${length} targets before filtering")
  _filter_targets()
  list(SORT ct_TARGETS)
  set(ct_TARGETS ${ct_TARGETS} PARENT_SCOPE)
  list(LENGTH ct_TARGETS length)
  _log(CHECK_PASS "Found ${length} targets")
endfunction()

# Get all the targets defined in a directory and its descendent directories.
# Since this is a recursive function, it uses the typical pattern of IO
# parameters instead of manipulating the parent scope.
# Parameters:
#   output: The variable to hold the list of found targets.
#   root_directory: The directory to search for targets. The function searches
#     descendent directories, too.
function(_get_defined_targets output root_directory)
  # Get targets defined in this directory, first.
  get_directory_property(targets DIRECTORY "${root_directory}" BUILDSYSTEM_TARGETS)
  list(LENGTH targets length)
  _log(STATUS "Found ${length} targets in ${root_directory}")

  # Search subdirectories.
  get_directory_property(subdirectories DIRECTORY "${root_directory}" SUBDIRECTORIES)
  foreach(subdirectory IN LISTS subdirectories)
    if(subdirectory MATCHES "${CMAKE_BINARY_DIR}/_deps/.*")
      _log(STATUS "Skipping ${subdirectory}")
    else()
      _get_defined_targets(subdirectory_targets "${subdirectory}")
      list(LENGTH subdirectory_targets length)
      if(length GREATER 0)
        list(APPEND targets ${subdirectory_targets})
      endif()
    endif()
  endforeach()

  # Hand it all back up the call stack.
  set(${output} ${targets} PARENT_SCOPE)
endfunction()

# Remove unwanted targets from the list of all build targets. This removes
# targets built in by CMake, such as CTest targets, and it removes any targets
# that match a list of regular expressions.
function(_filter_targets)
  _filter_ctest_targets()
  _filter_utility_targets()
  list(LENGTH ct_TARGETS length)
  _log(STATUS "Found ${length} targets after filtering CMake targets")
  if(ct_TARGET_EXCLUDES)
    _log(STATUS "Filtering by regular expressions: ${ct_TARGET_EXCLUDES}")
    _filter_by_regex(
      ct_TARGETS
      EXCLUDE_PATTERNS ${ct_TARGET_EXCLUDES}
      LIST ${ct_TARGETS}
    )
    list(LENGTH ct_TARGETS length)
    _log(STATUS "Found ${length} targets after filtering by regular expressions")
  endif()
  set(ct_TARGETS ${ct_TARGETS} PARENT_SCOPE)
endfunction()

# Filter CTest targets out of the list of build targets.
function(_filter_ctest_targets)
  foreach(type IN ITEMS "Experimental" "Nightly" "Continuous")
    foreach(stage IN ITEMS "" "MemoryCheck" "Start" "Update" "Configure" "Build" "Test" "Coverage" "MemCheck" "Submit")
      list(REMOVE_ITEM ct_TARGETS ${type}${stage})
    endforeach()
  endforeach()
  set(ct_TARGETS ${ct_TARGETS} PARENT_SCOPE)
endfunction()

# Remove CMake utility targets from the list of targets created by the project.
# These are targets created by commands like add_custom_target().
function(_filter_utility_targets)
  foreach(target IN LISTS ct_TARGETS)
    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "UTILITY")
      list(REMOVE_ITEM ct_TARGETS ${target})
    endif()
  endforeach()
  set(ct_TARGETS ${ct_TARGETS} PARENT_SCOPE)
endfunction()


#===============================================================================
#                                                             Utility Functions
#===============================================================================
# Write a message to the console if the user enabled `VERBOSE`.
# Parameters:
#   level: The level of the message. This must be a valid `mode` or `checkState`
#     value for the `message()` command.
#   ARGN: The values passed directly to the `message()` command for output.
function(_log level)
  if(NOT ct_VERBOSE)
    return()
  endif()
  if(level STREQUAL "CHECK_PASS" OR level STREQUAL "CHECK_FAIL")
    list(POP_BACK CMAKE_MESSAGE_INDENT)
  endif()
  message(${level} ${ARGN})
  if(level STREQUAL "CHECK_START")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
  endif()
  set(CMAKE_MESSAGE_INDENT ${CMAKE_MESSAGE_INDENT} PARENT_SCOPE)
endfunction()


#===============================================================================
#                                                         JSON Output Functions
#===============================================================================
function(_write_targets_to_json)
  set(targets_json "[")
  foreach(target IN LISTS ct_TARGETS)
    _write_target_to_json(${target})
    string(APPEND targets_json "${target_json}" ",")
  endforeach()
  # Remove the trailing comma, then append the closing ']' to create valid JSON.
  string(REGEX REPLACE ",$" "]" targets_json "${targets_json}")
  file(WRITE "${CMAKE_SOURCE_DIR}/targets.json" "${targets_json}")
endfunction()

function(_write_target_to_json target)
  set(target_json "{}")
  string(JSON target_json SET "${target_json}" "target" "\"${target}\"")
  get_target_property(target_type ${target} TYPE)
  string(JSON target_json SET "${target_json}" "type" "\"${target_type}\"")
  if(ct_NAMESPACE AND TARGET ${ct_NAMESPACE}::${target})
    get_target_property(aliased_target ${ct_NAMESPACE}::${target} ALIASED_TARGET)
    if(aliased_target)
      string(JSON target_json SET "${target_json}" "alias" "\"${ct_NAMESPACE}::${target}\"")
    endif()
  endif()
  get_target_property(dependencies ${target} LINK_LIBRARIES)
  if(dependencies)
    string(GENEX_STRIP "${dependencies}" dependencies)
    string(REPLACE ";" "\", \"" dependencies "${dependencies}")
    string(PREPEND dependencies "[\"")
    string(APPEND dependencies "\"]")
    string(JSON target_json SET "${target_json}" "dependencies" "${dependencies}")
  endif()
  set(target_json "${target_json}" PARENT_SCOPE)
endfunction()
