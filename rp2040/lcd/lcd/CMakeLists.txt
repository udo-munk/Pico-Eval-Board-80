# Find all source files in the current directory
# and save their names in the DIR_LCD_SRCS variable
aux_source_directory(. DIR_LCD_SRCS)

include_directories(../config)
include_directories(../font)

# Generate link library
add_library(lcd ${DIR_LCD_SRCS})
target_link_libraries(lcd PUBLIC config font pico_stdlib)
