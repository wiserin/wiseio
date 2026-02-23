add_executable(wiseio_bench
    bench_read.cpp
)
target_link_libraries(wiseio_bench PRIVATE WiseIO WiseLogging)

add_executable(wiseio_bench_stream
    bench_stream.cpp
)
target_link_libraries(wiseio_bench_stream PRIVATE WiseIO WiseLogging)


add_test(
    NAME WiseIO.Bench.Structured
    COMMAND wiseio_bench
)

set_tests_properties(WiseIO.Bench.Structured PROPERTIES
    TIMEOUT 300
    LABELS  "benchmark"
)

add_test(
    NAME WiseIO.Bench.Stream
    COMMAND wiseio_bench_stream
)
set_tests_properties(WiseIO.Bench.Stream PROPERTIES
    TIMEOUT 300
    LABELS  "benchmark"
)


add_custom_target(bench
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "========================================================================"
    COMMAND ${CMAKE_COMMAND} -E echo "  WiseIO Performance Suite"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================================================"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND $<TARGET_FILE:wiseio_bench>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND $<TARGET_FILE:wiseio_bench_stream>
    DEPENDS wiseio_bench wiseio_bench_stream
    COMMENT "Running full WiseIO benchmark suite"
    VERBATIM
)