add_subdirectory(printer)

add_executable(llvm-ir-printer${EXE_SUFFIX}
  main.cpp
)

target_define_file_basename(llvm-ir-printer${EXE_SUFFIX})
target_project_compile_options(llvm-ir-printer${EXE_SUFFIX})
target_project_compile_definitions(llvm-ir-printer${EXE_SUFFIX}
  PRIVATE_DEFS
    LOG_LEVEL=${LOG_LEVEL}
)

target_include_directories(llvm-ir-printer${EXE_SUFFIX}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(llvm-ir-printer${EXE_SUFFIX}
  llvm-ir-print
  #${libedit_LIBRARIES}
)

install(
  TARGETS llvm-ir-printer${EXE_SUFFIX}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)
