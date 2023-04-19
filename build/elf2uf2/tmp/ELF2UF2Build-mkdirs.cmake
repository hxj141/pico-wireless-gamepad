# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/pico-sdk/tools/elf2uf2"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/tmp"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/src/ELF2UF2Build-stamp"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/src"
  "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/src/ELF2UF2Build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/src/ELF2UF2Build-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/var/home/thonkpad/Documents/hw/sp2023/capstone/pico/src/picousbtest/build/elf2uf2/src/ELF2UF2Build-stamp${cfgdir}") # cfgdir has leading slash
endif()
