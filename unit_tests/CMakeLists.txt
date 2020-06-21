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
    target_compile_options(${test_name} PRIVATE ${compiler_options})
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    set_target_properties(${test_name} PROPERTIES AUTOMOC on)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endfunction()

  add_analyzer_unit_test(
    bounding_box_test
    SOURCE_FILES
      bounding_box_test.cpp
      "${CMAKE_SOURCE_DIR}/tracking-analyzer/bounding_box.cpp"
  )
  add_analyzer_unit_test(
    dataset_test
    SOURCE_FILES
      dataset_test.cpp
      "${CMAKE_SOURCE_DIR}/tracking-analyzer/dataset.cpp"
  )
  set_tests_properties(dataset_test PROPERTIES ENVIRONMENT "HOME=/home/user")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "test_dataset/Basketball"
      "test_dataset/Biker/img"
      "test_dataset/Bird1"
      "test_dataset/Bird2"
    COMMAND ${CMAKE_COMMAND} -E touch
      "test_dataset/Biker/img/0001.jpg"
      "test_dataset/Biker/img/0002.jpg"
      "test_dataset/Biker/img/0003.jpg"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()
