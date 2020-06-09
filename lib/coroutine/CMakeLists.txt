add_library(coroutine INTERFACE)
  target_compile_features(coroutine
      INTERFACE
        cxx_std_11)
  target_include_directories(coroutine
      INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)