function(add_multiwin_executable _glk)
  set(_target multiwin-${_glk})

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(${_target}
      ${_srcsDir}/multiwin.c)
  set_target_properties(${_target} PROPERTIES
      OUTPUT_NAME ${_glk}-multiwin)
  target_link_libraries(${_target} ${_glk}_start ${_glk})
endfunction()


add_multiwin_executable(qglk)

if(${BUILD_GLKTERM})
  add_multiwin_executable(glkterm)
endif()