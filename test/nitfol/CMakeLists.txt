find_package(Perl REQUIRED)

function(add_nitfol_executable _glk)
  set(_target nitfol-${_glk})
  set(_version 0.5_p1)

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR}/${_version})

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_os win)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(_os mac)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_os unix)
  else()
    message(FATAL_ERROR "Unsupported system for nitfol: ${CMAKE_SYSTEM_NAME}")
  endif()

  set(_osDir ${CMAKE_CURRENT_BINARY_DIR}/${_version}/${_os})
  set(_osSrc ${_osDir}/start${_os}.c)

  file(MAKE_DIRECTORY ${_osDir})
  add_custom_command(OUTPUT ${_osSrc}
      COMMAND ${CMAKE_COMMAND} -E copy ${_srcsDir}/opt2glkc.pl ${_osDir}/opt2glkc.pl
      COMMAND ${CMAKE_COMMAND} -E copy ${_srcsDir}/nitfol.opt ${_osDir}/nitfol.opt
      COMMAND ${PERL_EXECUTABLE} opt2glkc.pl -${_os} nitfol.opt
      WORKING_DIRECTORY ${_osDir})
  set_property(SOURCE ${_osSrc} PROPERTY GENERATED TRUE)

  add_executable(${_target}
      ${_osSrc}
      ${_srcsDir}/automap.c
      ${_srcsDir}/solve.c
      ${_srcsDir}/infix.c
      ${_srcsDir}/copying.c
      ${_srcsDir}/debug.c
      ${_srcsDir}/quetzal.c
      ${_srcsDir}/undo.c
      ${_srcsDir}/op_call.c
      ${_srcsDir}/decode.c
      ${_srcsDir}/errmesg.c
      ${_srcsDir}/globals.c
      ${_srcsDir}/iff.c
      ${_srcsDir}/init.c
      ${_srcsDir}/main.c
      ${_srcsDir}/io.c
      ${_srcsDir}/z_io.c
      ${_srcsDir}/op_jmp.c
      ${_srcsDir}/op_math.c
      ${_srcsDir}/op_save.c
      ${_srcsDir}/op_table.c
      ${_srcsDir}/op_v6.c
      ${_srcsDir}/oplist.c
      ${_srcsDir}/stack.c
      ${_srcsDir}/zscii.c
      ${_srcsDir}/tokenise.c
      ${_srcsDir}/struct.c
      ${_srcsDir}/objects.c
      ${_srcsDir}/portfunc.c
      ${_srcsDir}/hash.c
      ${_srcsDir}/blorb.c)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME ${_glk}-nitfol)
    target_compile_definitions(${_target}
        PRIVATE
          USE_DIRECT_THREADING
          USE_MMAP
          USE_INLINE)
    target_include_directories(${_target}
        PRIVATE
          ${_srcsDir})
  if(BISON_FOUND)
    target_yacc_sources(${_target}
        PRIVATE
          ${_srcsDir}/inform.y)
  else()
    target_sources(${_target}
        PRIVATE
          ${_srcsDir}/inform.c)
  endif()
    target_link_libraries(${_target}
        PRIVATE
          ${_glk})
    install(TARGETS ${_target} DESTINATION bin)
endfunction()


add_nitfol_executable(qglk)
  target_sources(nitfol-qglk
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/0.5_p1/sound.c
        ${CMAKE_CURRENT_SOURCE_DIR}/0.5_p1/graphics.c)

if(${BUILD_GLKTERM})
  add_nitfol_executable(glkterm)
    target_sources(nitfol-glkterm
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/0.5_p1/no_snd.c
        ${CMAKE_CURRENT_SOURCE_DIR}/0.5_p1/no_graph.c)
endif()