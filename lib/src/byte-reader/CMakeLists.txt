set(WISEIO_BYTE_READER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/byte_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_engine.cpp)


target_sources(WiseIO PRIVATE ${WISEIO_BYTE_READER_SRC})
add_subdirectory(chunks)

