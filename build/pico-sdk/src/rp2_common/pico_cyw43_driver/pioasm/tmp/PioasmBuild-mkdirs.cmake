# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/pico-sdk/tools/pioasm"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pioasm"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
