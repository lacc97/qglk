find_package(Qt5 REQUIRED COMPONENTS Core Gui Multimedia Widgets)
set(QT_REQUIRED_LIBS Qt5::Core Qt5::Gui Qt5::Multimedia Qt5::Widgets)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/qglk_window.cpp)
  set_target_properties(qglk PROPERTIES
      AUTOMOC                     ON
      AUTOUIC                     ON
      CXX_VISIBILITY_PRESET       hidden)
  target_compile_definitions(qglk
      PRIVATE
        QT_NO_LINKED_LIST
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
        $<$<NOT:$<CONFIG:Debug>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>)
  target_include_directories(qglk
      PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(qglk
      PRIVATE
        coroutine
        fmt::fmt
        ${QT_REQUIRED_LIBS}
        spdlog::spdlog)
  install(TARGETS qglk DESTINATION ${CMAKE_INSTALL_LIBDIR}/glk)

add_library(qglk_start INTERFACE)
  target_include_directories(qglk_start
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  target_sources(qglk_start
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/glkstart.c)

add_subdirectory(blorb/)
add_subdirectory(event/)
add_subdirectory(file/)
add_subdirectory(lib/coroutine/)
add_subdirectory(log/)
add_subdirectory(sound/)
add_subdirectory(stream/)
add_subdirectory(thread/)
add_subdirectory(window/)
