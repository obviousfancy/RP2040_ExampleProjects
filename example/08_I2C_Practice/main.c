/**
 * @file main.c
 * @brief Escaneo de direcciones en el bus I2C
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
#include "hardware/i2c.h"

/* ─── Defines ──────────────────────────────────────────── */
#define I2C_SDA  0
#define I2C_SCL  1
#define I2C_PORT i2c0
#define I2C_FREQ 400000


/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();

    uint freq_real = i2c_init(I2C_PORT, I2C_FREQ);

    // El periferico opera en modo maestro por defecto; no es necesario
    // llamar a i2c_set_slave_mode(), reservada para cuando el RP2040
    // deba responder como esclavo ante otro maestro (no es el caso aqui).

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("Frecuencia solicitada: %d Hz, frecuencia real: %u Hz\n", I2C_FREQ, freq_real);
    sleep_ms(2000);  // Margen para abrir la terminal antes del primer escaneo

    while (1) {
        printf("\nEscaneando bus I2C...\n");
        int dispositivos = 0;

        for (uint8_t addr = 0x08; addr < 0x78; addr++) {
            uint8_t rxdata;
            int resultado = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);

            if (resultado >= 0) {
                printf("Dispositivo encontrado en 0x%02X\n", addr);
                dispositivos++;
            }
        }

        if (dispositivos == 0) {
            printf("Ningun dispositivo respondio en el bus\n");
        }

        sleep_ms(5000);
    }
    return 0;
}

