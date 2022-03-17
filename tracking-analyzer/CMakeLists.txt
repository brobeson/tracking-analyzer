add_library(
  ${PROJECT_NAME}
  tracking-analyzer/bounding_box.cpp
  tracking-analyzer/bounding_box.h
  tracking-analyzer/dataset.cpp
  tracking-analyzer/dataset.h
  tracking-analyzer/filesystem.cpp
  tracking-analyzer/filesystem.h
  tracking-analyzer/tracking_results.h
  tracking-analyzer/tracking_results.cpp
  tracking-analyzer/training_metadata.h
  tracking-analyzer/training_metadata.cpp
  tracking-analyzer/training_samples.cpp
  tracking-analyzer/training_samples.h
)
add_library(tracking::analyzer ALIAS tracking-analyzer)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_link_libraries(${PROJECT_NAME} PUBLIC GSL PRIVATE Qt5::Core)
target_compile_options(${PROJECT_NAME} PRIVATE ${CMAKE_TOOLS_COMPILE_OPTIONS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
