set_property(DIRECTORY . PROPERTY SKIP_AUTOGEN ON)

add_executable(model-qglk
        model.c
        ../src/glkstart.c
)
    target_include_directories(model-qglk PRIVATE ../src/)
    target_link_libraries(model-qglk qglk)

add_executable(multiwin-qglk
        multiwin.c
        ../src/glkstart.c
)
    target_include_directories(multiwin-qglk PRIVATE ../src/)
    target_link_libraries(multiwin-qglk qglk)

add_executable(git-qglk 
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
        git-1.3.5/terp.c)
    target_compile_definitions(git-qglk PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
    target_include_directories(git-qglk PRIVATE git-1.3.5/ ../src/)
    target_link_libraries(git-qglk qglk)

    
find_package(BISON REQUIRED)
bison_target(InformParser nitfol-0.5_p1/inform.y ${CMAKE_CURRENT_BINARY_DIR}/inform.c COMPILE_FLAGS "-y")
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/inform.h PROPERTY SKIP_AUTOGEN ON)

add_executable(nitfol-qglk
        nitfol-0.5_p1/automap.c
        nitfol-0.5_p1/solve.c
        nitfol-0.5_p1/infix.c
        nitfol-0.5_p1/copying.c
        nitfol-0.5_p1/debug.c
        ${BISON_InformParser_OUTPUTS}
#         nitfol-0.5_p1/inform.c
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
        nitfol-0.5_p1/startunix.c
        nitfol-0.5_p1/graphics.c
        nitfol-0.5_p1/blorb.c
        nitfol-0.5_p1/sound.c
)
#     target_compile_options(nitfol-qglk PRIVATE "-ansi")
    target_compile_definitions(nitfol-qglk PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(nitfol-qglk PRIVATE "-DDEBUGGING")
endif()
    target_include_directories(nitfol-qglk PRIVATE nitfol-0.5_p1// ../src/)
    target_link_libraries(nitfol-qglk qglk)
        

find_package(Curses REQUIRED)

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

add_executable(model-glkterm
        model.c
        glkterm-1.0.4/glkstart.c
)
    target_include_directories(model-glkterm PRIVATE glkterm-1.0.4/)
    target_link_libraries(model-glkterm glkterm)

add_executable(multiwin-glkterm
        multiwin.c
        glkterm-1.0.4/glkstart.c
)
    target_include_directories(multiwin-glkterm PRIVATE glkterm-1.0.4/)
    target_link_libraries(multiwin-glkterm glkterm)

add_executable(git-glkterm
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
        git-1.3.5/terp.c)
    target_compile_definitions(git-glkterm PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
    target_include_directories(git-glkterm PRIVATE git-1.3.5/ glkterm-1.0.4/)
    target_link_libraries(git-glkterm glkterm)

add_executable(nitfol-glkterm
        nitfol-0.5_p1/automap.c
        nitfol-0.5_p1/solve.c
        nitfol-0.5_p1/infix.c
        nitfol-0.5_p1/copying.c
        nitfol-0.5_p1/debug.c
        ${BISON_InformParser_OUTPUTS}
#         nitfol-0.5_p1/inform.c
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
        nitfol-0.5_p1/startunix.c
        nitfol-0.5_p1/no_graph.c
        nitfol-0.5_p1/blorb.c
        nitfol-0.5_p1/no_snd.c
)
    target_compile_options(nitfol-glkterm PRIVATE "-ansi")
    target_compile_definitions(nitfol-glkterm PRIVATE "-DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(nitfol-glkterm PRIVATE "-DDEBUGGING")
endif()
    target_include_directories(nitfol-glkterm PRIVATE nitfol-0.5_p1/ glkterm-1.0.4/)
    target_link_libraries(nitfol-glkterm glkterm)
