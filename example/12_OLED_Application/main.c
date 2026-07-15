/**
 * @file main.c
 * @brief Aplicacion sensor TMP235 con impresion OLED
 *
 * @author obviousfancy
 * @date 2026-07-12
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h" 
#include <stdio.h>



/* ─── Defines ──────────────────────────────────────────── */
// ─── Configuraciones I2C y OLED ────────────────────────
#define I2C_PORT i2c0
#define I2C_SDA 0 // Pin GP4
#define I2C_SCL 1 // Pin GP5
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    stdio_init_all();

    adc_init();
    adc_set_clkdiv(0);                   // Divisor de reloj: 0 = velocidad maxima de conversion
    adc_set_temp_sensor_enabled(true);    // Habilita el canal interno de temperatura
    adc_select_input(4);                  // Selecciona el canal 4 (sensor interno)
    // 1. Inicializar I2C (400kHz es ideal para refrescar pantallas rápido)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL);

    // 2. Configurar la estructura de la pantalla
    ssd1306_t display;
    // CRÍTICO: Configurar external_vcc a false antes del init, 
    // de lo contrario la bomba de carga no enciende y la pantalla se queda negra.
    display.external_vcc = false; 

    // Inicializar el display
    bool success = ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDR, I2C_PORT);
    
    if (!success) {
        printf("Fallo al inicializar la pantalla OLED. Revisa las conexiones.\n");
        while(1) sleep_ms(1000); // Detener ejecución si falla
    }

    // 3. Limpiar el buffer de memoria
    ssd1306_clear(&display);

    // 4. Dibujar en el buffer (x, y, escala, texto)
    // El tamaño base de la fuente es de 8x5 pixeles.
    ssd1306_draw_string(&display, 0, 0, 1, "Sistema Listo");
    ssd1306_draw_string(&display, 0, 16, 2, "TMP235"); // Texto más grande
    ssd1306_draw_string(&display, 0, 40, 1, "Leyendo ADC...");

    // 5. Enviar el buffer a la pantalla para que sea visible
    ssd1306_show(&display);


    while (1) {
// --- OPCIÓN A: Sensor interno del Pico (Canal 4) ---
        uint16_t muestra = adc_read();
        float voltaje = muestra * 3.3f / 4096.0f;
        float temperatura = 27.0f - (voltaje - 0.706f) / 0.001721f;

        /* --- OPCIÓN B: Sensor físico TMP235 en GP26 (Canal 0) ---
         * Descomenta esto y cambia adc_select_input(4) por adc_select_input(0) arriba
         *
         * uint16_t muestra = adc_read();
         * float voltaje = muestra * 3.3f / 4096.0f;
         * float temperatura = (voltaje - 0.5f) * 100.0f; // Offset 500mV, 10mV/°C
         */

        ssd1306_clear(&display);
        char buffer_texto[32];
        char intro[30] = "TEMP ";
        // Imprime flotante con 1 decimal (ej. "Temp: 24.5 C")
        sprintf(buffer_texto, "Temp: %.1f C", temperatura);
        ssd1306_draw_string(&display, 40, 0, 2, intro);
        ssd1306_draw_line(&display, 0, 18, 127, 18);
        ssd1306_draw_string(&display, 28, 30, 1, buffer_texto);

        ssd1306_show(&display);

        sleep_ms(500);
    }
    return 0;
}

