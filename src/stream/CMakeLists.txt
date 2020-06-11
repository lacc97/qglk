target_sources(qglk
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/chunkbuf.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/latin1stream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/membuf.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/nullbuf.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/stream.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unicodestream.cpp)
