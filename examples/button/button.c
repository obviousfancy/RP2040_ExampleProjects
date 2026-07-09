#include "pico/stdlib.h"
#include <stdio.h>

#define LED_PIN    25
#define BUTTON_PIN 14

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  // botón a GND: presionado = 0

    bool estado_anterior = true;  // true = no presionado (pull-up)

    while (true) {
        bool estado_actual = gpio_get(BUTTON_PIN);

        if (estado_actual != estado_anterior) {
            sleep_ms(20);  // debounce: espera a que se estabilice
            estado_actual = gpio_get(BUTTON_PIN);

            if (estado_actual != estado_anterior) {
                gpio_put(LED_PIN, !estado_actual);
                printf("Boton %s\n", estado_actual ? "liberado" : "presionado");
                estado_anterior = estado_actual;
            }
        }
    }
}