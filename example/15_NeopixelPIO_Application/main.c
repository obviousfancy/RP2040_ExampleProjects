/**
 * @file main.c
 * @brief Control de una tira de LEDs direccionables WS2812/Neopixel mediante
 *        PIO puro, sin DMA: la CPU escribe cada pixel al FIFO de la maquina
 *        de estados con pio_sm_put_blocking.
 *
 * @author obviousfancy
 * @date 2026-07-13
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

/* ─── Defines ──────────────────────────────────────────── */
#define WS2812_PIN   16      // GPIO conectado a DIN de la tira
#define NUM_LEDS     8       // Cantidad de LEDs en la tira
#define WS2812_FREQ  800000  // Frecuencia de bit del protocolo (800 kHz)

/* ─── Prototipos ───────────────────────────────────────── */
static inline void put_pixel(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b);

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, WS2812_FREQ);

    uint8_t hue = 0;

    while (1) {
        // Efecto arcoiris simple: cada LED se desplaza en tono respecto al anterior.
        // Sin DMA, el nucleo se ocupa brevemente enviando los NUM_LEDS pixeles;
        // pio_sm_put_blocking solo bloquea si el FIFO (4 words) esta lleno.
        for (int i = 0; i < NUM_LEDS; i++) {
            uint8_t phase = hue + (uint8_t)(i * (256 / NUM_LEDS));
            uint8_t r, g, b;

            if (phase < 85) {
                r = 255 - phase * 3; g = phase * 3;       b = 0;
            } else if (phase < 170) {
                phase -= 85;
                r = 0;               g = 255 - phase * 3; b = phase * 3;
            } else {
                phase -= 170;
                r = phase * 3;       g = 0;                b = 255 - phase * 3;
            }

            put_pixel(pio, sm, r, g, b);
        }

        hue++;
        sleep_ms(30);
    }
    return 0;
}

/* ─── Funciones auxiliares ─────────────────────────────── */

// Empaqueta un color en el orden GRB de 24 bits que espera el WS2812 y lo
// envia al FIFO de la maquina de estados (bloqueante si esta lleno).
static inline void put_pixel(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
    pio_sm_put_blocking(pio, sm, grb << 8u);
}
