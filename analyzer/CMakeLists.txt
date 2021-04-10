find_package(Qt5 5.9 REQUIRED COMPONENTS Core)
add_library(analyzer STATIC file.cpp file.h)
add_library(analyzer::analyzer ALIAS analyzer)
target_link_libraries(analyzer PUBLIC Qt5::Core)
target_include_directories(analyzer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(analyzer PRIVATE ${compiler_options})
target_compile_features(analyzer PRIVATE cxx_std_17)
set_target_properties(
  analyzer
  PROPERTIES
    CXX_CLANG_TIDY
      "clang-tidy;-p;${CMAKE_BINARY_DIR};--extra-arg=-Wno-unknown-warning-option"
)