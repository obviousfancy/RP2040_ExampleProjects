/**
 * @file main.c
 * @brief Proyecto 00_Blink_Practice para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/* ─── Defines ──────────────────────────────────────────── */
#define LED_PIN 25

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);   // Enciende el LED
        sleep_ms(1000);         // Retardo de 1 segundo
        gpio_put(LED_PIN, 0);   // Apaga el LED
        sleep_ms(1000);         // Retardo de 1 segundo
    }
}
