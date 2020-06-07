find_package(Qt5 REQUIRED COMPONENTS Charts Core Gui Widgets)
add_executable(
  ${PROJECT_NAME}
  gui/main_window.cpp
  gui/main_window.h
  main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Charts Qt5::Widgets)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOUIC true AUTOMOC true)