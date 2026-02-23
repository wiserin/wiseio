set(WISEIO_BYTE_READER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_namer.cpp)


target_sources(WiseIO PRIVATE ${WISEIO_BYTE_READER_SRC})

add_subdirectory(chunks)
add_subdirectory(file)
add_subdirectory(views)

