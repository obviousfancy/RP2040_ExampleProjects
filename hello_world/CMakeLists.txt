cmake_minimum_required(VERSION 3.13)                    #Minimum version required for compile the project

include(pico_sdk_import.cmake)                          #Importing pico-sdk through the bridge file makeit into the main folder 
# This allows us to use its functions and libraries

project(hello_world)                                    #Project name, make the output files with the name of the project

pico_sdk_init()                                         #init the SDK, configure the ARM toolchain and other settings
# This is necessary to use pico-sdk functions in our project and always put after project name

add_executable(hello_world hello_world.c)               #Add executable file, the first argument is the output file name, the second argument is the source files

target_link_libraries(hello_world pico_stdlib)        # This links all the libraries that are needed for our project to run properly. 
#In this case, we only need pico-sdk libraries.

pico_add_extra_outputs(hello_world)                     #This adds extra outputs like ELF and HEX files to the build process

