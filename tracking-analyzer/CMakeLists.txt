find_package(Qt5 REQUIRED COMPONENTS Charts Core Gui Widgets)
add_executable(
  ${PROJECT_NAME}
  bounding_box.cpp
  bounding_box.h
  gui/main_window.cpp
  gui/main_window.h
  main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Charts Qt5::Widgets)
target_compile_options(${PROJECT_NAME} PRIVATE ${compiler_options})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOUIC true AUTOMOC true)