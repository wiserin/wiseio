set(WISEIO_STREAM_READ_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/cread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/read_all.cpp)


target_sources(WiseIO PRIVATE ${WISEIO_STREAM_READ_SRC})
