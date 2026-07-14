cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

# ─────────────────────────────────────────────
# CONFIGURACIÓN DEL PROYECTO
# Generado automáticamente por pico-new
# ─────────────────────────────────────────────
set(PROJECT_NAME    "SPI")
set(PROJECT_SOURCES "SPI.c")
set(PICO_BOARD      "pico")

set(PROJECT_LIBS
    pico_stdlib
    hardware_spi
)
# ─────────────────────────────────────────────
# NO MODIFICAR DE AQUÍ EN ADELANTE
# ─────────────────────────────────────────────

project(${PROJECT_NAME})

pico_sdk_init()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})

pico_add_extra_outputs(${PROJECT_NAME})
