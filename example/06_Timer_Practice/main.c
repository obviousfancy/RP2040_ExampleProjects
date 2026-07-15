/**
 * @file main.c
 * @brief Parpadeo del LED integrado mediante un temporizador repetitivo de hardware
 *
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/timer.h"

/* ─── Defines ──────────────────────────────────────────── */
#define LED_PIN        25
#define INTERVALO_MS   500


/* ─── Callback del temporizador ────────────────────────────
 * Se invoca automaticamente cada INTERVALO_MS. Retornar
 * "true" indica al SDK que debe programar la siguiente
 * repeticion; retornar "false" la cancelaria.
 */
bool repetir_callback(struct repeating_timer *t) {
    static bool estado = false;
    estado = !estado;
    gpio_put(LED_PIN, estado);
    return true;
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    struct repeating_timer timer;
    add_repeating_timer_ms(-INTERVALO_MS, repetir_callback, NULL, &timer);

        // Usa sleep_ms() / sleep_us() de pico_stdlib
    // o configura alarmas con hardware_alarm_*


    while (1) {
        // TODO: lógica principal del proyecto
        tight_loop_contents();
    }
    return 0;
}

