#
# top-level CMake file, see also CMakePresets.json
#
# usage:
#
# cmake --preset Debug
# cmake --build --preset Debug
#
# cmake --preset Release
# cmake --build --preset Release
#
#

cmake_minimum_required(VERSION 3.25)


# Toolchain file and compiler flags must be specified before the project() command
# because project() triggers compiler detection and evaluation.
include(toolchain_arm-none-eabi-gcc-13.3.Rel1.cmake)
include(common_settings.cmake)


# set up project and enable languages
project(hello_world C CXX ASM)

# utils
include(cmake_utils.cmake)

# product specific defines
list(APPEND compile_definitions "CORE_M4")
list(APPEND compile_definitions "__CORTEX_M4F")
# config-specific defines from CMakePresets.json
list(APPEND compile_definitions ${config_compile_definitions})

# project specific includes
list(APPEND include_directories "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND include_directories "${CMAKE_CURRENT_LIST_DIR}/system/include/cmsis")
list(APPEND include_directories "${CMAKE_CURRENT_LIST_DIR}/system/include/platform")
list(APPEND include_directories "${CMAKE_CURRENT_LIST_DIR}/system/include/S6E2CCA/PDL")




#
# firmware image
#
set(target_name "${PROJECT_NAME}.axf")
add_executable(${target_name})

target_compile_definitions(${target_name} PRIVATE ${compile_definitions} )
target_include_directories(${target_name} PRIVATE ${include_directories} )


# linker script
set(linker_script "${CMAKE_CURRENT_LIST_DIR}/ldscripts/s6e2cca_rom.ld")
target_link_options(${target_name} PRIVATE "-T${linker_script}")

# create map file
target_link_options(${target_name} PRIVATE "-Wl,-Map=${PROJECT_NAME}.map")

# create .hex file as post-build step
add_custom_command(TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} --output-target ihex "${PROJECT_NAME}.axf" "${PROJECT_NAME}.hex"
    VERBATIM
    )


target_sources(${target_name} PRIVATE
    system/include/S6E2CCA/startup/startup_s6e2cc.S
    system/include/S6E2CCA/PDL/system_s6e2cc.c
    system/include/platform/utils.c
    system/include/platform/platform.c
    hello_world.cpp
    )


