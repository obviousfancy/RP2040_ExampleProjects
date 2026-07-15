/**
 * @file main.c
 * @brief Lectura del sensor de temperatura interno mediante el ADC
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
#include "hardware/adc.h"

/* ─── Defines ──────────────────────────────────────────── */
// #define MY_PIN 0

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();

    adc_init();
    adc_set_clkdiv(0);                   // Divisor de reloj: 0 = velocidad maxima de conversion
    adc_set_temp_sensor_enabled(true);    // Habilita el canal interno de temperatura
    adc_select_input(4);                  // Selecciona el canal 4 (sensor interno)
 
    // Si en cambio se empleara un canal externo (0 a 3, asociado a un pin
    // fisico), seria indispensable inicializar antes ese pin para uso
    // analogico, deshabilitando sus resistencias de pull-up/pull-down y
    // su buffer digital:
    // adc_gpio_init(26);  // GPIO26 corresponde al canal 0
 
    // El periferico admite recorrer varios canales de manera automatica
    // (round-robin) en lugar de seleccionarlos uno por uno; no es
    // necesario aqui porque solo se utiliza un canal:
    // adc_set_round_robin(0x1F);  // ejemplo: recorreria los canales 0 a 4
 
    // Las conversiones tambien pueden acumularse en una FIFO interna en
    // lugar de leerse una por una con adc_read(); no es necesario aqui:
    // adc_fifo_setup(true, false, 1, false, false);
 
    // El periferico puede generar una interrupcion al completar cada
    // conversion, en lugar de que el programa consulte el resultado
    // directamente; no es necesario aqui:
    // adc_irq_set_enabled(false);
 
    while (1) {
        uint16_t muestra = adc_read();
        float voltaje = muestra * 3.3f / 4096.0f;
        float temperatura = 27.0f - (voltaje - 0.706f) / 0.001721f;

        printf("Temperatura: %.2f C\n", temperatura);
        sleep_ms(1000);
    }
    return 0;
}

