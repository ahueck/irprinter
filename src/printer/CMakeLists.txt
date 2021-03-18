add_library(llvm-ir-print STATIC
  IRNodeFinder.cpp
  LLVMTool.cpp
  )

target_define_file_basename(llvm-ir-print)
target_project_compile_options(llvm-ir-print)
target_project_compile_definitions(llvm-ir-print
  PRIVATE_DEFS
    LOG_LEVEL=${LOG_LEVEL}
)

target_include_directories(llvm-ir-print
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(llvm-ir-print 
  SYSTEM
  PUBLIC
  ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(llvm-ir-print PUBLIC
  clangTooling
  clangCodeGen
  )

install(
  TARGETS llvm-ir-print
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
#install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CLANG_INSTALL_PREFIX}/lib/clang ${CMAKE_INSTALL_PREFIX}/lib/clang)")
