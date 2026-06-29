# Práctica 03 — Botón y GPIO como Entrada (GPIO Input)
### Raspberry Pi Pico SDK 2.2.0 — RP2040

> Leer el estado de un botón físico mediante GPIO configurado como entrada,
> y controlar el LED integrado en función de su estado.
> Esta práctica introduce los conceptos de pull-up, pull-down y lectura digital.

---

## ¿Qué vamos a hacer?

Leer el estado del botón de usuario integrado en el board (GP24) y usarlo
para controlar el LED integrado (GP25):

```
Botón sin presionar  →  LED apagado
Botón presionado     →  LED encendido
```

---

## Conceptos fundamentales antes de empezar

### ¿Qué es un GPIO como entrada?

Un GPIO configurado como entrada permite al microcontrolador **leer** el voltaje
presente en ese pin. El RP2040 interpreta:

- **1 (HIGH)** → el pin tiene 3.3V
- **0 (LOW)**  → el pin tiene 0V (GND)

### El problema del pin flotante

Cuando un botón no está presionado y el pin no está conectado a nada,
queda en un estado indefinido llamado **pin flotante (floating pin)**.
En este estado el pin puede leer valores aleatorios — a veces 0, a veces 1 —
porque capta ruido eléctrico del ambiente.

```
Estado flotante (problemático):
GP24 ── [Botón] ── GND
  │
  ? (lectura indefinida cuando el botón no está presionado)
```

Para evitar esto se usan **resistencias de pull**.

---

## Resistencias de Pull — explicación completa

### Pull-Down

Conecta el pin a GND a través de una resistencia cuando el botón no está presionado.
Garantiza que el pin lee **0 en reposo** y **1 al presionar**.

```
3.3V ──[Botón]── GPIO
                   │
                 [10kΩ]   ← resistencia pull-down
                   │
                  GND
```

| Estado del botón | Lectura del pin |
|---|---|
| Sin presionar | 0 (LOW) — pin jalado a GND |
| Presionado | 1 (HIGH) — pin conectado a 3.3V |

**Lógica directa:** 1 = presionado, 0 = suelto.

---

### Pull-Up

Conecta el pin a 3.3V a través de una resistencia cuando el botón no está presionado.
Garantiza que el pin lee **1 en reposo** y **0 al presionar**.

```
3.3V
  │
[10kΩ]   ← resistencia pull-up
  │
GPIO ──[Botón]── GND
```

| Estado del botón | Lectura del pin |
|---|---|
| Sin presionar | 1 (HIGH) — pin jalado a 3.3V |
| Presionado | 0 (LOW) — pin conectado a GND |

**Lógica invertida:** 0 = presionado, 1 = suelto.

---

### Pull interna del RP2040

El RP2040 tiene resistencias de pull-up y pull-down **integradas en el silicio**
del chip. Se activan por software sin necesitar componentes externos.
Son de aproximadamente **50kΩ** — más débiles que una resistencia externa
de 10kΩ, pero suficientes para la mayoría de aplicaciones.

| Función | Efecto |
|---|---|
| `gpio_pull_up(pin)` | Activa pull-up interna (~50kΩ a 3.3V) |
| `gpio_pull_down(pin)` | Activa pull-down interna (~50kΩ a GND) |
| `gpio_disable_pulls(pin)` | Desactiva ambas — pin queda flotante si no hay externa |

---

## Investigación del hardware — ¿tiene pull externa?

Antes de escribir código, es importante saber si el board tiene resistencias
externas o si depende de las internas. El proceso para determinarlo:

### Paso 1 — Probar con pull-down interna

```c
gpio_pull_down(BUTTON_PIN);
// Leer gpio_get() sin presionar
// Si lee 1 → tiene pull-up externa
// Si lee 0 → pull-down está funcionando correctamente
```

### Paso 2 — Desactivar todas las pulls

```c
gpio_disable_pulls(BUTTON_PIN);
```

**Resultado en nuestro board:**
- Al presionar el botón encendía el LED
- Al soltar, el LED **se quedaba encendido** y el comportamiento era errático
- Conclusión: **pin flotante** → no hay resistencia externa

### Paso 3 — Conclusión

El circuito del botón en este board es:

```
GP24 ──[USRKEY]── GND
```

Sin ninguna resistencia física. El chip conecta el pin a GND al presionar,
y cuando se suelta queda completamente desconectado. Por eso es **obligatorio**
usar `gpio_pull_up()` por software.

---

## Librerías del SDK utilizadas

### `pico/stdlib.h`
Header estándar del Pico. Agrupa las funcionalidades básicas del SDK.
En esta práctica provee la inicialización del sistema.

### `hardware/gpio.h`
Librería de control de pines GPIO del RP2040. Todas las funciones
de configuración y lectura/escritura de pines vienen de aquí.

---

## Funciones utilizadas

### `gpio_init(uint gpio)`
Inicializa el pin GPIO y lo pone en estado conocido.
Siempre es el primer paso antes de cualquier configuración.

```c
gpio_init(25);  // Inicializa GPIO 25
gpio_init(24);  // Inicializa GPIO 24
```

### `gpio_set_dir(uint gpio, bool out)`
Configura la dirección del pin.

```c
gpio_set_dir(25, GPIO_OUT);  // Pin 25 como salida (LED)
gpio_set_dir(24, GPIO_IN);   // Pin 24 como entrada (Botón)
```

| Constante | Valor | Uso |
|---|---|---|
| `GPIO_OUT` | true | El pin genera voltaje (salida) |
| `GPIO_IN` | false | El pin lee voltaje (entrada) |

### `gpio_pull_up(uint gpio)`
Activa la resistencia de pull-up interna del RP2040 en el pin especificado.
El pin queda jalado a 3.3V cuando no hay nada conectado.

```c
gpio_pull_up(24);  // Pull-up en GP24
```

### `gpio_pull_down(uint gpio)`
Activa la resistencia de pull-down interna del RP2040 en el pin especificado.
El pin queda jalado a GND cuando no hay nada conectado.

```c
gpio_pull_down(24);  // Pull-down en GP24
```

### `gpio_disable_pulls(uint gpio)`
Desactiva tanto la pull-up como la pull-down interna.
El pin queda flotante si no tiene resistencia externa.

```c
gpio_disable_pulls(24);  // Sin pull interna en GP24
```

### `gpio_get(uint gpio)`
Lee el estado digital del pin configurado como entrada.
Devuelve `true` (1) si el pin está en HIGH, `false` (0) si está en LOW.

```c
bool estado = gpio_get(24);  // Lee GP24
```

### `gpio_put(uint gpio, bool value)`
Escribe un valor digital en el pin configurado como salida.

```c
gpio_put(25, 1);  // LED encendido
gpio_put(25, 0);  // LED apagado
```

---

## Estructura del proyecto

```
RPPicoProjects/
└── button/
    ├── pico_sdk_import.cmake   ← puente con el SDK, nunca se modifica
    ├── CMakeLists.txt          ← configuración de compilación
    ├── button.c                ← código fuente
    └── build/
        ├── button.uf2          ← para flashear en modo BOOTSEL
        └── button.elf          ← para flashear con openocd/CMSIS-DAP
```

---

## Paso 1 — Crear el proyecto

```bash
mkdir -p ~/Documents/tuusuario/RPPicoProjects/button
cd ~/Documents/tuusuario/RPPicoProjects/button
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
```

---

## Paso 2 — Código fuente (`button.c`)

```c
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN    25   // LED integrado del Pico RP2040
#define BUTTON_PIN 24   // Botón de usuario USRKEY del board

int main() {

    // Configuración del LED como salida
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Configuración del botón como entrada
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);   // Pull-up interna obligatoria
                                // El board no tiene resistencia externa
                                // Circuito: GP24 ── [USRKEY] ── GND
                                // Reposo = 1 (HIGH), Presionado = 0 (LOW)

    while (1) {
        if (gpio_get(BUTTON_PIN)) {
            gpio_put(LED_PIN, 0);   // Reposo (1) → LED apagado
        } else {
            gpio_put(LED_PIN, 1);   // Presionado (0) → LED encendido
        }
    }
}
```

### Análisis de la lógica

Como el botón usa pull-up, la lógica de lectura es **invertida**:

| `gpio_get(BUTTON_PIN)` | Significado real | Acción |
|---|---|---|
| `1` (HIGH) | Botón **sin presionar** (pull-up jalando a 3.3V) | LED apagado |
| `0` (LOW) | Botón **presionado** (conectado a GND) | LED encendido |

El operador `!` (NOT lógico) es una alternativa más compacta para la misma lógica:

```c
// Equivalente más compacto:
gpio_put(LED_PIN, !gpio_get(BUTTON_PIN));
```

---

## Paso 3 — `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(button)

pico_sdk_init()

add_executable(button
    button.c
)

target_link_libraries(button
    pico_stdlib
    hardware_gpio
)

pico_add_extra_outputs(button)
```

---

## Paso 4 — Compilar

```bash
cmake -S . -B build
cmake --build build
```

Si necesitas recompilar desde cero:

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

---

## Paso 5 — Flashear

### Opción A — BOOTSEL

```bash
cp build/button.uf2 /media/$USER/RPI-RP2/
```

### Opción B — CMSIS-DAP con CH552 Multiprotocol

```bash
openocd -f interface/cmsis-dap.cfg \
        -f target/rp2040.cfg \
        -c "adapter speed 5000" \
        -c "program build/button.elf verify reset exit"
```

---

## Tabla resumen — configuración de GPIO según caso de uso

| Caso | `set_dir` | Pull | `gpio_get` presionado |
|---|---|---|---|
| Botón a GND, sin resistencia externa | `GPIO_IN` | `gpio_pull_up()` | `0` |
| Botón a GND, con pull-down externa | `GPIO_IN` | `gpio_disable_pulls()` | `1` |
| Botón a 3.3V, sin resistencia externa | `GPIO_IN` | `gpio_pull_down()` | `1` |
| Botón a 3.3V, con pull-up externa | `GPIO_IN` | `gpio_disable_pulls()` | `0` |

---

## Errores comunes

### LED se queda encendido o apagado sin importar el botón
El pin está flotando. Falta activar una resistencia de pull:
```c
gpio_pull_up(BUTTON_PIN);   // si el botón conecta a GND
gpio_pull_down(BUTTON_PIN); // si el botón conecta a 3.3V
```

### El botón funciona al revés de lo esperado
La lógica está invertida por el tipo de pull. Agrega `!` al leer:
```c
gpio_put(LED_PIN, !gpio_get(BUTTON_PIN));
```

### El LED parpadea erráticamente sin tocar el botón
Pin flotante — `gpio_disable_pulls()` activo sin resistencia externa.
Activa la pull correspondiente al circuito.

---

## Proceso de diagnóstico de hardware (pull externa o interna)

Cuando no tienes el esquemático del board:

```
1. Activa gpio_pull_down() y observa comportamiento sin presionar
      ├── Lee 1 constantemente → tiene pull-up externa
      └── Lee 0 constantemente → pull-down funciona, úsala

2. Desactiva con gpio_disable_pulls() y observa
      ├── Comportamiento errático / LED se queda fijo → pin flotante
      │   → No tiene resistencia externa, usa pull interna
      └── Funciona correctamente → tiene resistencia externa
```

---

## Resumen de lo aprendido

```
GPIO como entrada:
gpio_set_dir(pin, GPIO_IN)      ← configurar como entrada

Resistencias de pull:
gpio_pull_up(pin)               ← jala a 3.3V, lógica invertida
gpio_pull_down(pin)             ← jala a GND, lógica directa
gpio_disable_pulls(pin)         ← sin pull interna

Lectura digital:
gpio_get(pin)                   ← devuelve 1 o 0
!gpio_get(pin)                  ← invertir la lógica

Regla de oro:
Siempre define el estado de reposo del pin antes de usarlo.
Un pin sin pull definida es un pin flotante — comportamiento impredecible.
```

---

## Siguiente práctica sugerida

- **Práctica 04** — PWM: controlar el brillo del LED gradualmente
- **Práctica 05** — Interrupciones GPIO: reaccionar al botón sin polling en `while(1)`
