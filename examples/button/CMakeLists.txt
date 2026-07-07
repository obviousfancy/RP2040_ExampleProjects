cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(button)

pico_sdk_init()

add_executable(button button.c)

target_link_libraries(button pico_stdlib hardware_gpio)

pico_add_extra_outputs(button)