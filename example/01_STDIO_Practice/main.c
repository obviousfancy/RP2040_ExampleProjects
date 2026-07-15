/**
 * @file main.c
 * @brief Proyecto 01_STDIO_Practice para Raspberry Pi Pico
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

/* ─── Main ─────────────────────────────────────────────── */
int main() {

stdio_init_all();

    uint32_t contador = 0;
    while (1) {
        printf("Contador: %lu\n", contador);
        contador++;
        sleep_ms(1000);

    }
}

