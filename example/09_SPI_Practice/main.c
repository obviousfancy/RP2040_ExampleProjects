/**
 * @file main.c
 * @brief Verificacion de comunicacion SPI con el sensor BMI270 (lectura de CHIP_ID)
 *
 * @author obviousfancy
 * @date 2026-07-14
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

/* ─── Defines ──────────────────────────────────────────── */
#define SPI_PORT   spi0
#define PIN_SCK    2
#define PIN_MOSI   3
#define PIN_MISO   4
#define PIN_CS     5

#define BMI270_CHIP_ID_REG    0x00
#define BMI270_CMD_REG        0x7E
#define BMI270_CMD_SOFT_RESET 0xB6
#define BMI270_CHIP_ID_VALUE  0x24


/* ─── Lectura de un registro del BMI270 por SPI ───────────
 * El BMI270 requiere, en cada lectura por SPI: byte de direccion
 * (con bit 7 en 1), un byte dummy adicional, y recien despues el
 * dato real. El dato valido llega en rx_buf[2], no en rx_buf[1].
 */
static uint8_t bmi270_leer_registro(uint8_t reg) {
    uint8_t tx_buf[3] = { reg | 0x80, 0x00, 0x00 };  // bit 7 en 1 indica lectura (convencion del BMI270)
    uint8_t rx_buf[3] = { 0 };

    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(SPI_PORT, tx_buf, rx_buf, 3);
    gpio_put(PIN_CS, 1);

    return rx_buf[2];
}

/* ─── Escritura de un registro del BMI270 por SPI ───────── */
static void bmi270_escribir_registro(uint8_t reg, uint8_t valor) {
    uint8_t tx_buf[2] = { reg, valor };

    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, tx_buf, 2);
    gpio_put(PIN_CS, 1);
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    // CS se configura y se fuerza a bajo antes de cualquier otra
    // inicializacion, en linea con la recomendacion del fabricante del
    // modulo sobre la seleccion de interfaz del BMI270 al energizarse.
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0);

    stdio_init_all();

    uint baud_real = spi_init(SPI_PORT, 1000 * 1000);  // 1 MHz

    // El periferico SPI opera en modo maestro por defecto tras spi_init();
    // no es necesario llamar a spi_set_slave(), reservada para cuando el
    // RP2040 deba operar como esclavo de otro maestro (no es el caso aqui).

    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    // CS ya se inicializo como salida y en bajo al inicio de main(); con el
    // resto del bus configurado, se libera a su nivel de reposo (alto,
    // activo en bajo).
    gpio_put(PIN_CS, 1);

    printf("Frecuencia SPI solicitada: 1000000 Hz, real: %u Hz\n", baud_real);
    sleep_ms(10);

    // El BMI270 arranca configurado para I2C; una primera lectura por SPI
    // (cuyo resultado se descarta) es necesaria para forzar la seleccion
    // de la interfaz SPI, conforme al procedimiento de arranque del fabricante.
    bmi270_leer_registro(BMI270_CHIP_ID_REG);
    sleep_ms(1);

    // Soft reset para partir de un estado conocido. Tras el reset el
    // sensor vuelve a modo I2C, por lo que se repite la lectura dummy.
    bmi270_escribir_registro(BMI270_CMD_REG, BMI270_CMD_SOFT_RESET);
    sleep_ms(2);
    bmi270_leer_registro(BMI270_CHIP_ID_REG);
    sleep_ms(1);

    while (1) {
        uint8_t chip_id = bmi270_leer_registro(BMI270_CHIP_ID_REG);

        if (chip_id == BMI270_CHIP_ID_VALUE) {
            printf("BMI270 detectado correctamente. CHIP_ID = 0x%02X\n", chip_id);
        } else {
            printf("BMI270 no responde como se esperaba. CHIP_ID leido = 0x%02X (esperado 0x%02X)\n",
                   chip_id, BMI270_CHIP_ID_VALUE);
        }

        sleep_ms(1000);
    }
    return 0;
}
