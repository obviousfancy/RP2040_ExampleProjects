/**
 * @file main.c
 * @author obviousfancy
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// --- Asignacion de pines ---
#define ADC_PIN 26
#define ADC_CHANNEL 0
#define PWM_PIN 16
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define OLED_ADDRESS 0x3C

// --- Parametros de PWM (f_pwm = 1 kHz exacto) ---
#define PWM_WRAP 999
#define PWM_CLKDIV 125.0f

// --- Referencias del ADC ---
#define ADC_MAX 4095.0f
#define ADC_VREF 3.3f

// --- Tasa de refresco del OLED, independiente del bucle principal ---
#define OLED_REFRESH_MS 150

int main(void) {
    stdio_init_all();

    // --- Inicializacion ADC ---
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);

    // --- Inicializacion PWM ---
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_CLKDIV);
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_init(slice_num, &config, true);

    // --- Inicializacion I2C y OLED ---
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t oled;
    oled.external_vcc = false;
    ssd1306_init(&oled, 128, 64, OLED_ADDRESS, I2C_PORT);
    ssd1306_clear(&oled);

    absolute_time_t next_refresh = get_absolute_time();
    char line_raw[24];
    char line_pct[24];
    char line_volt[24];

    while (1) {
        // 1. Lectura del ADC (0 - 4095)
        uint16_t adc_raw = adc_read();

        // 2. Reescalamiento de rango: 4096 niveles de ADC a (wrap + 1) niveles de PWM
        uint16_t duty = (uint32_t)(adc_raw * PWM_WRAP) / (uint32_t)ADC_MAX;

        // 3. Aplicacion del ciclo de trabajo
        pwm_set_gpio_level(PWM_PIN, duty);

        // 4. Actualizacion del OLED a tasa independiente y mas lenta
        if (absolute_time_diff_us(get_absolute_time(), next_refresh) <= 0) {
            float percentage = (adc_raw * 100.0f) / ADC_MAX;
            float voltage = (adc_raw * ADC_VREF) / ADC_MAX;

            snprintf(line_raw, sizeof(line_raw), "Raw:  %u", adc_raw);
            snprintf(line_pct, sizeof(line_pct), "Pct:  %.1f %%", percentage);
            snprintf(line_volt, sizeof(line_volt), "Volt: %.2f V", voltage);

            ssd1306_clear(&oled);
            ssd1306_draw_string(&oled, 34, 0, 1, "ADC - PWM");
            ssd1306_draw_line(&oled, 0, 10, 127, 10);
            ssd1306_draw_string(&oled, 0, 20, 1, line_raw);
            ssd1306_draw_string(&oled, 0, 34, 1, line_pct);
            ssd1306_draw_string(&oled, 0, 48, 1, line_volt);
            ssd1306_show(&oled);

            next_refresh = make_timeout_time_ms(OLED_REFRESH_MS);
        }

        // Ritmo de muestreo del par ADC-PWM (mas rapido que el refresco del OLED)
        sleep_ms(15);
    }

    return 0;
}