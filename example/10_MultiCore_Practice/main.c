/**
 * @file main.c
 * @brief Parpadeo del LED desde Core1, con el intervalo recibido desde Core0 por FIFO
 *        Variante: prueba de paralelismo real (busy_wait + identificacion de nucleo)
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
#include "pico/multicore.h"

/* ─── Defines ──────────────────────────────────────────── */
#define LED_PIN 25

/* ─── Tarea de Core1 ───────────────────────────────────── */
void core1_main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t intervalo_ms = 500;  // valor por defecto, mientras no llegue ninguno por FIFO

    while (1) {
        if (multicore_fifo_rvalid()) {
            intervalo_ms = multicore_fifo_pop_blocking();
            // get_core_num() lee el registro CPUID del propio nucleo (0 o 1);
            // confirma, desde el hardware, que este bloque corre en Core1.
            printf("Core%d: intervalo recibido = %lu ms\n", get_core_num(), intervalo_ms);
        }

        gpio_put(LED_PIN, 1);
        sleep_ms(intervalo_ms);
        gpio_put(LED_PIN, 0);
        sleep_ms(intervalo_ms);
    }
}


/* ─── Main (Core0) ─────────────────────────────────────── */
int main() {
    stdio_init_all();
    multicore_launch_core1(core1_main);

    uint32_t intervalos[] = {500, 100, 250};
    int idx = 0;

    while (1) {
        multicore_fifo_push_blocking(intervalos[idx]);
        printf("Core%d: intervalo enviado = %lu ms\n", get_core_num(), intervalos[idx]);

        idx = (idx + 1) % 3;

        // Variante: en vez de sleep_ms (espera de bajo consumo con evento __wfe),
        // se usa busy_wait_us_32 para mantener a Core0 ocupado activamente durante
        // 4 segundos, sin ceder el CPU en ningun momento. Si el parpadeo de Core1
        // continua sin alteracion durante este bloqueo, queda demostrado que ambos
        // bucles corren en procesadores fisicamente independientes.
        busy_wait_us_32(4000 * 1000);
    }
    return 0;
}