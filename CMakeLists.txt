cmake_minimum_required(VERSION 3.13)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
set(CMAKE_BUILD_TYPE Release)
endif()

# Set platform, board, and compiler
set(PICO_PLATFORM rp2040)
set (PICO_BOARD pico)
#set(PICO_BOARD pico_w)
set(PICO_COMPILER pico_arm_gcc)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(picosim C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
	picosim.c
	disks.c
	simcfg.c
	simio.c
	simmem.c
	lcd.c
	../z80pack/iodevices/sd-fdc.c
	../z80pack/z80core/sim8080.c
	../z80pack/z80core/simcore.c
	../z80pack/z80core/simdis.c
	../z80pack/z80core/simglb.c
	../z80pack/z80core/simice.c
	../z80pack/z80core/simutil.c
	../z80pack/z80core/simz80-cb.c
	../z80pack/z80core/simz80-dd.c
	../z80pack/z80core/simz80-ddcb.c
	../z80pack/z80core/simz80-ed.c
	../z80pack/z80core/simz80-fd.c
	../z80pack/z80core/simz80-fdcb.c
	../z80pack/z80core/simz80.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}
	../z80pack/iodevices
	../z80pack/z80core
	./lcd/config
	./lcd/font
	./lcd/lcd
)


add_subdirectory(no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src FatFs)
add_subdirectory(stdio_msc_usb)
add_subdirectory(lcd/config)
add_subdirectory(lcd/font)
add_subdirectory(lcd/lcd)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	PICO_STACK_SIZE=4096
	PICO_CORE1_STACK_SIZE=4096
	PICO_HEAP_SIZE=8192
)

# compiler diagnostic options
if (PICO_C_COMPILER_IS_GNU)
	target_compile_options(${PROJECT_NAME} PUBLIC -fdiagnostics-color=always)
elseif (PICO_C_COMPILER_IS_CLANG)
	target_compile_options(${PROJECT_NAME} PUBLIC -fcolor-diagnostics)
endif ()
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
if (PICO_C_COMPILER_IS_CLANG)
	target_compile_options(${PROJECT_NAME} PUBLIC -Weverything)
endif ()
# -Wno-unused-parameter because no-OS-FatFS-SD-SDIO-SPI-RPi-Pico isn't clean
target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unused-parameter)

target_link_libraries(${PROJECT_NAME}
	pico_stdlib
	stdio_msc_usb
	tinyusb_device
	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
	lcd
	font
	config
	hardware_spi
)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_options(${PROJECT_NAME} PRIVATE -Xlinker --print-memory-usage)

pico_set_program_name(picosim "z80pack picosim")
pico_set_program_description(picosim "z80pack on Waveshare Pico-Eval-Board")
pico_set_program_version(picosim "1.2")
pico_set_program_url(picosim "https://github.com/udo-munk/Pico-Eval-Board-80")

# disable UART in/out, enable USB in/out
pico_enable_stdio_uart(picosim 0)
#pico_enable_stdio_usb(picosim 1)
