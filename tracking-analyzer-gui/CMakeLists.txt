project(tracking-analyzer-gui LANGUAGES CXX)
add_executable(
  ${PROJECT_NAME}
  application.cpp
  application.h
  gui/main_window.cpp
  gui/main_window.h
  gui/main_window.ui
  main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    ${CMAKE_TOOLS_LINK_LIBRARIES}
    GSL
    Qt5::Charts
    Qt5::Widgets
    tracking::analyzer
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
  target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(${PROJECT_NAME} PRIVATE ${CMAKE_TOOLS_COMPILE_OPTIONS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOUIC true AUTOMOC true)
