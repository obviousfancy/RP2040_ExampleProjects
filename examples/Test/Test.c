/**
 * @file Test.c
 * @brief Proyecto Test para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-07-10
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

/* ─── Defines ──────────────────────────────────────────── */
// #define MY_PIN 0

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();
 // gpio_init(PIN);
    // gpio_set_dir(PIN, GPIO_OUT);
    stdio_init_all();
  // uint slice = pwm_gpio_to_slice_num(PIN);
    // pwm_set_enabled(slice, true);
    stdio_init_all();
  // i2c_init(i2c0, 100 * 1000);
    // gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    stdio_init_all();
    // uart_init(uart0, 115200);

    while (1) {
        // TODO: lógica principal del proyecto

    }
}
