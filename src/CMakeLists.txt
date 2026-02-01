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

target_link_libraries(llvm-ir-printer${EXE_SUFFIX}
  LLVMLineEditor
  llvm-ir-print
)

install(
  TARGETS llvm-ir-printer${EXE_SUFFIX}
  RUNTIME DESTINATION bin
)
