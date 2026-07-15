if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Run the build with 'picodev build'.")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
