find_package(BISON)
find_package(FLEX)

if(FLEX_FOUND)
  function(target_flex_sources target ...)
    math(EXPR _argcEnd "${ARGC} - 1")
    foreach(_i RANGE 1 ${_argcEnd})
      set(_currArg ${ARGV${_i}})
      if(${_currArg} STREQUAL PRIVATE)
        set(_currScope PRIVATE)
      elseif(${_currArg} STREQUAL PUBLIC)
        set(_currScope PUBLIC)
      elseif(${_currArg} STREQUAL INTERFACE)
        set(_currScope INTERFACE)
      else()
        if(NOT _currScope)
          message(FATAL_ERROR "Visibility (PRIVATE, PUBLIC or INTERFACE) not specified for '${_currArg}'")
        endif()

        # We process the source
        get_filename_component(_inAbsFile ${_currArg} ABSOLUTE)
        get_filename_component(_inFile    ${_currArg} NAME)
        string(REGEX REPLACE ".y$" "" _inBaseFile ${_inFile})

        set(_outDir     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/flex)
        set(_outFileSrc ${_outDir}/${_inBaseFile}.yy.c)

        add_custom_command(OUTPUT ${_outFileSrc}
            COMMAND               ${CMAKE_COMMAND} -E make_directory ${_outDir}
            COMMAND               ${FLEX_EXECUTABLE} --outfile=${_outFileSrc} ${_inAbsFile}
            MAIN_DEPENDENCY       ${_inAbsFile}
            COMMENT               "Generating flex scanner from ${_inFile}")
        target_include_directories(${target} ${_currScope} ${_outDir})
        target_sources(${target} ${_currScope} ${_outFileSrc})
      endif()
    endforeach()
  endfunction()
endif()

if(BISON_FOUND)
  function(target_bison_sources target ...)
    math(EXPR _argcEnd "${ARGC} - 1")
    foreach(_i RANGE 1 ${_argcEnd})
      set(_currArg ${ARGV${_i}})
      if(${_currArg} STREQUAL PRIVATE)
        set(_currScope PRIVATE)
      elseif(${_currArg} STREQUAL PUBLIC)
        set(_currScope PUBLIC)
      elseif(${_currArg} STREQUAL INTERFACE)
        set(_currScope INTERFACE)
      else()
        if(NOT _currScope)
          message(FATAL_ERROR "Visibility (PRIVATE, PUBLIC or INTERFACE) not specified for '${_currArg}'")
        endif()

        # We process the source
        get_filename_component(_inAbsFile  ${_currArg} ABSOLUTE)
        get_filename_component(_inFile     ${_currArg} NAME)
        string(REGEX REPLACE ".y$" "" _inBaseFile ${_inFile})

        set(_outDir     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/bison)
        set(_outFileSrc ${_outDir}/${_inBaseFile}.tab.c)
        set(_outFileHdr ${_outDir}/${_inBaseFile}.tab.h)

        add_custom_command(OUTPUT ${_outFileSrc} ${_outFileHdr}
            COMMAND               ${CMAKE_COMMAND} -E make_directory ${_outDir}
            COMMAND               ${BISON_EXECUTABLE} --name-prefix=${_inBaseFile}_ --defines=${_outFileHdr} --output=${_outFileSrc} ${_inAbsFile}
            MAIN_DEPENDENCY       ${_inAbsFile}
            COMMENT               "Generating bison parser from ${_inFile}")
        target_include_directories(${target} ${_currScope} ${_outDir})
        target_sources(${target} ${_currScope} ${_outFileSrc} ${_outFileHdr})
      endif()
    endforeach()
  endfunction()

  function(target_yacc_sources target ...)
    math(EXPR _argcEnd "${ARGC} - 1")
    foreach(_i RANGE 1 ${_argcEnd})
      set(_currArg ${ARGV${_i}})
      if(${_currArg} STREQUAL PRIVATE)
        set(_currScope PRIVATE)
      elseif(${_currArg} STREQUAL PUBLIC)
        set(_currScope PUBLIC)
      elseif(${_currArg} STREQUAL INTERFACE)
        set(_currScope INTERFACE)
      else()
        if(NOT _currScope)
          message(FATAL_ERROR "Visibility (PRIVATE, PUBLIC or INTERFACE) not specified for '${_currArg}'")
        endif()

        # We process the source
        get_filename_component(_inAbsFile  ${_currArg} ABSOLUTE)
        get_filename_component(_inFile     ${_currArg} NAME)
        string(REGEX REPLACE ".y$" "" _inBaseFile ${_inFile})

        set(_outDir     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/bison)
        set(_outFileSrc ${_outDir}/${_inBaseFile}.c)
        set(_outFileHdr ${_outDir}/${_inBaseFile}.h)

        add_custom_command(OUTPUT ${_outFileSrc} ${_outFileHdr}
            COMMAND               ${CMAKE_COMMAND} -E make_directory ${_outDir}
            COMMAND               ${BISON_EXECUTABLE} -y --name-prefix=${_inBaseFile}_ --defines=${_outFileHdr} --output=${_outFileSrc} ${_inAbsFile}
            MAIN_DEPENDENCY       ${_inAbsFile}
            COMMENT               "Generating yacc parser from ${_inFile}")
        target_include_directories(${target} ${_currScope} ${_outDir})
        target_sources(${target} ${_currScope} ${_outFileSrc} ${_outFileHdr})
      endif()
    endforeach()
  endfunction()
endif()