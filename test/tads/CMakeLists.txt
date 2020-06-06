function(add_tads2_executable _glk)
  set(_target tads2-${_glk})
  set(_version 3.0.8)

  set(_srcsDir ${CMAKE_CURRENT_SOURCE_DIR}/${_version}/tads2)

  if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(_osDef OSANSI GLKUNIX)
  else()
    message(FATAL_ERROR "Unsupported system for tads2: ${CMAKE_SYSTEM_NAME}")
  endif()

  add_executable(${_target}
      ${_srcsDir}/glk/oem_glk.c
      ${_srcsDir}/glk/os_glk.c
      ${_srcsDir}/glk/oss_glk.c
      ${_srcsDir}/glk/glkstart.c
      ${_srcsDir}/ler.c
      ${_srcsDir}/mcm.c
      ${_srcsDir}/mcs.c
      ${_srcsDir}/mch.c
      ${_srcsDir}/obj.c
      ${_srcsDir}/cmd.c
      ${_srcsDir}/errmsg.c
      ${_srcsDir}/fioxor.c
      ${_srcsDir}/oserr.c
      ${_srcsDir}/runstat.c
      ${_srcsDir}/fio.c
      ${_srcsDir}/getstr.c
      ${_srcsDir}/cmap.c
      ${_srcsDir}/askf_os.c
      ${_srcsDir}/indlg_tx.c
      ${_srcsDir}/osifc.c
      ${_srcsDir}/dat.c
      ${_srcsDir}/lst.c
      ${_srcsDir}/run.c
      ${_srcsDir}/out.c
      ${_srcsDir}/voc.c
      ${_srcsDir}/bif.c
      ${_srcsDir}/output.c
      ${_srcsDir}/suprun.c
      ${_srcsDir}/regex.c
      ${_srcsDir}/vocab.c
      ${_srcsDir}/execmd.c
      ${_srcsDir}/ply.c
      ${_srcsDir}/qas.c
      ${_srcsDir}/trd.c
      ${_srcsDir}/dbgtr.c
      ${_srcsDir}/linfdum.c
      ${_srcsDir}/osrestad.c
      ${_srcsDir}/bifgdum.c)
    set_target_properties(${_target} PROPERTIES
        OUTPUT_NAME ${_glk}-tads2)
    target_compile_definitions(${_target}
        PRIVATE
          ${_osDef}
          GLK
          memicmp=strncasecmp)
    target_include_directories(${_target}
        PRIVATE
          ${_srcsDir}
          ${_srcsDir}/glk)
    target_link_libraries(${_target}
        PRIVATE
          ${_glk})
    install(TARGETS ${_target} DESTINATION bin)
endfunction()


add_tads2_executable(qglk)

if(${BUILD_GLKTERM})
  add_tads2_executable(glkterm)
    target_compile_definitions(tads2-glkterm
        PRIVATE
          GLKTERM)
endif()