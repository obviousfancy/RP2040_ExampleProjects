# Práctica 02 — Blink LED (GPIO Output)
### Raspberry Pi Pico SDK 2.2.0 — RP2040

> Encender y apagar el LED integrado del Pico usando GPIO,
> la base de cualquier proyecto de hardware embebido.

---

## ¿Qué vamos a hacer?

Controlar el LED integrado del Pico (conectado internamente al **GPIO 25**)
para que parpadee cada segundo. Es el "Hello World" del hardware embebido.

```
LED ON  ───────┐     ┌─────
               │     │
LED OFF        └─────┘
           1 seg  1 seg
```

---

## Hardware necesario

- Raspberry Pi Pico (RP2040)
- Cable USB para alimentación y flasheo
- Nada más — el LED ya está integrado en la placa

> El LED integrado está conectado internamente al GPIO 25.
> No necesitas componentes externos para esta práctica.

---

## Librerías del SDK utilizadas

### `pico/stdlib.h`
La librería estándar del Pico. Es un header que agrupa todo lo básico
y casi siempre se incluye en todos los proyectos. Provee:

| Función / Header incluido | Para qué sirve |
|---|---|
| `sleep_ms(uint32_t ms)` | Pausa la ejecución por N milisegundos |
| `sleep_us(uint64_t us)` | Pausa la ejecución por N microsegundos |
| `pico/time.h` | Incluido automáticamente dentro de stdlib |
| `pico/platform.h` | Funciones básicas del chip |

> Si no vas a usar `printf` ni comunicación serial, no necesitas
> llamar a `stdio_init_all()`. Solo incluye el header y usa `sleep_ms`.

### `hardware/gpio.h`
Librería específica para controlar los pines GPIO del RP2040. Provee:

| Función | Descripción |
|---|---|
| `gpio_init(uint gpio)` | Inicializa el pin GPIO especificado |
| `gpio_set_dir(uint gpio, bool out)` | Configura el pin como entrada o salida |
| `gpio_put(uint gpio, bool value)` | Pone el pin en alto (1) o en bajo (0) |
| `gpio_get(uint gpio)` | Lee el estado actual del pin |

---

## Funciones utilizadas en esta práctica

### `gpio_init(uint gpio)`
Inicializa el pin GPIO para que el chip lo active y lo prepare para usar.
Siempre es el primer paso antes de configurar o usar cualquier pin.

```c
gpio_init(25);  // Inicializa GPIO 25
```

### `gpio_set_dir(uint gpio, bool out)`
Configura la dirección del pin: salida o entrada.
Los GPIOs pueden funcionar en ambos modos, debes especificar cuál.

```c
gpio_set_dir(25, GPIO_OUT);  // GPIO 25 como salida
gpio_set_dir(25, GPIO_IN);   // GPIO 25 como entrada
```

### `gpio_put(uint gpio, bool value)`
Pone el pin en alto (3.3V) o en bajo (0V).
En alto el LED enciende, en bajo apaga.

```c
gpio_put(25, 1);  // Enciende el LED (3.3V)
gpio_put(25, 0);  // Apaga el LED (0V)
```

### `sleep_ms(uint32_t ms)`
Pausa la ejecución por N milisegundos.
1000 ms = 1 segundo.

```c
sleep_ms(1000);  // Espera 1 segundo
sleep_ms(500);   // Espera medio segundo
```

---

## Estructura del proyecto

```
RPPicoProjects/
└── blink/
    ├── pico_sdk_import.cmake   ← puente con el SDK, nunca se modifica
    ├── CMakeLists.txt          ← configuración de compilación
    ├── blink.c                 ← tu código fuente
    └── build/                  ← generado por cmake, no tocar
        ├── blink.uf2           ← archivo para flashear al Pico
        ├── blink.elf           ← para depuración con openocd/gdb
        └── blink.hex           ← formato alternativo de flasheo
```

---

## Paso 1 — Crear el proyecto

```bash
mkdir -p ~/Documents/tuusuario/RPPicoProjects/blink
cd ~/Documents/tuusuario/RPPicoProjects/blink
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
```

---

## Paso 2 — Código fuente (`blink.c`)

```c
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25      // GPIO del LED integrado en Pico RP2040

int main() {
    gpio_init(LED_PIN);             // 1. Inicializar el pin
    gpio_set_dir(LED_PIN, GPIO_OUT); // 2. Configurar como salida

    while (1) {                      // 3. Bucle infinito
        gpio_put(LED_PIN, 1);        // Encender LED
        sleep_ms(1000);              // Esperar 1 segundo
        gpio_put(LED_PIN, 0);        // Apagar LED
        sleep_ms(1000);              // Esperar 1 segundo
    }
}
```

### ¿Por qué `while(1)`?
En sistemas embebidos no hay sistema operativo que mantenga vivo el programa.
Si `main()` termina, el microcontrolador queda en un estado indefinido.
El `while(1)` mantiene el programa corriendo indefinidamente.

### ¿Por qué `#define LED_PIN 25`?
Para no hardcodear el número del pin por todo el código.
Si en el futuro cambias de pin, solo modificas esa línea y todo el
resto del código se actualiza automáticamente.

---

## Paso 3 — Configuración de compilación (`CMakeLists.txt`)

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(blink)

pico_sdk_init()

add_executable(blink
    blink.c
)

target_link_libraries(blink
    pico_stdlib
    hardware_gpio
)

pico_add_extra_outputs(blink)
```

### ¿Qué hace cada línea?

| Línea | Significado |
|---|---|
| `cmake_minimum_required` | Versión mínima de CMake requerida |
| `include(pico_sdk_import.cmake)` | Carga el archivo puente con el SDK |
| `project(blink)` | Nombre del proyecto y de los archivos de salida |
| `pico_sdk_init()` | Inicializa el SDK y el toolchain ARM, siempre después de `project()` |
| `add_executable(blink blink.c)` | Qué archivos `.c` compilar |
| `target_link_libraries(...)` | Qué librerías del SDK enlazar |
| `pico_add_extra_outputs(blink)` | Genera `.uf2`, `.hex`, `.bin` además del `.elf` |

> **Error común:** en `target_link_libraries` solo van nombres de librerías
> del SDK, nunca nombres de archivos `.c`.

---

## Paso 4 — Compilar

```bash
cmake -S . -B build
cmake --build build
```

| Comando | Qué hace |
|---|---|
| `cmake -S . -B build` | Lee el `CMakeLists.txt`, detecta el toolchain ARM y prepara la carpeta `build/` |
| `cmake --build build` | Compila el código con `gcc-arm-none-eabi` y genera los binarios |

Si necesitas recompilar desde cero (tras cambios en `CMakeLists.txt`):

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

Verifica que el `.uf2` fue generado:

```bash
ls build/*.uf2
# build/blink.uf2
```

---

## Paso 5 — Flashear al Pico

### Opción A — Modo BOOTSEL (arrastrar y soltar)

1. Mantén presionado el botón **BOOTSEL** del Pico
2. Conéctalo por USB mientras lo presionas
3. Suéltalo — aparece como unidad USB `RPI-RP2`
4. Copia el archivo:

```bash
cp build/blink.uf2 /media/$USER/RPI-RP2/
```

5. El Pico se reinicia automáticamente y el LED empieza a parpadear

### Opción B — CMSIS-DAP / SWD con CH552 Multiprotocol (openocd)

Esta es la forma profesional de trabajar — permite flashear y depurar
sin tener que entrar en modo BOOTSEL cada vez.

#### Hardware necesario adicional
- CH552 Multiprotocol flasheado con firmware CMSIS-DAP (`1a86:8011`)
- 3 cables dupont hembra-hembra

#### Verificar que Ubuntu reconoce el CH552

```bash
lsusb
# debe aparecer: QinHeng Electronics MultiProtocol CMSIS-DAP

ls /dev/ttyACM*
# debe aparecer: /dev/ttyACM0
```

#### Conexión física SWD al Pico

| CH552 (label) | Pin físico Pico | Función |
|---|---|---|
| SWDIO | Pin 34 | Data |
| SWDCLK | Pin 32 | Clock |
| GND | Cualquier GND | Tierra común |

> El Pico se alimenta por su propio USB — no necesitas alimentarlo
> desde el CH552. Solo conectas los 3 cables SWD.

#### Comando para flashear

Desde la carpeta raíz de tu proyecto:

```bash
openocd -f interface/cmsis-dap.cfg \
        -f target/rp2040.cfg \
        -c "adapter speed 5000" \
        -c "program build/blink.elf verify reset exit"
```

| Parámetro | Significado |
|---|---|
| `-f interface/cmsis-dap.cfg` | Le dice a openocd que el debugger es CMSIS-DAP genérico |
| `-f target/rp2040.cfg` | Le dice que el chip objetivo es el RP2040 |
| `-c "adapter speed 5000"` | Velocidad de comunicación SWD en KHz |
| `program build/blink.elf` | Archivo a flashear (usar `.elf`, no `.uf2`) |
| `verify` | Verifica que se escribió correctamente en flash |
| `reset` | Reinicia el Pico para que corra el programa inmediatamente |
| `exit` | Cierra openocd al terminar |

> **Importante:** openocd usa el `.elf`, no el `.uf2`.
> El `.elf` contiene información de depuración adicional que el `.uf2` no tiene.

#### Output esperado al flashear correctamente

```
Open On-Chip Debugger 0.12.0
Info : CMSIS-DAP: SWD supported
Info : CMSIS-DAP: Atomic commands supported
Info : Connecting to target via SWD
Info : rp2040.core0: hardware has 4 breakpoints, 2 watchpoints
Info : starting gdb server for rp2040.core0 on 3333
** Programming Started **
** Programming Finished **
** Verify Started **
** Verified OK **
** Resetting Target **
shutdown command invoked
```

---

## Errores comunes

### `implicit declaration of function 'stdio_init_all'`
Falta incluir `pico/stdlib.h`:
```c
#include "pico/stdlib.h"  // ← agregar esta línea
```

### `gpio_delay_us` no existe
Esa función no existe en el SDK. Usa `sleep_us()` o `sleep_ms()`:
```c
sleep_ms(1000);   // 1 segundo
sleep_us(500000); // 0.5 segundos
```

### El LED no parpadea después de flashear
Verifica que el `.uf2` se copió correctamente y que el Pico salió
del modo BOOTSEL (la unidad USB `RPI-RP2` debe desaparecer).

---

## Resumen de lo aprendido

```
#include "pico/stdlib.h"    ← sleep_ms, funciones básicas del Pico
#include "hardware/gpio.h"  ← control de pines GPIO

gpio_init(pin)              ← siempre primero
gpio_set_dir(pin, GPIO_OUT) ← configurar como salida
gpio_put(pin, 1 o 0)        ← encender o apagar
sleep_ms(ms)                ← esperar N milisegundos
while(1) { }                ← bucle infinito, obligatorio en embebido
```

---

## Siguiente práctica sugerida

- **Práctica 03** — Leer un botón (GPIO Input) y controlar el LED con él
- **Práctica 04** — PWM para controlar el brillo del LED gradualmente
