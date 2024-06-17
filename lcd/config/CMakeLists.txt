# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_Config_SRCS 变量
aux_source_directory(. DIR_CONFIG_SRCS)

# 生成链接库
add_library(config ${DIR_CONFIG_SRCS})
target_link_libraries(config PUBLIC pico_stdlib hardware_spi hardware_pwm)
