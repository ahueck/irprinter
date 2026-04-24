CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(irprinter
  VERSION 0.4
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH 
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

include(ToolchainOptions)
include(CMakePackageConfigHelpers)

irprinter_add_format_target(format-sources
  "Formats project source files"
  TARGETS src/*.cpp
          include/*.h
)

add_subdirectory(src)
if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(test)
endif()