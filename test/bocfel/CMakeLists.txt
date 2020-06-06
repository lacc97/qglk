function(add_bocfel_executable _glk)
  set(_target bocfel-${_glk})
  set(_version 1.0.1)

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR}/${_version})

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_osDef ZTERP_WIN32)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_osDef ZTERP_UNIX)
  else()
    message(FATAL_ERROR "Unsupported system for bocfel: ${CMAKE_SYSTEM_NAME}")
  endif()

  add_executable(${_target}
      ${_srcsDir}/blorb.c
      ${_srcsDir}/branch.c
      ${_srcsDir}/dict.c
      ${_srcsDir}/glkstart.c
      ${_srcsDir}/iff.c
      ${_srcsDir}/io.c
      ${_srcsDir}/math.c
      ${_srcsDir}/meta.c
      ${_srcsDir}/memory.c
      ${_srcsDir}/objects.c
      ${_srcsDir}/osdep.c
      ${_srcsDir}/patches.c
      ${_srcsDir}/process.c
      ${_srcsDir}/random.c
      ${_srcsDir}/screen.c
      ${_srcsDir}/sound.c
      ${_srcsDir}/stack.c
      ${_srcsDir}/unicode.c
      ${_srcsDir}/util.c
      ${_srcsDir}/zoom.c
      ${_srcsDir}/zterp.c)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME ${_glk}-bocfel)
    target_compile_definitions(${_target}
        PRIVATE
          ${_osDef}
          ZTERP_GLK
          ZTERP_GLK_TICK
          ZTERP_NO_CHEAT
          ZTERP_NO_SAFETY_CHECKS
          ZTERP_NO_WATCHPOINTS)
    target_include_directories(${_target}
        PRIVATE
          ${_srcsDir})
    target_link_libraries(${_target}
        PRIVATE
          ${_glk})
    install(TARGETS ${_target} DESTINATION bin)
endfunction()


add_bocfel_executable(qglk)

if(${BUILD_GLKTERM})
  add_bocfel_executable(glkterm)
endif()