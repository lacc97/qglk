target_sources(qglk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gi_blorb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gi_dispa.c
    ${CMAKE_CURRENT_SOURCE_DIR}/glk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_blorb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_char.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_dispatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_fileref.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_time.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_window.cpp
)

add_subdirectory(blorb/)
add_subdirectory(event/)
add_subdirectory(file/)
add_subdirectory(sound/)
add_subdirectory(stream/)
add_subdirectory(thread/)
add_subdirectory(window/)
