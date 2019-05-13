set_property(DIRECTORY . PROPERTY SKIP_AUTOGEN ON)

add_executable(test main.c)
    target_include_directories(test PRIVATE ../src/)
    target_link_libraries(test qglk)

if(UNIX)
    set(MATH_LIBRARY m)
else()
    set(MATH_LIBRARY)
endif()

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

add_library(glkterm STATIC
        glkterm-1.0.4/main.c
#         glkterm-1.0.4/glkstart.c
        glkterm-1.0.4/gtevent.c
        glkterm-1.0.4/gtfref.c
        glkterm-1.0.4/gtgestal.c
        glkterm-1.0.4/gtinput.c
        glkterm-1.0.4/gtmessag.c
        glkterm-1.0.4/gtmessin.c
        glkterm-1.0.4/gtmisc.c
        glkterm-1.0.4/gtstream.c
        glkterm-1.0.4/gtstyle.c
        glkterm-1.0.4/gtw_blnk.c
        glkterm-1.0.4/gtw_buf.c
        glkterm-1.0.4/gtw_grid.c
        glkterm-1.0.4/gtw_pair.c
        glkterm-1.0.4/gtwindow.c
        glkterm-1.0.4/gtschan.c
        glkterm-1.0.4/gtblorb.c
        glkterm-1.0.4/cgunicod.c
        glkterm-1.0.4/cgdate.c
        glkterm-1.0.4/gi_dispa.c
        glkterm-1.0.4/gi_blorb.c
)
    target_include_directories(glkterm PRIVATE glkterm-1.0.4/ ${CURSES_INCLUDE_DIR})
    target_link_libraries(glkterm ${CURSES_LIBRARIES})


add_executable(model-qglk
        model.c
        ../src/glkstart.c
)
    target_include_directories(model-qglk PRIVATE ../src/)
    target_link_libraries(model-qglk qglk)
    
add_executable(model-glkterm
        model.c
        glkterm-1.0.4/glkstart.c
)
    target_include_directories(model-glkterm PRIVATE glkterm-1.0.4/)
    target_link_libraries(model-glkterm glkterm)
    

add_executable(multiwin-qglk
        multiwin.c
        ../src/glkstart.c
)
    target_include_directories(multiwin-qglk PRIVATE ../src/)
    target_link_libraries(multiwin-qglk qglk)
    
add_executable(multiwin-glkterm
        multiwin.c
        glkterm-1.0.4/glkstart.c
)
    target_include_directories(multiwin-glkterm PRIVATE glkterm-1.0.4/)
    target_link_libraries(multiwin-glkterm glkterm)
    

set(GIT_SOURCES
    git-1.3.5/accel.c
    git-1.3.5/compiler.c
    git-1.3.5/gestalt.c
    git-1.3.5/git.c
    git-1.3.5/git_unix.c
    git-1.3.5/glkop.c
    git-1.3.5/heap.c
    git-1.3.5/memory.c
    git-1.3.5/opcodes.c
    git-1.3.5/operands.c
    git-1.3.5/peephole.c
    git-1.3.5/savefile.c
    git-1.3.5/saveundo.c
    git-1.3.5/search.c
    git-1.3.5/terp.c
)
    
add_executable(git-qglk ${GIT_SOURCES})
    target_compile_definitions(git-qglk PRIVATE "-DUSE_MMAP -DUSE_INLINE")
    target_include_directories(git-qglk PRIVATE git-1.3.5/ ../src/)
    target_link_libraries(git-qglk qglk)
install(TARGETS git-qglk DESTINATION bin)
    
add_executable(git-glkterm ${GIT_SOURCES})
    target_compile_definitions(git-glkterm PRIVATE "-DUSE_MMAP -DUSE_INLINE")
    target_include_directories(git-glkterm PRIVATE git-1.3.5/ glkterm-1.0.4/)
    target_link_libraries(git-glkterm glkterm ${MATH_LIBRARY})

    
find_package(Perl REQUIRED)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startunix.c
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/opt2glkc.pl ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-opt2glkc.pl
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/nitfol.opt ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-nitfol.opt
    COMMAND ${PERL_EXECUTABLE} nitfol-0.5_p1-opt2glkc.pl -unix nitfol-0.5_p1-nitfol.opt
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/startunix.c ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startunix.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startunix.c PROPERTY GENERATED TRUE)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startwin.c
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/opt2glkc.pl ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-opt2glkc.pl
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/nitfol.opt ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-nitfol.opt
    COMMAND ${PERL_EXECUTABLE} nitfol-0.5_p1-opt2glkc.pl -win nitfol-0.5_p1-nitfol.opt
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/startwin.c ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startwin.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startwin.c PROPERTY GENERATED TRUE)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startmac.c
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/opt2glkc.pl ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-opt2glkc.pl
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/nitfol-0.5_p1/nitfol.opt ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-nitfol.opt
    COMMAND ${PERL_EXECUTABLE} nitfol-0.5_p1-opt2glkc.pl -mac nitfol-0.5_p1-nitfol.opt
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/startmac.c ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startmac.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startmac.c PROPERTY GENERATED TRUE)
    
find_package(BISON)
if(BISON_FOUND)
    bison_target(InformParser nitfol-0.5_p1/inform.y ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-inform.c COMPILE_FLAGS "-y")
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-inform.c PROPERTY GENERATED TRUE)
    set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-inform.h PROPERTY SKIP_AUTOGEN TRUE)
else()
    set(BISON_InformParser_OUTPUTS nitfol-0.5_p1/inform.c)
endif()

set(NITFOL_SOURCES
    nitfol-0.5_p1/automap.c
    nitfol-0.5_p1/solve.c
    nitfol-0.5_p1/infix.c
    nitfol-0.5_p1/copying.c
    nitfol-0.5_p1/debug.c
    ${BISON_InformParser_OUTPUTS}
    nitfol-0.5_p1/quetzal.c
    nitfol-0.5_p1/undo.c
    nitfol-0.5_p1/op_call.c
    nitfol-0.5_p1/decode.c
    nitfol-0.5_p1/errmesg.c
    nitfol-0.5_p1/globals.c
    nitfol-0.5_p1/iff.c
    nitfol-0.5_p1/init.c
    nitfol-0.5_p1/main.c
    nitfol-0.5_p1/io.c
    nitfol-0.5_p1/z_io.c
    nitfol-0.5_p1/op_jmp.c
    nitfol-0.5_p1/op_math.c
    nitfol-0.5_p1/op_save.c
    nitfol-0.5_p1/op_table.c
    nitfol-0.5_p1/op_v6.c
    nitfol-0.5_p1/oplist.c
    nitfol-0.5_p1/stack.c
    nitfol-0.5_p1/zscii.c
    nitfol-0.5_p1/tokenise.c
    nitfol-0.5_p1/struct.c
    nitfol-0.5_p1/objects.c
    nitfol-0.5_p1/portfunc.c
    nitfol-0.5_p1/hash.c
    nitfol-0.5_p1/blorb.c
)

if(WIN32) # windows
    set(NITFOL_OS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startwin.c)
elseif(UNIX AND NOT APPLE) # linux
    set(NITFOL_OS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startunix.c)
elseif(APPLE) # mac
    set(NITFOL_OS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/nitfol-0.5_p1-startmac.c)
endif()
    
add_executable(nitfol-qglk
        ${NITFOL_SOURCES}
        ${NITFOL_OS_SOURCES}
        nitfol-0.5_p1/sound.c
        nitfol-0.5_p1/graphics.c
)
    target_compile_definitions(nitfol-qglk PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
    target_include_directories(nitfol-qglk PRIVATE nitfol-0.5_p1/ ../src/)
    target_link_libraries(nitfol-qglk qglk)
install(TARGETS nitfol-qglk DESTINATION bin)

add_executable(nitfol-glkterm
        ${NITFOL_SOURCES}
        ${NITFOL_OS_SOURCES}
        nitfol-0.5_p1/no_graph.c
        nitfol-0.5_p1/no_snd.c
)
    target_compile_definitions(nitfol-glkterm PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
    target_include_directories(nitfol-glkterm PRIVATE nitfol-0.5_p1/ glkterm-1.0.4/)
    target_link_libraries(nitfol-glkterm glkterm ${MATH_LIBRARY})

set(GLULXE_SOURCES
    glulxe-0.5.4/main.c
    glulxe-0.5.4/files.c
    glulxe-0.5.4/vm.c
    glulxe-0.5.4/exec.c
    glulxe-0.5.4/funcs.c
    glulxe-0.5.4/operand.c
    glulxe-0.5.4/string.c
    glulxe-0.5.4/glkop.c
    glulxe-0.5.4/heap.c
    glulxe-0.5.4/serial.c
    glulxe-0.5.4/search.c
    glulxe-0.5.4/accel.c
    glulxe-0.5.4/float.c
    glulxe-0.5.4/gestalt.c
    glulxe-0.5.4/osdepend.c
    glulxe-0.5.4/profile.c
    glulxe-0.5.4/debugger.c
)

if(UNIX)
    set(GLULXE_OS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/glulxe-0.5.4/unixstrt.c)
endif()

add_executable(glulxe-qglk
        ${GLULXE_SOURCES}
        ${GLULXE_OS_SOURCES}
)
    target_compile_definitions(glulxe-qglk PRIVATE "-DOS_UNIX")
    target_include_directories(glulxe-qglk PRIVATE glulxe-0.5.4/ ../src/)
    target_link_libraries(glulxe-qglk qglk)
install(TARGETS glulxe-qglk DESTINATION bin)

set(TADS2_SOURCES
    tads-3.0.8/tads2/glk/oem_glk.c
    tads-3.0.8/tads2/glk/os_glk.c
    tads-3.0.8/tads2/glk/oss_glk.c
    tads-3.0.8/tads2/glk/glkstart.c
    tads-3.0.8/tads2/ler.c
    tads-3.0.8/tads2/mcm.c
    tads-3.0.8/tads2/mcs.c
    tads-3.0.8/tads2/mch.c
    tads-3.0.8/tads2/obj.c
    tads-3.0.8/tads2/cmd.c
    tads-3.0.8/tads2/errmsg.c
    tads-3.0.8/tads2/fioxor.c
    tads-3.0.8/tads2/oserr.c
    tads-3.0.8/tads2/runstat.c
    tads-3.0.8/tads2/fio.c
    tads-3.0.8/tads2/getstr.c
    tads-3.0.8/tads2/cmap.c
    tads-3.0.8/tads2/askf_os.c
    tads-3.0.8/tads2/indlg_tx.c
    tads-3.0.8/tads2/osifc.c
    tads-3.0.8/tads2/dat.c
    tads-3.0.8/tads2/lst.c
    tads-3.0.8/tads2/run.c
    tads-3.0.8/tads2/out.c
    tads-3.0.8/tads2/voc.c
    tads-3.0.8/tads2/bif.c
    tads-3.0.8/tads2/output.c
    tads-3.0.8/tads2/suprun.c
    tads-3.0.8/tads2/regex.c
    tads-3.0.8/tads2/vocab.c
    tads-3.0.8/tads2/execmd.c
    tads-3.0.8/tads2/ply.c
    tads-3.0.8/tads2/qas.c
    tads-3.0.8/tads2/trd.c
    tads-3.0.8/tads2/dbgtr.c
    tads-3.0.8/tads2/linfdum.c
    tads-3.0.8/tads2/osrestad.c
    tads-3.0.8/tads2/bifgdum.c
)

add_executable(tads2-qglk
        ${TADS2_SOURCES}
)
    target_compile_definitions(tads2-qglk PRIVATE
        OSANSI
        GLKUNIX
        GLK
        memicmp=strncasecmp
    )
    target_include_directories(tads2-qglk PRIVATE tads-3.0.8/tads2/ tads-3.0.8/tads2/glk ../src/)
    target_link_libraries(tads2-qglk qglk)
install(TARGETS tads2-qglk DESTINATION bin)

add_executable(tads2-glkterm
        ${TADS2_SOURCES}
)
    target_compile_definitions(tads2-glkterm PRIVATE
        OSANSI
        GLKTERM
        GLKUNIX
        GLK
        memicmp=strncasecmp
    )
    target_include_directories(tads2-glkterm PRIVATE tads-3.0.8/tads2/ tads-3.0.8/tads2/glk ../src/)
    target_link_libraries(tads2-glkterm glkterm ${MATH_LIBRARY})
