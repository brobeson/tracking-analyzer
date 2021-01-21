if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(
    compiler_options
    -pedantic-errors
    -Walloca
    -Wcast-qual
    -Wconversion
    -Wdate-time
    -Wduplicated-branches
    -Wduplicated-cond
    -Werror
    -Wfloat-equal
    -Wformat=2
    -Winvalid-pch
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnoexcept
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpedantic
    -Wplacement-new=2
    -Wredundant-decls
    -Wregister
    -Wshadow
    -Wsign-conversion
    -Wsign-promo
    -Wsubobject-linkage
    -Wswitch-default
    -Wswitch-enum
    -Wtrampolines
    -Wundef
    -Wunused
    -Wunused-macros
    -Wuseless-cast
    -Wzero-as-null-pointer-constant
    -Wall
    -Wextra
    "$<$<CONFIG:Debug>:-fstack-protector-strong>"
    "$<$<CONFIG:Debug>:-Wstack-protector>"
    "$<$<CONFIG:Debug>:-ggdb>"
  )
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
    list(
      APPEND
        compiler_options
        -fdiagnostics-show-template-tree
        -Wcast-align=strict
        -Wextra-semi
    )
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(
    compiler_options
    -fdiagnostics-show-template-tree
    -pedantic-errors
    -Weverything
    -Werror
    -Wno-c++98-c++11-c++14-compat
    -Wno-c++98-compat
    -Wno-c++11-compat
    -Wno-c++14-compat
    -Wno-documentation
    -Wno-documentation-unknown-command
    -Wno-padded
    -Wno-weak-vtables
    "$<$<CONFIG:Debug>:-fstack-protector-strong>"
    "$<$<CONFIG:Debug>:-Wstack-protector>"
    "$<$<CONFIG:Debug>:-ggdb>"
  )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(compiler_options /analyze /Wall /WX)
else()
  message(
    WARNING
      " Default compiler options are not enabled for your compiler.\n"
      " Detected CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}\n"
  )
endif()

# option(analyzer_CPPCHECK "Enable or disable running Cppcheck as part of the build." on)
# if(analyzer_CPPCHECK)
#   set(
#     CMAKE_CXX_CPPCHECK
#     cppcheck
#     --enable=all
#     --language=c++
#     --library=qt
#     --project=${CMAKE_BINARY_DIR}/compile_commands.json
#     --std=c++20
#     --quiet
#     --inline-suppr
#     -i ${CMAKE_BINARY_DIR}
#     -D Q_MOC_OUTPUT_REVISION=67
#   )
# endif()
