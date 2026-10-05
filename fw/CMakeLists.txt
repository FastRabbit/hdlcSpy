cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

project(hdlcspy_fw C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(hdlcspy_fw
    src/main.c
)

pico_enable_stdio_usb(hdlcspy_fw 1)
pico_enable_stdio_uart(hdlcspy_fw 0)

target_link_libraries(hdlcspy_fw
    pico_stdlib
)

pico_add_extra_outputs(hdlcspy_fw)
