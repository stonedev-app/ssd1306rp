# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(ssd1306test C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ssd1306test
  src/ssd1306test.c
  src/ssd1306.c
)

pico_set_program_name(ssd1306test "ssd1306test")
pico_set_program_version(ssd1306test "0.1")

pico_enable_stdio_uart(ssd1306test 1)
pico_enable_stdio_usb(ssd1306test 0)

# Add the standard library to the build
target_link_libraries(ssd1306test
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(ssd1306test PRIVATE
  include/
)

# Add any user requested libraries
target_link_libraries(ssd1306test 
        hardware_i2c
        )

pico_add_extra_outputs(ssd1306test)

