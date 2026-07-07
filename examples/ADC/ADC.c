/**
 * @file ADC.c
 * @brief Proyecto ADC para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-06-28
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

/* ─── Defines ──────────────────────────────────────────── */
#define ADC_PIN 26
#define LED_PIN 25
/* ─── Main ─────────────────────────────────────────────── */
int main() {
    adc_init();
    adc_gpio_init(ADC_PIN); // GP26 = ADC0
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT); // LED conectado a GP25
    adc_select_input(0);
    while (1) {
        // TODO: lógica principal del proyecto
        uint16_t raw = adc_read();  // así lo guardas para usarlo
        if(raw >= 4095/2){
            gpio_put(LED_PIN, 1); // enciende el LED
        }else{
            gpio_put(LED_PIN, 0); // apaga el LED
        }
        sleep_ms(1); // espera 1 segund
    }
    
}
