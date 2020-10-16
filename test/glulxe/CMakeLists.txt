function(add_glulxe_executable _glk)
  set(_target glulxe-${_glk})
  set(_version 0.5.4)

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR}/${_version})

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_osSrc ${_srcsDir}/unixstrt.c)
    set(_osDef OS_UNIX)
  else()
    message(FATAL_ERROR "Unsupported system for glulxe: ${CMAKE_SYSTEM_NAME}")
  endif()

  add_executable(${_target}
      ${_osSrc}
      ${_srcsDir}/main.c
      ${_srcsDir}/files.c
      ${_srcsDir}/vm.c
      ${_srcsDir}/exec.c
      ${_srcsDir}/funcs.c
      ${_srcsDir}/operand.c
      ${_srcsDir}/string.c
      ${_srcsDir}/glkop.c
      ${_srcsDir}/heap.c
      ${_srcsDir}/serial.c
      ${_srcsDir}/search.c
      ${_srcsDir}/accel.c
      ${_srcsDir}/float.c
      ${_srcsDir}/gestalt.c
      ${_srcsDir}/osdepend.c
      ${_srcsDir}/profile.c
      ${_srcsDir}/debugger.c)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME ${_glk}-glulxe)
    target_compile_definitions(${_target}
        PRIVATE
          ${_osDef})
    target_include_directories(${_target}
        PRIVATE
          ${_srcsDir})
    target_link_libraries(${_target}
        PRIVATE
          ${_glk})
    install(TARGETS ${_target} DESTINATION bin)
endfunction()


add_glulxe_executable(qglk)

if(${BUILD_GLKTERM})
  add_glulxe_executable(glkterm)
endif()