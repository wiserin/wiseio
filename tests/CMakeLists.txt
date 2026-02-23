add_executable(wiseio_tests
    cases/test_stream_read.cpp
    cases/test_stream_write.cpp
    cases/test_bytes_buffer.cpp
    cases/test_string_buffer.cpp
    cases/test_stream_basic.cpp
    cases/test_integration.cpp
    cases/test_storage.cpp
    cases/test_num_view.cpp
    cases/test_chunks.cpp
    cases/test_bytefile.cpp
    cases/test_wrapper_pattern.cpp
)

target_link_libraries(wiseio_tests
    PRIVATE
    GTest::gtest_main
    WiseIO
    WiseLogging
)

target_include_directories(wiseio_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/include
)

include(GoogleTest)
gtest_discover_tests(wiseio_tests)