find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

add_library(c_math INTERFACE)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_link_libraries(c_math INTERFACE m)
endif()

add_library(glkterm STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtevent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtfref.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtgestal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtinput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtmessag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtmessin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtmisc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtstyle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtw_blnk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtw_buf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtw_grid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtw_pair.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtwindow.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtschan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gtblorb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/cgunicod.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/cgdate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gi_dispa.c
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/gi_blorb.c)
  target_compile_options(glkterm
      PRIVATE
        ${CURSES_CFLAGS})
  target_include_directories(glkterm
      PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4
      PRIVATE
        ${CURSES_INCLUDE_DIRS})
  target_link_libraries(glkterm
      PRIVATE
        ${CURSES_LIBRARIES}
      PUBLIC
        c_math)

add_library(glkterm_start INTERFACE)
  target_include_directories(glkterm_start
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4)
  target_sources(glkterm_start
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/1.0.4/glkstart.c)