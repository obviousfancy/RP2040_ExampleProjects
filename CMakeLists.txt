cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

# ─────────────────────────────────────────────
# CONFIGURACIÓN DEL PROYECTO — editar aquí
# ─────────────────────────────────────────────
set(PROJECT_NAME    "mi_proyecto")   # nombre del proyecto y del .elf/.uf2
set(PROJECT_SOURCES "main.c")        # archivos .c separados por espacio
set(PICO_BOARD      "pico")          # pico | pico2 | pico_w

# Librerías del SDK — agrega o quita según lo que uses
set(PROJECT_LIBS
    pico_stdlib
    # hardware_gpio
    # hardware_pwm
    # hardware_i2c
    # hardware_spi
    # hardware_uart
    # hardware_adc
    # hardware_timer
    # hardware_irq
)
# ─────────────────────────────────────────────
# NO MODIFICAR DE AQUÍ EN ADELANTE
# ─────────────────────────────────────────────

project(${PROJECT_NAME})

pico_sdk_init()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})

pico_add_extra_outputs(${PROJECT_NAME})
