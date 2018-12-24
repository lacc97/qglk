set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(QT_REQUIRED_LIBS Qt5::Core Qt5::Gui Qt5::Multimedia Qt5::Widgets )

add_library(qglk STATIC
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
    target_include_directories(qglk PRIVATE src/)
    target_link_libraries(qglk ${QT_REQUIRED_LIBS})

install(TARGETS qglk DESTINATION lib/glk)


add_subdirectory(blorb/)
add_subdirectory(event/)
add_subdirectory(file/)
add_subdirectory(sound/)
add_subdirectory(stream/)
add_subdirectory(thread/)
add_subdirectory(window/)
