add_subdirectory(printer)

add_executable(irprinter${EXE_SUFFIX}
  main.cpp
)

target_define_file_basename(irprinter${EXE_SUFFIX})
target_project_compile_options(irprinter${EXE_SUFFIX})
target_project_compile_definitions(irprinter${EXE_SUFFIX}
  PRIVATE_DEFS
    LOG_LEVEL=${LOG_LEVEL}
)

target_include_directories(irprinter${EXE_SUFFIX}
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(irprinter${EXE_SUFFIX}
  irprint
  #${libedit_LIBRARIES}
)

install(
  TARGETS irprinter${EXE_SUFFIX}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)