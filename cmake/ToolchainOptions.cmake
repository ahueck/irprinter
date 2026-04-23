include(FeatureSummary)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(LLVM CONFIG HINTS "${LLVM_DIR}")
if(NOT LLVM_FOUND)
  message(STATUS "LLVM not found at: ${LLVM_DIR}.")
  find_package(LLVM REQUIRED CONFIG)
endif()

set_package_properties(LLVM PROPERTIES
  URL https://llvm.org/
  TYPE REQUIRED
  PURPOSE
  "LLVM framework installation required to compile."
)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

find_package(Clang QUIET HINTS "${Clang_DIR}")

if(NOT Clang_FOUND)
  # Optional: only needed for CIR-enabled Clang packages:
  find_package(MLIR CONFIG QUIET HINTS "${MLIR_DIR}")
  find_package(Clang REQUIRED HINTS "${Clang_DIR}")
endif()

include(AddLLVM)
include(clang-tidy)
include(clang-format)
include(log-util)
include(target-util)

set(IRPRINTER_LOG_LEVEL 0 CACHE STRING "Granularity of the logger. 3 is most verbose, 0 is least.")

option(IRPRINTER_AUTO_RESOURCE_DIR "Try to automatically set the Clang resource directory" OFF)
option(IRPRINTER_ENABLE_COVERAGE "Enable LLVM-based coverage" OFF)

if(IRPRINTER_AUTO_RESOURCE_DIR AND NOT IRPRINTER_CLANG_RESOURCE_DIR)
  find_program(CLANG_EXECUTABLE NAMES clang-${LLVM_VERSION_MAJOR} clang)
  if(CLANG_EXECUTABLE)
    execute_process(
      COMMAND ${CLANG_EXECUTABLE} -print-resource-dir
      OUTPUT_VARIABLE CLANG_RESOURCE_DIR_VAR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(IRPRINTER_CLANG_RESOURCE_DIR ${CLANG_RESOURCE_DIR_VAR} CACHE STRING "Clang resource directory")
  endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  message(STATUS "Building as debug (default)")
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${irprinter_SOURCE_DIR}/install/irprinter" CACHE PATH "Default install path" FORCE)
  message(STATUS "Installing to (default): ${CMAKE_INSTALL_PREFIX}")
endif ()
