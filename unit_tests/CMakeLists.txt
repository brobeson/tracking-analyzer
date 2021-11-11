find_package(Qt5 REQUIRED COMPONENTS Test)

project(tracking_analyzer_unit_tests LANGUAGES CXX)

# To create a new unit test, add the test name to this list. Then write the
# test in the file <your_new_test>.cpp. If you need to set any test-specific
# properties, do so after the loop.
set(tests bounding_box_test dataset_test filesystem_test training_metadata_test)
foreach(test_name IN LISTS tests)
  add_executable(${test_name} ${test_name}.cpp)
  target_link_libraries(
    ${test_name}
    PRIVATE
      Qt5::Test
      tracking::analyzer
      $<$<CONFIG:Debug>:gcov>
      $<$<CONFIG:Debug>:ubsan>
  )
  target_compile_options(${test_name} PRIVATE ${compiler_options})
  target_compile_features(${test_name} PRIVATE cxx_std_20)
  set_target_properties(${test_name} PROPERTIES AUTOMOC on)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
set_tests_properties(filesystem_test PROPERTIES ENVIRONMENT "HOME=/home/user")
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
  COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/test_metadata" 
    "${PROJECT_BINARY_DIR}/test_metadata"
  COMMAND_ERROR_IS_FATAL ANY
)

# There are some clang-tidy checks that I want to disable in the unit test code,
# but keep in the library and application code. I'm unaware of a method for
# clang-tidy to combine configuration files, so I'm doing this manipulation
# here to generate a .clang-tidy file specific to the unit tests.
file(STRINGS "${CMAKE_SOURCE_DIR}/.clang-tidy" clang_tidy_configuration)
list(FIND clang_tidy_configuration "WarningsAsErrors: '*'" index)
# This subtraction puts the additional checks before the last check. This way, I
# don't need to worry about adding a comma to the last check in the list.
math(EXPR index "${index} - 1")
cmake_policy(SET CMP0121 NEW)
list(
  INSERT clang_tidy_configuration ${index}
  "  -cppcoreguidelines-avoid-magic-numbers,"
  "  -readability-convert-member-functions-to-static,"
)
list(
  PREPEND clang_tidy_configuration
  "# CMake configured this file. DON'T EDIT IT."
  "# Make changes in unit_tests/CMakeLists.txt."
)
# Ensures there is a blank line at the end of the file.
list(APPEND clang_tidy_configuration "")
string(REPLACE ";" "\n" clang_tidy_configuration "${clang_tidy_configuration}")
file(WRITE .clang-tidy ${clang_tidy_configuration})
