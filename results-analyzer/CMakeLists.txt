find_package(Qt5 5.9 REQUIRED COMPONENTS Charts Widgets) #Core Gui Widgets)
add_executable(
  results-analyzer
  gui/main_window.cpp
  gui/main_window.h
  gui/main_window.ui
  main.cpp
)
target_link_libraries(
  results-analyzer
  PRIVATE
    analyzer::analyzer
    Qt5::Charts
    Qt5::Widgets
)
# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
#   target_link_libraries(results-analyzer PRIVATE stdc++fs)
# endif()
# target_include_directories(results-analyzer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(results-analyzer PRIVATE ${compiler_options})
target_compile_features(results-analyzer PRIVATE cxx_std_17)
set_target_properties(
  results-analyzer
  PROPERTIES
    AUTOUIC true
    AUTOMOC true
#     CXX_CLANG_TIDY
#       "clang-tidy;-p;${CMAKE_BINARY_DIR};--extra-arg=-Wno-unknown-warning-option"
)