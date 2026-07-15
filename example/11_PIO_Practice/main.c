/**
 * @file main.c
 * @brief Parpadeo del LED integrado mediante una maquina de estados PIO
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"

/* ─── Defines ──────────────────────────────────────────── */
#define LED_PIN 25

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &blink_program);

    blink_program_init(pio, sm, offset, LED_PIN);
    pio_sm_set_enabled(pio, sm, true);

    // Valor de retardo para 1 Hz, con clk_sys = 125 MHz (ver Concepto Teorico)
    uint32_t delay_value = (clock_get_hz(clk_sys) / 2) - 3;
    pio_sm_put_blocking(pio, sm, delay_value);

    while (1) {
        // El nucleo queda libre: el parpadeo lo sostiene la maquina de
        // estados PIO de manera completamente autonoma.
        tight_loop_contents();
    }
    return 0;
}

