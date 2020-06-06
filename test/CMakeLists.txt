include(cmake/FlexBison.cmake)

if(${BUILD_GLKTERM})
  add_subdirectory(glkterm)
endif()

add_subdirectory(git)
add_subdirectory(glulxe)
add_subdirectory(model)
add_subdirectory(multiwin)
add_subdirectory(nitfol)
add_subdirectory(tads)