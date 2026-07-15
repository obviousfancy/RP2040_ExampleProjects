/**
 * @file main.c
 * @brief Atenuacion progresiva del LED integrado mediante PWM
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/pwm.h"

/* ─── Defines ──────────────────────────────────────────── */
#define LED_PIN 25

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);

    uint slice   = pwm_gpio_to_slice_num(LED_PIN);
    uint channel = pwm_gpio_to_channel(LED_PIN);

    // Configuracion completa del slice, aunque esta practica solo
    // varie el nivel de un canal de forma gradual:
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 100.0f);                  // Divisor de reloj
    pwm_config_set_wrap(&config, 255);                        // Periodo del contador: 256 pasos (0-255)
    pwm_config_set_phase_correct(&config, false);             // Conteo simple (no simetrico)
    pwm_config_set_output_polarity(&config, false, false);    // Polaridad normal en ambos canales

    pwm_init(slice, &config, true);  // true = arrancar de inmediato

    int nivel = 0;
    int paso = 5;


    while (1) {
        // TODO: lógica principal del proyecto
        pwm_set_chan_level(slice, channel, (uint16_t)nivel);

        nivel += paso;
        if (nivel >= 255) { nivel = 255; paso = -paso; }
        else if (nivel <= 0) { nivel = 0; paso = -paso; }

        sleep_ms(15);
    }
    return 0;
}

