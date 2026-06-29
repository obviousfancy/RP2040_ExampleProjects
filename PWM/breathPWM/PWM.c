/**
 * @file PWM.c
 * @brief Proyecto PWM para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-06-28
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <math.h>

/* ─── Defines ──────────────────────────────────────────── */
#define PWM_LED_PIN 25
#define GAMMA 2.2f

uint8_t gamma_table[256];


/* ─── Functions ───────────────────────────────────────────── */
void gamma_init() {
    for(int i = 0; i < 256; i++) {
        float normal = i / 255.0f;
        float gamma_val = powf(normal,GAMMA);
        gamma_table[i] = (uint8_t)(gamma_val * 255.0f);
    }
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {

    gamma_init();
    uint slice_num = pwm_gpio_to_slice_num(PWM_LED_PIN);   // devuelve el número de slice (0-7)
    uint channel = pwm_gpio_to_channel(PWM_LED_PIN);    // devuelve el canal (PWM_CHAN_A o PWM_CHAN_B)
    gpio_set_function(PWM_LED_PIN,GPIO_FUNC_PWM); // Configura el pin como PWM
    pwm_set_wrap(slice_num, 255); // Establece el número de ciclos en cada período
    pwm_set_chan_level(slice_num, channel, 0); // Establece el nivel de amplitud (0-65535)
    pwm_set_enabled(slice_num, true); // Habilita el PWM

    while (1) {
        // TODO: lógica principal del proyecto
        for(int i = 0; i <= 255; i++){
            pwm_set_chan_level(slice_num, channel, gamma_table[i]);
            sleep_ms(8);
        }
        for(int j = 255; j >= 0; j--){
            pwm_set_chan_level(slice_num, channel, gamma_table[j]);
            sleep_ms(8);
        }
  
    }
    return 0;
}
