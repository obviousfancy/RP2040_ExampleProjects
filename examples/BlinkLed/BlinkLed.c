/**
 * @file BlinkLed.c
 * @brief Proyecto BlinkLed para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-06-29
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

/* ─── Defines ──────────────────────────────────────────── */
// #define MY_PIN 0

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    // gpio_init(PIN);
    // gpio_set_dir(PIN, GPIO_OUT);
    // uint slice = pwm_gpio_to_slice_num(PIN);
    // pwm_set_enabled(slice, true);
    // i2c_init(i2c0, 100 * 1000);
    // gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    while (1) {
        // TODO: lógica principal del proyecto

    }
}
