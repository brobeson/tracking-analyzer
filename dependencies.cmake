include(CMakePrintHelpers)

function(cmake_tools_make_target_dependency_graphs)
  cmake_parse_arguments(
    ct
    "NO_PLANTUML;VERBOSE"
    "NAMESPACE;OUTPUT_DIRECTORY;SOURCE_DIRECTORY"
    "DEPENDENCY_EXCLUDES;PLANTUML_ARGS;TARGET_EXCLUDES"
    ${ARGN}
  )
  if(NOT ct_SOURCE_DIRECTORY)
    set(ct_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}")
  endif()
  if(NOT ct_OUTPUT_DIRECTORY)
    set(ct_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dependency_graphs")
  endif()
  message(CHECK_START "Building target dependency graphs")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")
  _clean_output_directory()
  _get_filtered_targets()
  _get_filtered_dependencies()
  _write_targets_to_json("${ct_TARGETS};${ct_ALLOWED_DEPENDENCIES}" ct_JSON_DATABASE)
  file(WRITE "${CMAKE_SOURCE_DIR}/targets.json" "${ct_JSON_DATABASE}")
  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "done")
endfunction()


#===============================================================================
#                                                        Implementation Details
#===============================================================================
# Get the list of dependencies allowed in the dependency graph.
# Return the list of dependencies in ct_ALLOWED_DEPENDENCIES in the parent
# scope.
function(_get_filtered_dependencies)
  _log(CHECK_START "Filtering dependencies")
  foreach(target IN LISTS ct_TARGETS)
    get_target_property(dependencies ${target} LINK_LIBRARIES)
    list(APPEND all_dependencies ${dependencies})
  endforeach()
  list(REMOVE_DUPLICATES all_dependencies)
  # TODO This can probably be string(GENEX_STRIP)
  list(FILTER all_dependencies EXCLUDE REGEX "\\$<.*")
  _filter_by_regex(
    all_dependencies
    EXCLUDE_PATTERNS ${ct_DEPENDENCY_EXCLUDES}
    LIST ${all_dependencies}
  )
  list(SORT all_dependencies)
  set(ct_ALLOWED_DEPENDENCIES ${all_dependencies} PARENT_SCOPE)
  list(LENGTH all_dependencies length)
  _log(CHECK_PASS "Found ${length} dependencies")
endfunction()

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
# Remove all files from the graph output directory.
function(_clean_output_directory)
  file(GLOB old_files LIST_DIRECTORIES false "${ct_OUTPUT_DIRECTORY}/*")
  if(old_files)
    file(REMOVE ${old_files})
  endif()
endfunction()

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

# Remove elements from a list by regular expression.
# Parameters:
#   output: The variable to hold the filtered list to return to the calling
#     function.
#   EXCLUDE_PATTERNS pattern [pattern ...]
#     The list of regular expressions to use for filtering list elements. If an
#     element matches any of these patterns, the function removes it from the
#     list.
#   LIST item [item ...]
#     The list to filter.
function(_filter_by_regex output)
  cmake_parse_arguments(filter "" "" "EXCLUDE_PATTERNS;LIST" ${ARGN})
  foreach(regex IN LISTS filter_EXCLUDE_PATTERNS)
    list(FILTER filter_LIST EXCLUDE REGEX "${regex}")
  endforeach()
  set(${output} ${filter_LIST} PARENT_SCOPE)
endfunction()

#===============================================================================
#                                                         JSON Output Functions
#===============================================================================
function(_write_targets_to_json targets output)
  set(targets_json "[")
  foreach(target IN LISTS targets)
    _write_object_to_json(${target})
    string(APPEND targets_json "${object_json}" ",")
  endforeach()
  # Remove the trailing comma, then append the closing ']' to create valid JSON.
  string(REGEX REPLACE ",$" "]" targets_json "${targets_json}")
  set(${output} "${targets_json}" PARENT_SCOPE)
endfunction()

function(_write_object_to_json object)
  set(object_json "{}")
  string(JSON object_json SET "${object_json}" "id" "\"${object}\"")
  if(object MATCHES ".+::.+")
    string(REGEX MATCH "([^:]+)::([^:]+)" unused "${object}")
    string(JSON object_json SET "${object_json}" "namespace" "\"${CMAKE_MATCH_1}\"")
    string(JSON object_json SET "${object_json}" "name" "\"${CMAKE_MATCH_2}\"")
  else()
    string(JSON object_json SET "${object_json}" "name" "\"${object}\"")
  endif()
  if(TARGET ${object})
    _write_target_to_json(${object} "${object_json}")
  else()
    _write_library_to_json(${object} "${object_json}")
  endif()
  set(object_json "${object_json}" PARENT_SCOPE)
endfunction()

function(_write_target_to_json target target_json)
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
    foreach(dependency IN LISTS dependencies)
      if(NOT dependency IN_LIST ct_ALLOWED_DEPENDENCIES)
        list(REMOVE_ITEM dependencies ${dependency})
      endif()
    endforeach()
    string(REPLACE ";" "\", \"" dependencies "${dependencies}")
    string(PREPEND dependencies "[\"")
    string(APPEND dependencies "\"]")
    string(JSON target_json SET "${target_json}" "dependencies" "${dependencies}")
  endif()
  set(object_json "${target_json}" PARENT_SCOPE)
endfunction()

function(_write_library_to_json library target_json)
  set(dependency_json "{}")
  string(JSON dependency_json SET "${dependency_json}" "name" "\"${library}\"")
  # if(ct_NAMESPACE AND library ${ct_NAMESPACE}::${library})
  #   get_target_property(aliased_target ${ct_NAMESPACE}::${library} ALIASED_TARGET)
  #   if(aliased_target)
  #     string(JSON dependency_json SET "${dependency_json}" "alias" "\"${ct_NAMESPACE}::${library}\"")
  #   endif()
  # endif()
  # get_target_property(dependencies ${library} LINK_LIBRARIES)
  # if(dependencies)
  #   string(GENEX_STRIP "${dependencies}" dependencies)
  #   foreach(library IN LISTS dependencies)
  #     if(NOT library IN_LIST ct_ALLOWED_DEPENDENCIES)
  #       list(REMOVE_ITEM dependencies ${library})
  #     endif()
  #   endforeach()
  #   string(REPLACE ";" "\", \"" dependencies "${dependencies}")
  #   string(PREPEND dependencies "[\"")
  #   string(APPEND dependencies "\"]")
  #   string(JSON dependency_json SET "${dependency_json}" "dependencies" "${dependencies}")
  # endif()
  set(object_json "${dependency_json}" PARENT_SCOPE)
endfunction()
