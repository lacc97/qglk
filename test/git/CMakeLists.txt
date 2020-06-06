function(add_git_executable _glk)
  set(_target git-${_glk})
  set(_version 1.3.5)

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR}/${_version})

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(_osSrc ${_srcsDir}/git_windows.c)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(_osSrc ${_srcsDir}/git_mac.c)
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_osSrc ${_srcsDir}/git_unix.c)
  else()
    message(FATAL_ERROR "Unsupported system for git: ${CMAKE_SYSTEM_NAME}")
  endif()

  add_executable(${_target}
      ${_osSrc}
      ${_srcsDir}/accel.c
      ${_srcsDir}/compiler.c
      ${_srcsDir}/gestalt.c
      ${_srcsDir}/git.c
      ${_srcsDir}/git_unix.c
      ${_srcsDir}/glkop.c
      ${_srcsDir}/heap.c
      ${_srcsDir}/memory.c
      ${_srcsDir}/opcodes.c
      ${_srcsDir}/operands.c
      ${_srcsDir}/peephole.c
      ${_srcsDir}/savefile.c
      ${_srcsDir}/saveundo.c
      ${_srcsDir}/search.c
      ${_srcsDir}/terp.c)
  set_target_properties(${_target} PROPERTIES
      OUTPUT_NAME ${_glk}-git)
  target_compile_definitions(${_target}
      PRIVATE
        USE_MMAP
        USE_INLINE)
  target_include_directories(${_target}
      PRIVATE
        ${_srcsDir})
  target_link_libraries(${_target}
      PRIVATE
        ${_glk})
  install(TARGETS ${_target} DESTINATION bin)
endfunction()

add_git_executable(qglk)

if(${BUILD_GLKTERM})
    add_git_executable(glkterm)
endif()