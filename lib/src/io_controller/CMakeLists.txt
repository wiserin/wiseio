set(WISEIO_STREAM_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.cpp)


target_sources(WiseIO PRIVATE ${WISEIO_STREAM_SRC})

add_subdirectory(sys_raw)
add_subdirectory(read)
add_subdirectory(write)
add_subdirectory(utils)