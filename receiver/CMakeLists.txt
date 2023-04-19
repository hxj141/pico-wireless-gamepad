cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(nrf24_spi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH &{PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_executable(nrf24_spi
        nrf24_spi.c
        )

pico_enable_stdio_usb(nrf24_spi 1)
pico_enable_stdio_uart(nrf24_spi 0)

# Pull in our pico_stdlib which pulls in commonly used libraries
target_link_libraries(nrf24_spi pico_stdlib hardware_spi)

# create map/bin/hex file etc.
pico_add_extra_outputs(nrf24_spi)


