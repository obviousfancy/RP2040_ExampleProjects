/**
 * @file I2C.c
 * @brief Proyecto I2C para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-07-08
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* ─── Defines ──────────────────────────────────────────── */
#define I2C_PORT     i2c1
#define I2C_SDA_PIN  2 //4
#define I2C_SCL_PIN  3 //5
#define OLED_ADDR    0x3C
#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   (OLED_HEIGHT / 8)

static const uint8_t ssd1306_init_cmds[] = {
    0xAE,
    0xD5, 0x80,
    0xA8, 0x3F,
    0xD3, 0x00,
    0x40,
    0x8D, 0x14,
    0x20, 0x00,
    0xA1,
    0xC8,
    0xDA, 0x12,
    0x81, 0xCF,
    0xD9, 0xF1,
    0xDB, 0x40,
    0xA4,
    0xA6,
    0xAF
};

static void oled_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(I2C_PORT, OLED_ADDR, buf, 2, false);
}

static void oled_fill_pattern(uint8_t pattern) {
    uint8_t page_buf[OLED_WIDTH + 1];
    page_buf[0] = 0x40;
    for (int p = 0; p < OLED_PAGES; p++) {
        oled_send_cmd(0xB0 + p);
        oled_send_cmd(0x00);
        oled_send_cmd(0x10);
        for (int c = 1; c <= OLED_WIDTH; c++) page_buf[c] = pattern;
        i2c_write_blocking(I2C_PORT, OLED_ADDR, page_buf, OLED_WIDTH + 1, false);
    }
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();
    
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    sleep_ms(100);

    for (size_t i = 0; i < sizeof(ssd1306_init_cmds); i++) {
        oled_send_cmd(ssd1306_init_cmds[i]);
    }
    oled_fill_pattern(0xAA);

    bool inverted = false;

    while (1) {
        // TODO: lógica principal del proyecto
        sleep_ms(1000);
        inverted = !inverted;
        oled_send_cmd(inverted ? 0xA7 : 0xA6);
        printf("OLED: comando de %s enviado via Pico SDK (C)\n",
               inverted ? "inversion" : "normalizacion");
    }
}
