/**
 * @file main.c
 * @brief Proyecto 13_OLEDSPIBMI270_Application para Raspberry Pi Pico
 *
 * Integracion del sensor inercial BMI270 (SPI) con la pantalla OLED
 * SSD1306 (I2C): se inicializa el sensor con su archivo de configuracion
 * oficial, se leen aceleracion y velocidad angular en los tres ejes, y
 * los valores se despliegan en la pantalla y en la consola serial.
 *
 * @author obviousfancy
 * @date 2026-07-13
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "hardware/spi.h"
#include "bmi270_config.h"   // Archivo de configuracion oficial de Bosch (8192 bytes, BSD-3-Clause)

/* ─── Defines ──────────────────────────────────────────── */
// ─── Configuraciones I2C y OLED ────────────────────────
#define I2C_PORT i2c0
#define I2C_SDA 0 // Pin GP0
#define I2C_SCL 1 // Pin GP1
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// ─── Configuraciones SPI y BMI270 ──────────────────────
#define SPI_PORT   spi0
#define PIN_SCK    2
#define PIN_MOSI   3
#define PIN_MISO   4
#define PIN_CS     5

// Registros del BMI270 utilizados en esta aplicacion
#define BMI270_REG_CHIP_ID          0x00
#define BMI270_REG_DATA_ACC_X_LSB   0x0C  // Inicio del bloque de datos: ACC XYZ + GYR XYZ (12 bytes)
#define BMI270_REG_INTERNAL_STATUS  0x21
#define BMI270_REG_ACC_CONF         0x40
#define BMI270_REG_ACC_RANGE        0x41
#define BMI270_REG_GYR_CONF         0x42
#define BMI270_REG_GYR_RANGE        0x43
#define BMI270_REG_INIT_CTRL        0x59
#define BMI270_REG_INIT_ADDR_0      0x5B
#define BMI270_REG_INIT_ADDR_1      0x5C
#define BMI270_REG_INIT_DATA        0x5E
#define BMI270_REG_CMD              0x7E
#define BMI270_REG_PWR_CONF         0x7C
#define BMI270_REG_PWR_CTRL         0x7D

#define BMI270_CHIP_ID_VALUE        0x24
#define BMI270_CMD_SOFT_RESET       0xB6

// Rangos configurados y sus factores de conversion.
// Los datos crudos son enteros con signo de 16 bits: el fondo de escala
// corresponde a 32768 cuentas, por lo que 1 cuenta = rango / 32768.
#define ACC_RANGE_G      2.0f     // ACC_RANGE = 0x00 -> +/-2 g
#define GYR_RANGE_DPS    2000.0f  // GYR_RANGE = 0x00 -> +/-2000 grados/s

/* ─── Funciones de acceso al BMI270 por SPI ────────────── */

// Lectura de un solo registro. El BMI270 requiere, en cada lectura por
// SPI: byte de direccion (con bit 7 en 1), un byte dummy adicional, y
// recien despues el dato real. El dato valido llega en rx_buf[2].
static uint8_t bmi270_leer_registro(uint8_t reg) {
    uint8_t tx_buf[3] = { reg | 0x80, 0x00, 0x00 };
    uint8_t rx_buf[3] = { 0 };

    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(SPI_PORT, tx_buf, rx_buf, 3);
    gpio_put(PIN_CS, 1);

    return rx_buf[2];
}

// Lectura en rafaga de varios registros consecutivos. El byte dummy
// aparece una sola vez por transaccion (tras el byte de direccion);
// los datos siguientes llegan de forma continua.
static void bmi270_leer_rafaga(uint8_t reg, uint8_t *datos, size_t n) {
    uint8_t tx = reg | 0x80;
    uint8_t descarte;

    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &tx, 1);        // direccion
    spi_read_blocking(SPI_PORT, 0x00, &descarte, 1); // byte dummy
    spi_read_blocking(SPI_PORT, 0x00, datos, n);     // datos reales
    gpio_put(PIN_CS, 1);
}

// Escritura de un solo registro. En escritura no existe byte dummy:
// direccion (bit 7 en 0) seguida inmediatamente del dato.
static void bmi270_escribir_registro(uint8_t reg, uint8_t valor) {
    uint8_t tx_buf[2] = { reg, valor };

    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, tx_buf, 2);
    gpio_put(PIN_CS, 1);
}

// Escritura en rafaga (usada para cargar el archivo de configuracion).
static void bmi270_escribir_rafaga(uint8_t reg, const uint8_t *datos, size_t n) {
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_write_blocking(SPI_PORT, datos, n);
    gpio_put(PIN_CS, 1);
}

/* ─── Inicializacion del BMI270 ────────────────────────── */

// Secuencia de arranque conforme al datasheet de Bosch (seccion de
// inicializacion). El sensor no produce datos de aceleracion ni giro
// hasta completar la carga del archivo de configuracion de 8192 bytes.
// Devuelve true si la inicializacion fue exitosa.
static bool bmi270_init(void) {
    // 1. Lectura dummy de CHIP_ID: la primera transaccion por SPI tras el
    //    encendido activa la interfaz SPI del sensor (arranca en modo I2C).
    (void)bmi270_leer_registro(BMI270_REG_CHIP_ID);
    sleep_ms(1);

    // 2. Soft reset para partir de un estado conocido. Tras el reset el
    //    sensor vuelve a modo I2C, por lo que se repite la lectura dummy.
    bmi270_escribir_registro(BMI270_REG_CMD, BMI270_CMD_SOFT_RESET);
    sleep_ms(2);
    (void)bmi270_leer_registro(BMI270_REG_CHIP_ID);
    sleep_ms(1);

    // 3. Verificacion de identidad.
    uint8_t chip_id = bmi270_leer_registro(BMI270_REG_CHIP_ID);
    if (chip_id != BMI270_CHIP_ID_VALUE) {
        printf("BMI270: CHIP_ID incorrecto (0x%02X, esperado 0x%02X)\n",
               chip_id, BMI270_CHIP_ID_VALUE);
        return false;
    }

    // 4. Deshabilitar el modo de ahorro avanzado de energia; el sensor
    //    arranca con el en 1 y en ese estado la carga de configuracion
    //    no es posible. Espera minima de 450 us tras el cambio.
    bmi270_escribir_registro(BMI270_REG_PWR_CONF, 0x00);
    sleep_us(500);

    // 5. Preparar la carga: INIT_CTRL = 0x00.
    bmi270_escribir_registro(BMI270_REG_INIT_CTRL, 0x00);

    // 6. Cargar el archivo de configuracion en bloques. Antes de cada
    //    bloque se escribe la direccion destino (en palabras de 16 bits)
    //    en INIT_ADDR_0 (4 bits bajos) e INIT_ADDR_1 (8 bits altos).
    const size_t TAM_BLOQUE = 256;
    for (size_t offset = 0; offset < sizeof(bmi270_config_file); offset += TAM_BLOQUE) {
        uint16_t addr_palabras = (uint16_t)(offset / 2);
        bmi270_escribir_registro(BMI270_REG_INIT_ADDR_0, addr_palabras & 0x0F);
        bmi270_escribir_registro(BMI270_REG_INIT_ADDR_1, (addr_palabras >> 4) & 0xFF);
        bmi270_escribir_rafaga(BMI270_REG_INIT_DATA,
                               &bmi270_config_file[offset], TAM_BLOQUE);
    }

    // 7. Finalizar la carga: INIT_CTRL = 0x01. Esta escritura no debe
    //    realizarse mas de una vez por ciclo de encendido o soft reset.
    bmi270_escribir_registro(BMI270_REG_INIT_CTRL, 0x01);

    // 8. Esperar a que INTERNAL_STATUS reporte inicializacion correcta
    //    (message = 0b0001). El datasheet indica un maximo de 20 ms;
    //    se sondea con margen amplio.
    bool listo = false;
    for (int i = 0; i < 100; i++) {
        uint8_t estado = bmi270_leer_registro(BMI270_REG_INTERNAL_STATUS);
        if ((estado & 0x0F) == 0x01) {
            listo = true;
            break;
        }
        sleep_ms(1);
    }
    if (!listo) {
        printf("BMI270: la carga de configuracion no finalizo correctamente\n");
        return false;
    }

    // 9. Habilitar acelerometro, giroscopio y sensor de temperatura
    //    (PWR_CTRL = 0x0E). El bit de interfaz auxiliar permanece en 0:
    //    no hay sensor externo conectado al hub del BMI270.
    bmi270_escribir_registro(BMI270_REG_PWR_CTRL, 0x0E);

    // 10. Configuracion de ambos sensores en modo de rendimiento:
    //     ACC_CONF = 0xA8 -> ODR 100 Hz, filtro en modo rendimiento.
    //     GYR_CONF = 0xA9 -> ODR 200 Hz, filtro en modo rendimiento.
    bmi270_escribir_registro(BMI270_REG_ACC_CONF, 0xA8);
    bmi270_escribir_registro(BMI270_REG_GYR_CONF, 0xA9);

    // 11. Rangos explicitos, para que la conversion a unidades fisicas
    //     sea inequivoca: +/-2 g y +/-2000 grados/s.
    bmi270_escribir_registro(BMI270_REG_ACC_RANGE, 0x00);
    bmi270_escribir_registro(BMI270_REG_GYR_RANGE, 0x00);

    // 12. PWR_CONF = 0x02: ahorro avanzado deshabilitado (modo de
    //     rendimiento continuo), fifo_self_wake_up habilitado (valor
    //     recomendado por el procedimiento de arranque del fabricante).
    bmi270_escribir_registro(BMI270_REG_PWR_CONF, 0x02);
    sleep_ms(10);

    return true;
}

// Lectura de los 6 valores crudos (ACC XYZ + GYR XYZ) en una sola rafaga
// de 12 bytes a partir de 0x0C. Cada valor es de 16 bits, LSB primero.
static void bmi270_leer_datos(int16_t acc[3], int16_t gyr[3]) {
    uint8_t crudo[12];
    bmi270_leer_rafaga(BMI270_REG_DATA_ACC_X_LSB, crudo, sizeof(crudo));

    acc[0] = (int16_t)((crudo[1] << 8) | crudo[0]);
    acc[1] = (int16_t)((crudo[3] << 8) | crudo[2]);
    acc[2] = (int16_t)((crudo[5] << 8) | crudo[4]);
    gyr[0] = (int16_t)((crudo[7] << 8) | crudo[6]);
    gyr[1] = (int16_t)((crudo[9] << 8) | crudo[8]);
    gyr[2] = (int16_t)((crudo[11] << 8) | crudo[10]);
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    // CS se configura y se fuerza a bajo antes de cualquier otra
    // inicializacion, en linea con la recomendacion del fabricante del
    // modulo sobre la seleccion de interfaz del BMI270 al energizarse.
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0);

    stdio_init_all();

    // Retardo de arranque: da tiempo a que el host establezca la conexion
    // USB CDC antes de emitir cualquier printf.
    sleep_ms(10000);

    // 1. Inicializar I2C (400 kHz permite un refresco agil de la pantalla)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // 2. Inicializar SPI a 1 MHz, modo 0 (CPOL=0, CPHA=0), MSB primero
    uint baud_real = spi_init(SPI_PORT, 1000 * 1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    // CS ya se inicializo como salida al inicio de main(); configurado el
    // resto del bus, se libera a su nivel de reposo (alto, activo en bajo).
    gpio_put(PIN_CS, 1);

    printf("Frecuencia SPI solicitada: 1000000 Hz, real: %u Hz\n", baud_real);

    // 3. Inicializar la pantalla OLED
    ssd1306_t display;
    // CRITICO: configurar external_vcc a false antes del init; de lo
    // contrario la bomba de carga no enciende y la pantalla queda negra.
    display.external_vcc = false;

    bool oled_ok = ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDR, I2C_PORT);
    if (!oled_ok) {
        printf("Fallo al inicializar la pantalla OLED. Revisa las conexiones.\n");
        while (1) sleep_ms(1000);
    }
    printf("Pantalla OLED inicializada correctamente.\n");

    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, 1, "Iniciando BMI270...");
    ssd1306_show(&display);

    // 4. Inicializar el BMI270 (carga de configuracion incluida)
    if (!bmi270_init()) {
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, "Error BMI270");
        ssd1306_draw_string(&display, 0, 16, 1, "Revisa conexiones");
        ssd1306_show(&display);
        while (1) sleep_ms(1000);
    }
    printf("BMI270 inicializado correctamente.\n");

    // 5. Bucle principal: leer, convertir, desplegar
    int16_t acc_crudo[3], gyr_crudo[3];
    char linea[24];

    while (1) {
        bmi270_leer_datos(acc_crudo, gyr_crudo);

        // Conversion a unidades fisicas segun los rangos configurados
        float ax = acc_crudo[0] * ACC_RANGE_G / 32768.0f;
        float ay = acc_crudo[1] * ACC_RANGE_G / 32768.0f;
        float az = acc_crudo[2] * ACC_RANGE_G / 32768.0f;
        float gx = gyr_crudo[0] * GYR_RANGE_DPS / 32768.0f;
        float gy = gyr_crudo[1] * GYR_RANGE_DPS / 32768.0f;
        float gz = gyr_crudo[2] * GYR_RANGE_DPS / 32768.0f;

        // Despliegue en OLED: acelerometro arriba, giroscopio abajo.
        // Fuente base 5x8: escala 1 permite ~21 caracteres por linea.
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, "Acel [g]");
        snprintf(linea, sizeof(linea), "X%+.2f Y%+.2f", ax, ay);
        ssd1306_draw_string(&display, 0, 10, 1, linea);
        snprintf(linea, sizeof(linea), "Z%+.2f", az);
        ssd1306_draw_string(&display, 0, 20, 1, linea);

        ssd1306_draw_string(&display, 0, 34, 1, "Giro [dps]");
        snprintf(linea, sizeof(linea), "X%+.1f Y%+.1f", gx, gy);
        ssd1306_draw_string(&display, 0, 44, 1, linea);
        snprintf(linea, sizeof(linea), "Z%+.1f", gz);
        ssd1306_draw_string(&display, 0, 54, 1, linea);
        ssd1306_show(&display);

        // Despliegue en consola serial
        printf("Acel [g]: X=%+.3f Y=%+.3f Z=%+.3f | Giro [dps]: X=%+.2f Y=%+.2f Z=%+.2f\n",
               ax, ay, az, gx, gy, gz);

        // ~10 Hz de refresco: suficiente para visualizacion y muy por
        // debajo del costo de refresco I2C del OLED (~23 ms por cuadro).
        sleep_ms(100);
    }
    return 0;
}
