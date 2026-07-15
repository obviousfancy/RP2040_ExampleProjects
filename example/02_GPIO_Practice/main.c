/**
 * @file main.c
 * @brief Lectura de una entrada digital (boton) con pull-up y antirrebote simple
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/* ─── Defines ──────────────────────────────────────────── */
#define BUTTON_PIN 24

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    bool estado_anterior = gpio_get(BUTTON_PIN);

    while (1) {
        bool estado_actual = gpio_get(BUTTON_PIN);

        if (estado_actual != estado_anterior) {
            if (estado_actual) {
                printf("Boton liberado\n");
            } else {
                printf("Boton presionado\n");
            }
            estado_anterior = estado_actual;
        }

        sleep_ms(20);  // Margen de antirrebote
    }
    return 0;
}

