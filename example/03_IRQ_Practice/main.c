/**
 * @file main.c
 * @brief Deteccion de flancos en un boton mediante interrupcion externa
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

/* ─── Defines ──────────────────────────────────────────── */
#define BUTTON_PIN 24

/* ─── Rutina de servicio de interrupcion (ISR) ────────────
 * Se ejecuta automaticamente cuando ocurre alguno de los
 * eventos configurados sobre el pin indicado.
 */
void boton_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        printf("Boton presionado (interrupcion en GPIO %d)\n", gpio);
    } else if (events & GPIO_IRQ_EDGE_RISE) {
        printf("Boton liberado (interrupcion en GPIO %d)\n", gpio);
    }
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {

    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &boton_callback
    );

    while (1) {
        // El nucleo queda libre para otras tareas; la deteccion
        // del boton ocurre de forma asincrona, por interrupcion.
        tight_loop_contents();

    }
}

