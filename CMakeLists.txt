cmake_minimum_required(VERSION 3.0)
project(QGlk)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(QT_REQUIRED_LIBS Qt5::Core Qt5::Gui Qt5::Multimedia Qt5::Widgets)

add_library(qglk STATIC "")
    target_include_directories(qglk PRIVATE src/)
    target_link_libraries(qglk ${QT_REQUIRED_LIBS})

install(TARGETS qglk DESTINATION lib/glk)

add_subdirectory(src/)
add_subdirectory(test/)
