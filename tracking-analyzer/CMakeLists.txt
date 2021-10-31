find_package(Qt5 5.9 REQUIRED COMPONENTS Charts Core Gui Widgets)
add_executable(
  ${PROJECT_NAME}
  bounding_box.cpp
  bounding_box.h
  dataset.cpp
  dataset.h
  filesystem.cpp
  filesystem.h
  gui/main_window.cpp
  gui/main_window.h
  gui/main_window.ui
  main.cpp
  tracking_results.h
  tracking_results.cpp
  training_metadata.h
  training_metadata.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Charts Qt5::Widgets)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
  target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(${PROJECT_NAME} PRIVATE ${compiler_options})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOUIC true AUTOMOC true)
