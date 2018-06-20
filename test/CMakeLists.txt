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
    target_include_directories(git-qglk PRIVATE git-1.3.5/ ../src/)
    target_link_libraries(git-qglk qglk)

find_package(Curses REQUIRED)

add_library(glkterm STATIC
    glkterm-1.0.4/main.c
#     glkterm-1.0.4/glkstart.c
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
    target_include_directories(git-glkterm PRIVATE git-1.3.5/ glkterm-1.0.4/)
    target_link_libraries(git-glkterm glkterm)
