add_library(WiseIOCore SHARED)

target_include_directories(WiseIOCore PUBLIC include/)

set_target_properties(WiseIOCore PROPERTIES LINKER_LANGUAGE C)

add_subdirectory(src)
