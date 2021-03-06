cmake_minimum_required(VERSION 3.16)
project(QGlk)

include(GNUInstallDirs)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)


option(BUILD_GLKTERM    "Build glkterm glkt implementation (in test subdirectory)" OFF)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_C_VISIBILITY_PRESET   hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_subdirectory(lib/)
add_subdirectory(src/)
add_subdirectory(test/)
