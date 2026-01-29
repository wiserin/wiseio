add_library(WiseIO SHARED)

target_include_directories(WiseIO PUBLIC include/)

add_subdirectory(src)

target_link_libraries(WiseIO PRIVATE WiseIOCore)
target_link_libraries(WiseIO PRIVATE WiseLogging)

