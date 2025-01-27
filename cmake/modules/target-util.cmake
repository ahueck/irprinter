function(target_project_compile_options target)
  cmake_parse_arguments(ARG "" "" "PRIVATE_FLAGS;PUBLIC_FLAGS" ${ARGN})

  target_compile_options(${target} PRIVATE
    -Wall -Wextra -pedantic
    -Wunreachable-code -Wwrite-strings
    -Wpointer-arith -Wcast-align
    -Wcast-qual -Wno-unused-parameter
    -fno-rtti
    )

  if (ARG_PRIVATE_FLAGS)
    target_compile_options(${target} PRIVATE
      "${ARG_PRIVATE_FLAGS}"
      )
  endif ()

  if (ARG_PUBLIC_FLAGS)
    target_compile_options(${target} PUBLIC
      "${ARG_PUBLIC_FLAGS}"
      )
  endif ()
endfunction()

function(target_project_compile_definitions target)
  cmake_parse_arguments(ARG "" "" "PRIVATE_DEFS;PUBLIC_DEFS" ${ARGN})

  if (ARG_PRIVATE_DEFS)
    target_compile_definitions(${target} PRIVATE
      "${ARG_PRIVATE_DEFS}"
      )
  endif ()

  if (ARG_PUBLIC_DEFS)
    target_compile_definitions(${target} PUBLIC
      "${ARG_PUBLIC_DEFS}"
      )
  endif ()
endfunction()
