set(LLVM_IR_PRINTER_BINARY $<TARGET_FILE:llvm-ir-printer${EXE_SUFFIX}>)

irprinter_find_llvm_progs(IRPRINTER_FILECHECK_EXEC "FileCheck-${LLVM_VERSION_MAJOR};FileCheck")

if(LLVM_EXTERNAL_LIT)
  cmake_path(GET LLVM_EXTERNAL_LIT PARENT_PATH LLVM_EXTERNAL_LIT_DIR)
endif()
irprinter_find_llvm_progs(IRPRINTER_LIT_EXEC
  "llvm-lit;lit;lit.py"
  HINTS ${LLVM_EXTERNAL_LIT_DIR} /usr/lib/llvm-${LLVM_VERSION_MAJOR} /usr/lib/llvm /usr/bin /usr/local/bin /opt/local/bin
  ABORT_IF_MISSING
)

add_custom_target(check-irprinter
  COMMAND ${IRPRINTER_LIT_EXEC} -vv ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS llvm-ir-printer${EXE_SUFFIX}
  COMMENT "Running irprinter regression tests"
  USES_TERMINAL
)

if(IRPRINTER_ENABLE_COVERAGE)
  set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage_report)
  irprinter_find_llvm_progs(IRPRINTER_PROFDATA_EXEC "llvm-profdata-${LLVM_VERSION_MAJOR};llvm-profdata")
  irprinter_find_llvm_progs(IRPRINTER_COV_EXEC "llvm-cov-${LLVM_VERSION_MAJOR};llvm-cov")
  
  set(IGNORE_REGEX ".*(test|build|build_cov|external).*")

  add_custom_target(coverage-irprinter
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_CURRENT_BINARY_DIR}/*.profraw" "${CMAKE_CURRENT_BINARY_DIR}/merged.profdata"
    # Execute tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target check-irprinter
    # Merge all profraw files
    COMMAND ${CMAKE_COMMAND} -E env bash -c "${IRPRINTER_PROFDATA_EXEC} merge -sparse ${CMAKE_CURRENT_BINARY_DIR}/*.profraw -o ${CMAKE_CURRENT_BINARY_DIR}/merged.profdata"
    # Generate HTML
    COMMAND ${IRPRINTER_COV_EXEC} show "${LLVM_IR_PRINTER_BINARY}"
            "-instr-profile=${CMAKE_CURRENT_BINARY_DIR}/merged.profdata"
            -format=html
            "-output-dir=${COVERAGE_DIR}"
            -show-line-counts-or-regions
            "-ignore-filename-regex=${IGNORE_REGEX}"
    # Show summary
    COMMAND ${IRPRINTER_COV_EXEC} report "${LLVM_IR_PRINTER_BINARY}"
            "-instr-profile=${CMAKE_CURRENT_BINARY_DIR}/merged.profdata"
            "-ignore-filename-regex=${IGNORE_REGEX}"
    COMMENT "Generating coverage report"
    DEPENDS check-irprinter
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )
endif()

configure_file(lit.site.cfg.py.in lit.site.cfg.py.tmp @ONLY)
file(GENERATE OUTPUT lit.site.cfg.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py.tmp)