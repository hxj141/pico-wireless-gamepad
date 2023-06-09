cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(picousbtest C CXX ASM)
pico_sdk_init()

add_executable(picousbtest)

target_sources(picousbtest PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/nrf_receiver.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(picousbtest PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(picousbtest PUBLIC pico_stdlib tinyusb_device tinyusb_board hardware_spi)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(dev_hid_composite PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(picousbtest)

pico_enable_stdio_usb(picousbtest 1)
pico_enable_stdio_uart(picousbtest 0)

# add url via pico_set_program_url
#example_auto_set_url(dev_hid_composite)
