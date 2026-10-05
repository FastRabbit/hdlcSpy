cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)

project(hdlcspy_fw C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# clipilot's own tests/host demo are host programs; don't build them for the target.
set(CLI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CLI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/clipilot)

add_executable(hdlcspy_fw
    src/main.c
    src/led.c
)

pico_enable_stdio_usb(hdlcspy_fw 1)
pico_enable_stdio_uart(hdlcspy_fw 0)

target_link_libraries(hdlcspy_fw
    pico_stdlib
    pico_rand
    clipilot::clipilot
)

pico_add_extra_outputs(hdlcspy_fw)
