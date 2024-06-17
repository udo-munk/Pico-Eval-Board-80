aux_source_directory(. DIR_LCD_SRCS)

include_directories(../config)
include_directories(../font)

add_library(lcd ${DIR_LCD_SRCS})
target_link_libraries(lcd PUBLIC config font pico_stdlib)
