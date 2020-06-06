function(add_model_executable _glk)
  set(_target model-${_glk})

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(${_target}
      ${_srcsDir}/model.c)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME ${_glk}-model)
    target_link_libraries(${_target} ${_glk}_start ${_glk})
endfunction()


add_model_executable(qglk)

if(${BUILD_GLKTERM})
  add_model_executable(glkterm)
endif()