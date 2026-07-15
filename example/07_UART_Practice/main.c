/**
 * @file main.c
 * @brief Eco por UART0 hacia el conversor USB-serial CH340 del Shield
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
#include "hardware/uart.h"

/* ─── Defines ──────────────────────────────────────────── */
#define UART_ID    uart0
#define BAUDRATE   115200
#define TX_PIN     0
#define RX_PIN     1

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();
    uint baud_real = uart_init(UART_ID, BAUDRATE);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);  // 8N1
    uart_set_fifo_enabled(UART_ID, true);

    printf("Baudrate solicitado: %d, baudrate real: %u\n", BAUDRATE, baud_real);
    printf("Esperando caracteres desde el CH340...\n");

    while (1) {
        if (uart_is_readable(UART_ID)) {
            char recibido = uart_getc(UART_ID);
            uart_putc(UART_ID, recibido);                     // Eco de vuelta hacia el CH340
            printf("Recibido por UART0: '%c'\n", recibido);    // Reporte por USB
        }
    }
    return 0;
}

