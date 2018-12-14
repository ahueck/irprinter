add_library(irprint
  IRNodeFinder.cpp
  LLVMTool.cpp
)

target_define_file_basename(irprint)
target_project_compile_options(irprint)
target_project_compile_definitions(irprint
  PRIVATE_DEFS
    LOG_LEVEL=${LOG_LEVEL}
)

target_include_directories(irprint
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(irprint 
  SYSTEM
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(irprint
  ${CLANG_EXPORTED_TARGETS}
)

install(
  TARGETS irprint
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)

install(
  FILES 
    ${PROJECT_SOURCE_DIR}/include/printer/IRNodeFinder.h
    ${PROJECT_SOURCE_DIR}/include/printer/LLVMTool.h
  DESTINATION include
)

# Hacky: Here, the install code is guaranteed to run last (after ${CMAKE_INSTALL_PREFIX}/lib folder was created)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CLANG_INSTALL_PREFIX}/lib/clang ${CMAKE_INSTALL_PREFIX}/lib/clang)")