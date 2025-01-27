function(add_format_target target comment)
  macro(filter_dir _dir_name_)
    foreach (SOURCE_FILE ${ALL_CXX_FILES})
      string(FIND ${SOURCE_FILE} ${_dir_name_} EXCLUDE_FOUND)
      if (NOT ${EXCLUDE_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_CXX_FILES ${SOURCE_FILE})
      endif()
    endforeach()
  endmacro()

  cmake_parse_arguments(ARG "" "" "TARGETS;EXCLUDES;OTHER" ${ARGN})

  file(GLOB_RECURSE
    ALL_CXX_FILES
    ${ARG_TARGETS}
  )

  foreach(exclude ${ARG_EXCLUDES})
    filter_dir(${exclude})
  endforeach()

  find_program(FORMAT_COMMAND
               NAMES clang-format clang-format-12 clang-format-14 clang-format-18)
  if(FORMAT_COMMAND)
    add_custom_target(${target}
      COMMAND ${FORMAT_COMMAND} -i -style=file ${ARG_OTHER} ${ARG_UNPARSED_ARGUMENTS}
              ${ALL_CXX_FILES}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMENT "${comment}"
      USES_TERMINAL
    )
  else()
    message(WARNING "Could not find clang-format executable.")
    add_custom_target(${target}
      COMMAND ${CMAKE_COMMAND} -E echo "${target} does nothing, no clang-format found.")
  endif()
endfunction()
