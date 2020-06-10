if(BUILD_TESTING)
  find_package(Qt5 REQUIRED COMPONENTS Test)

  function(add_analyzer_unit_test test_name)
    cmake_parse_arguments(analyzer "" "" "SOURCE_FILES;LINK_LIBRARIES" ${ARGN})
    add_executable(${test_name} ${analyzer_SOURCE_FILES})
    target_link_libraries(${test_name} PRIVATE Qt5::Test ${analyzer_LINK_LIBRARIES})
    target_include_directories(
      ${test_name}
      PRIVATE
        "${PROJECT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/tracking-analyzer"
    )
    set_target_properties(${test_name} PROPERTIES AUTOMOC on)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endfunction()

  add_analyzer_unit_test(
    bounding_box_test
    SOURCE_FILES
      bounding_box_test.cpp
      "${CMAKE_SOURCE_DIR}/tracking-analyzer/bounding_box.cpp"
      "${CMAKE_SOURCE_DIR}/tracking-analyzer/bounding_box.h"
  )
endif()
