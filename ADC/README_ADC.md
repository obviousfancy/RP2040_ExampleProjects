# Práctica 06 — ADC: Conversor Analógico-Digital
### Raspberry Pi Pico SDK 2.2.0 — RP2040

> Lectura de señales analógicas mediante el ADC del RP2040.
> Control de un LED digital en función del voltaje leído en un potenciómetro.
> Esta práctica introduce la conversión analógico-digital, canales ADC,
> y el mapeo de valores crudos a voltaje real.

---

## Objetivo

Leer el voltaje variable de un potenciómetro conectado a GP26 (ADC0)
y usar ese valor para controlar el estado de un LED — encendido cuando
el voltaje supera la mitad del rango (≈1.65V) y apagado cuando está por debajo.

Conceptos aprendidos:
- Arquitectura del ADC del RP2040 (12 bits, 5 canales)
- Diferencia entre señal analógica y digital
- Conversión de lectura cruda (0–4095) a voltaje real
- Qué GPIOs tienen capacidad ADC y por qué
- Inicialización de múltiples GPIOs — patrones y opciones

---

## El ADC del RP2040 — arquitectura

El RP2040 tiene un conversor analógico-digital de **12 bits** con **5 canales**:

```
ADC0 → GP26 (Pin 31)  ← disponible para uso general
ADC1 → GP27 (Pin 32)  ← compartido con SWDCLK en algunos boards
ADC2 → GP28 (Pin 34)  ← compartido con SWDIO en algunos boards
ADC3 → GP29 (Pin 35)  ← usado internamente para medir VSYS
ADC4 → sensor de temperatura interno del chip (sin pin externo)
```

> **Importante:** solo GP26, GP27, GP28 y GP29 tienen capacidad ADC.
> Intentar usar `adc_gpio_init()` con cualquier otro GPIO no funcionará.
> Para esta práctica se usa **GP26 (ADC0)** — está libre en todos los boards.

---

## ¿Qué significa 12 bits?

El ADC convierte un voltaje analógico continuo (0V a 3.3V) en un número
entero discreto de 12 bits:

```
Resolución = 2^12 = 4096 niveles  (valores de 0 a 4095)

Voltaje → Lectura cruda:
  0.0V  → 0
  1.65V → 2047   (mitad del rango)
  3.3V  → 4095   (máximo)

Fórmula para convertir lectura cruda a voltaje real:
  V = (raw / 4095.0) × 3.3

Ejemplo: raw = 2048
  V = (2048 / 4095.0) × 3.3 = 1.651V

Resolución mínima (1 LSB):
  V_lsb = 3.3 / 4095 ≈ 0.000806V ≈ 0.8 mV por count
```

---

## Hardware necesario

- Raspberry Pi Pico (RP2040)
- Potenciómetro de 10kΩ
- CH552 Multiprotocol CMSIS-DAP (flasheo por SWD)
- Cables dupont

### Conexión del potenciómetro

```
Pin izquierdo  (extremo) → 3.3V del Pico
Pin central    (cursor)  → GP26 (ADC0)
Pin derecho    (extremo) → GND

     3.3V
      │
    ┌─┤ potenciómetro 10kΩ
    │ ├──────── GP26 (ADC0)
    └─┤
      │
     GND
```

Girando el potenciómetro varías el voltaje en GP26 de 0V a 3.3V,
que el ADC convierte a un valor de 0 a 4095.

### Conexión CH552 → Pico (SWD)

| CH552 | Pin Pico | Función |
|---|---|---|
| SWDIO | Pin 34 | Data |
| SWDCLK | Pin 32 | Clock |
| GND | GND | Tierra común |

---

## Librerías del SDK utilizadas

### `hardware/adc.h`
Control del periférico ADC del RP2040.

| Función | Descripción |
|---|---|
| `adc_init()` | Inicializa el periférico ADC global |
| `adc_gpio_init(gpio)` | Configura el GPIO como entrada analógica |
| `adc_select_input(channel)` | Selecciona el canal a leer (0–4) |
| `adc_read()` | Realiza una lectura y retorna `uint16_t` (0–4095) |

### `hardware/gpio.h`
```c
gpio_init(gpio)              // inicializa el pin
gpio_set_dir(gpio, dir)      // configura dirección
gpio_put(gpio, value)        // escribe valor digital
```

---

## Inicialización de múltiples GPIOs

`gpio_init()` solo acepta un pin a la vez. Para múltiples pines hay tres opciones:

### Opción A — Llamadas individuales (más común y legible)
```c
gpio_init(LED_PIN);
gpio_init(BUTTON_PIN);
gpio_set_dir(LED_PIN,    GPIO_OUT);
gpio_set_dir(BUTTON_PIN, GPIO_IN);
```

### Opción B — `gpio_init_mask()` con bitmask
```c
// Inicializa GP24 y GP25 simultáneamente
gpio_init_mask((1 << 24) | (1 << 25));
// La dirección se sigue configurando pin por pin
gpio_set_dir(25, GPIO_OUT);
gpio_set_dir(24, GPIO_IN);
```

### Opción C — Función de setup propia (patrón profesional)
```c
void gpio_setup(void) {
    const uint outputs[] = {LED_PIN, BUZZER_PIN};
    for (int i = 0; i < 2; i++) {
        gpio_init(outputs[i]);
        gpio_set_dir(outputs[i], GPIO_OUT);
        gpio_put(outputs[i], 0);  // estado inicial apagado
    }

    const uint inputs[] = {BUTTON_PIN};
    for (int i = 0; i < 1; i++) {
        gpio_init(inputs[i]);
        gpio_set_dir(inputs[i], GPIO_IN);
        gpio_pull_up(inputs[i]);
    }
}
```

---

## Orden de inicialización del ADC

```
1. Inicializar el periférico ADC global
   adc_init();

2. Configurar el GPIO como entrada analógica
   adc_gpio_init(ADC_PIN);
   // Desconecta el pin del multiplexor digital
   // y lo conecta al ADC — no uses gpio_init() en este pin

3. Seleccionar el canal a leer
   adc_select_input(0);   // 0 = ADC0 = GP26

4. En el while(1) — leer el valor
   uint16_t raw = adc_read();   // 0 a 4095
```

> **Nota:** nunca llames `gpio_init()` en un pin ADC —
> `adc_gpio_init()` hace la configuración correcta del multiplexor interno.
> Usar `gpio_init()` en un pin ADC desconecta la ruta analógica.

---

## Estructura del proyecto

```
RPPicoProjects/
└── ADC/
    ├── pico_sdk_import.cmake
    ├── CMakeLists.txt
    ├── ADC.c
    └── build/
        ├── ADC.uf2
        └── ADC.elf
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

set(PROJECT_NAME    "ADC")
set(PROJECT_SOURCES "ADC.c")
set(PICO_BOARD      "pico")

set(PROJECT_LIBS
    pico_stdlib
    hardware_adc
    hardware_gpio
)

project(${PROJECT_NAME})
pico_sdk_init()
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
pico_add_extra_outputs(${PROJECT_NAME})
```

---

## Código fuente

```c
/**
 * @file ADC.c
 * @brief Lectura de potenciómetro con ADC y control de LED
 *
 * @details
 * Lee el voltaje en GP26 (ADC0) mediante el ADC de 12 bits del RP2040.
 * Enciende el LED integrado (GP25) cuando el voltaje supera ~1.65V
 * (mitad del rango = raw > 4095/2 = 2047).
 *
 * Resultado verificado con multímetro: el LED enciende a ~1.6V reales,
 * consistente con el umbral de 2047/4095 × 3.3V = 1.648V.
 *
 * @author   obviousfancy
 * @date     2026-06-28
 * @board    pico (RP2040)
 * @sdk      Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

/* ─── Defines ──────────────────────────────────────────── */
#define ADC_PIN  26     // GP26 = ADC0 (único canal libre en este board)
#define LED_PIN  25     // LED integrado del Pico RP2040
#define ADC_MID  (4095 / 2)  // umbral: mitad del rango = 2047

/* ─── Main ─────────────────────────────────────────────── */
int main() {

    // Inicializar ADC
    adc_init();
    adc_gpio_init(ADC_PIN);   // GP26 como entrada analógica
    adc_select_input(0);      // canal 0 = ADC0 = GP26

    // Inicializar LED como salida digital
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        uint16_t raw = adc_read();   // lectura cruda 0–4095

        if (raw >= ADC_MID) {
            gpio_put(LED_PIN, 1);    // voltaje alto → LED encendido
        } else {
            gpio_put(LED_PIN, 0);    // voltaje bajo → LED apagado
        }

        sleep_ms(100);   // 10 lecturas por segundo
    }
}
```

---

## Verificación con multímetro

El umbral teórico es:

```
V_umbral = (ADC_MID / 4095.0) × 3.3
V_umbral = (2047 / 4095.0) × 3.3
V_umbral = 0.4998 × 3.3
V_umbral = 1.649V
```

**Resultado medido:** el LED enciende a ~1.6V — consistente con el
cálculo teórico. La diferencia de ~50mV es normal por:
- Tolerancia del potenciómetro (±20% típico en componentes genéricos)
- Ruido inherente del ADC (~±5 counts)
- Variación de la referencia de 3.3V del regulador del Pico

---

## Errores comunes

### El ADC siempre lee 0 o 4095
Pin incorrecto — solo GP26, GP27, GP28 y GP29 tienen ADC:
```c
#define ADC_PIN 24   // MAL — GP24 no tiene ADC
#define ADC_PIN 26   // BIEN — GP26 = ADC0
```

### El LED no responde al potenciómetro
No guardas el valor de `adc_read()`:
```c
adc_read();              // MAL — tiras el valor
uint16_t raw = adc_read(); // BIEN
```

### Error de compilación en gpio_init
Falta el argumento del pin:
```c
gpio_init();         // MAL — falta el pin
gpio_init(LED_PIN);  // BIEN
```

### Operador `=>` no existe en C
```c
if (raw => 2047)   // MAL — => no existe en C
if (raw >= 2047)   // BIEN — >= es mayor o igual
```

---

## Configuraciones avanzadas del ADC (referencia)

### Sensor de temperatura interno (ADC4)
```c
adc_set_temp_sensor_enabled(true);
adc_select_input(4);
uint16_t raw = adc_read();

// Conversión según datasheet del RP2040:
float voltage = (raw / 4095.0f) * 3.3f;
float temp_c  = 27.0f - (voltage - 0.706f) / 0.001721f;
```

### Promediado para reducir ruido
```c
uint32_t suma = 0;
for (int i = 0; i < 16; i++) {
    suma += adc_read();
}
uint16_t promedio = suma / 16;
```

### Frecuencia de muestreo controlada
```c
// F_ADC_max = 48MHz / 96 = 500 kSPS
// Reducir con divisor:
adc_set_clkdiv(divisor);
// F_muestreo = 48,000,000 / (96 × (divisor + 1))
```

### Múltiples canales con round-robin
```c
// Alternar automáticamente entre ADC0, ADC1 y ADC2
adc_set_round_robin(0b00111);
```

### ADC con FIFO (modo continuo)
```c
adc_fifo_setup(true, false, 1, false, false);
adc_run(true);                    // inicia conversión continua
uint16_t val = adc_fifo_get_blocking(); // espera y lee
adc_run(false);
adc_fifo_drain();
```

---

## Resumen de lo aprendido

```
ADC del RP2040:
  12 bits → 4096 niveles (0 a 4095)
  5 canales: ADC0(GP26), ADC1(GP27), ADC2(GP28), ADC3(GP29), ADC4(temp)
  Solo esos GPIOs tienen capacidad analógica

Conversión cruda → voltaje:
  V = (raw / 4095.0) × 3.3

Orden de init:
  adc_init()           ← inicializa el periférico
  adc_gpio_init(pin)   ← configura pin como analógico (NO gpio_init)
  adc_select_input(ch) ← selecciona canal
  adc_read()           ← lectura 0-4095

Múltiples GPIOs:
  gpio_init_mask((1<<pin1)|(1<<pin2))  ← inicializa varios
  o función gpio_setup() propia        ← patrón profesional
```

---

## Siguiente práctica sugerida

- **Práctica 06B** — ADC + PWM: controlar brillo del LED proporcionalmente al potenciómetro
- **Práctica 07** — UART: ver los valores del ADC en monitor serial con printf
- **Práctica 08** — I2C: sensores y displays en bus de 2 hilos
