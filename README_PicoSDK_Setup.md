# Raspberry Pi Pico SDK — Setup completo en Ubuntu

> Guía basada en la experiencia real de configuración en Ubuntu 26.04 con SDK 2.2.0.
> Aplica para Pico (RP2040) y Pico 2 (RP2350).

---

## Requisitos previos

- Ubuntu 24.04 / 26.04
- Git instalado
- Conexión a internet
- Cuenta de GitHub configurada con SSH (recomendado)

---

## Paso 1 — Instalar dependencias del sistema

Instala el compilador ARM, CMake y librerías necesarias:

```bash
sudo apt install cmake python3 build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Verifica que todo quedó bien:

```bash
arm-none-eabi-gcc --version   # debe mostrar versión 14.x o superior
cmake --version               # debe mostrar versión 3.13 o superior
```

> **¿Qué es `gcc-arm-none-eabi`?**
> Es el cross-compiler — tu PC es x86_64 pero el Pico corre ARM,
> así que necesitas un compilador especial que genere código para ARM
> desde tu máquina Linux.

---

## Paso 2 — Clonar el SDK

Elige una ubicación fija para el SDK. Se recomienda dentro de tu carpeta de desarrollo:

```bash
git clone https://github.com/raspberrypi/pico-sdk.git ~/Documents/tuusuario/pico-sdk
```

### 2.1 — Inicializar los submódulos

El SDK depende de librerías externas (TinyUSB, BTstack, lwIP, mbedtls, etc.)
que no se descargan automáticamente al clonar. Debes inicializarlas manualmente:

```bash
cd ~/Documents/tuusuario/pico-sdk
git submodule update --init
```

> **¿Por qué es necesario esto?**
> Sin este paso, los headers como `pico/stdlib.h` no existen físicamente en disco
> y la compilación falla con `fatal error: pico/stdlib.h: No such file or directory`.

Submódulos que se descargan:
- `lib/tinyusb` — soporte USB
- `lib/btstack` — soporte Bluetooth
- `lib/lwip` — stack de red TCP/IP
- `lib/mbedtls` — criptografía
- `lib/cyw43-driver` — driver WiFi para Pico W

---

## Paso 3 — Configurar la variable de entorno

El SDK necesita ser localizable desde cualquier proyecto. Se configura una vez
y queda disponible permanentemente:

```bash
echo 'export PICO_SDK_PATH=~/Documents/tuusuario/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

Verifica:

```bash
echo $PICO_SDK_PATH
# debe imprimir: /home/tuusuario/Documents/tuusuario/pico-sdk
```

> **¿Por qué `~/.bashrc`?**
> Al agregarlo ahí, la variable se carga automáticamente cada vez que
> abres una terminal. Sin esto tendrías que exportarla manualmente
> en cada sesión.

---

## Paso 4 — Crear un proyecto nuevo

### Estructura de carpetas recomendada

```
~/Documents/tuusuario/
├── pico-sdk/           ← SDK compartido, no tocar
└── RPPicoProjects/     ← todos tus proyectos aquí
    ├── hello_world/
    ├── blink/
    └── ...
```

### 4.1 — Crear la carpeta del proyecto

```bash
mkdir -p ~/Documents/tuusuario/RPPicoProjects/hello_world
cd ~/Documents/tuusuario/RPPicoProjects/hello_world
```

### 4.2 — Copiar el archivo puente del SDK

```bash
cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
```

> **¿Qué es `pico_sdk_import.cmake`?**
> Es un archivo que conecta tu proyecto con el SDK. CMake no sabe nada del Pico
> por defecto — este archivo le dice "busca el SDK en `PICO_SDK_PATH` y carga todo".
> Se copia una vez por proyecto y nunca se modifica.

### 4.3 — Crear el archivo de código fuente

Crea `hello_world.c`:

```c
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    printf("Hello, world!\n");
    return 0;
}
```

| Línea | Significado |
|---|---|
| `#include <stdio.h>` | librería estándar de C, provee `printf` |
| `#include "pico/stdlib.h"` | librería base del SDK, inicializa el hardware |
| `stdio_init_all()` | activa UART o USB para que `printf` tenga por dónde salir |
| `printf(...)` | manda texto por serial, visible en monitor serial |

### 4.4 — Crear el archivo de construcción CMake

Crea `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_world)

pico_sdk_init()

add_executable(hello_world
    hello_world.c
)

target_link_libraries(hello_world
    pico_stdlib
)

pico_add_extra_outputs(hello_world)
```

| Línea | Significado |
|---|---|
| `cmake_minimum_required` | versión mínima de CMake requerida |
| `include(pico_sdk_import.cmake)` | carga el archivo puente, conecta con el SDK |
| `project(hello_world)` | nombre del proyecto y de los archivos de salida |
| `pico_sdk_init()` | inicializa el SDK y configura el toolchain ARM, siempre después de `project()` |
| `add_executable(...)` | qué archivos `.c` compilar |
| `target_link_libraries(...)` | qué librerías del SDK usar (`pico_stdlib` = lo básico: GPIO, UART, timers) |
| `pico_add_extra_outputs(...)` | genera `.uf2`, `.hex`, `.bin` además del `.elf` |

> **Error común:** no pongas nombres de archivos `.c` en `target_link_libraries`.
> Solo van nombres de librerías del SDK como `pico_stdlib`, `hardware_gpio`, etc.

### 4.5 — Verificar los tres archivos

```bash
ls
# debe mostrar:
# CMakeLists.txt  hello_world.c  pico_sdk_import.cmake
```

---

## Paso 5 — Compilar el proyecto

El proceso es siempre en dos pasos: **configurar** y luego **compilar**.

### 5.1 — Para Pico / Pico H (RP2040)

```bash
cmake -S . -B build
cmake --build build
```

### 5.2 — Para Pico 2 (RP2350)

```bash
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

> **¿Qué hace cada comando?**
> - `cmake -S . -B build` — lee el `CMakeLists.txt`, detecta el toolchain ARM y prepara todo en la carpeta `build/`.
> - `cmake --build build` — compila el código usando `gcc-arm-none-eabi` y genera los binarios.

> La primera compilación tarda más porque también compila el SDK y descarga `picotool`.
> Las siguientes son mucho más rápidas.

### 5.3 — Verificar el output

```bash
ls build/*.uf2
# debe mostrar: build/hello_world.uf2
```

---

## Paso 6 — Flashear el Pico

1. Mantén presionado el botón **BOOTSEL** del Pico
2. Conéctalo por USB mientras lo tienes presionado
3. Suéltalo — aparece como unidad USB llamada `RPI-RP2`
4. Copia el `.uf2`:

```bash
cp build/hello_world.uf2 /media/$USER/RPI-RP2/
```

5. El Pico se reinicia automáticamente y corre el programa

---

## Paso 7 — Ver el output serial (requiere monitor serial)

El `printf` sale por UART. Para verlo:

```bash
sudo apt install picocom
picocom -b 115200 /dev/ttyACM0
```

Debes ver:
```
Hello, world!
```

Para salir de picocom: `Ctrl+A` → `Ctrl+X`

---

## Flujo de compilación completo

```
tu código .c
      ↓
   CMakeLists.txt        ← defines qué compilar y qué librerías usar
      ↓
   cmake -S . -B build   ← configura, detecta toolchain ARM
      ↓
   cmake --build build   ← compila con gcc-arm-none-eabi
      ↓
   hello_world.uf2       ← lo que arrastras/copias al Pico
```

---

## Errores comunes y soluciones

### `fatal error: pico/stdlib.h: No such file or directory`
Los submódulos del SDK no están inicializados:
```bash
cd $PICO_SDK_PATH
git submodule update --init
cd -
rm -rf build
cmake -S . -B build && cmake --build build
```

### `PICO_SDK_PATH` no definido
La variable de entorno no está cargada:
```bash
echo 'export PICO_SDK_PATH=/ruta/a/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

### Error en `target_link_libraries`
No pongas archivos `.c` ahí — solo nombres de librerías:
```cmake
# MAL
target_link_libraries(hello_world hello_world.c)

# BIEN
target_link_libraries(hello_world pico_stdlib)
```

---

## Para proyectos futuros

La estructura de cada nuevo proyecto es siempre igual:

```
mi_proyecto/
├── pico_sdk_import.cmake   ← copiado de $PICO_SDK_PATH/external/
├── CMakeLists.txt          ← configuración del proyecto
├── main.c                  ← tu código
└── build/                  ← generado por cmake, no tocar manualmente
```

Solo cambia el contenido de `CMakeLists.txt` y tus archivos `.c`.
`pico_sdk_import.cmake` siempre es la misma copia idéntica.

---

## Paso 8 — Automatizar build y flasheo con `flash.sh`

Para evitar correr manualmente `rm -rf build`, `cmake -S . -B build`,
`cmake --build build` y `openocd` cada vez que quieras probar un cambio,
se usa un script bash inteligente que automatiza todo el ciclo con un solo comando.

### 8.1 — Crear la carpeta de scripts

```bash
mkdir -p ~/Documents/tuusuario/RPPicoProjects/scripts
```

### 8.2 — Crear el script `flash.sh`

```bash
nano ~/Documents/tuusuario/RPPicoProjects/scripts/flash.sh
```

Contenido del script:

```bash
#!/bin/bash

# ─────────────────────────────────────────────
# flash.sh — Build y flasheo automático
# Raspberry Pi Pico SDK + OpenOCD + CMSIS-DAP
# Obviousfancy Lab
# ─────────────────────────────────────────────

# Detecta el nombre del proyecto desde CMakeLists.txt automáticamente
PROJECT=$(grep "^project(" CMakeLists.txt | sed 's/project(//;s/)//' | tr -d ' ')

if [ -z "$PROJECT" ]; then
    echo "[ERROR] No se pudo detectar el nombre del proyecto desde CMakeLists.txt"
    exit 1
fi

echo "[INFO] Proyecto detectado: ${PROJECT}"

# Si no existe el build, configurar desde cero
if [ ! -d "build" ]; then
    echo "[INFO] Carpeta build no encontrada, configurando CMake..."
    cmake -S . -B build
    if [ $? -ne 0 ]; then
        echo "[ERROR] Falló la configuración de CMake"
        exit 1
    fi
fi

# Compilar (solo recompila lo que cambió)
echo "[INFO] Compilando..."
cmake --build build
if [ $? -ne 0 ]; then
    echo "[ERROR] Falló la compilación"
    echo "[INFO] Intentando limpiar build y recompilar..."
    rm -rf build
    cmake -S . -B build && cmake --build build
    if [ $? -ne 0 ]; then
        echo "[ERROR] Falló la recompilación limpia"
        exit 1
    fi
fi

# Verificar que el .elf existe
if [ ! -f "build/${PROJECT}.elf" ]; then
    echo "[ERROR] No se encontró build/${PROJECT}.elf"
    exit 1
fi

# Flashear por OpenOCD
echo "[INFO] Flasheando ${PROJECT}.elf por OpenOCD..."
openocd -f interface/cmsis-dap.cfg \
        -f target/rp2040.cfg \
        -c "adapter speed 5000" \
        -c "program build/${PROJECT}.elf verify reset exit"

if [ $? -ne 0 ]; then
    echo "[ERROR] Falló el flasheo por OpenOCD"
    exit 1
fi

echo "[OK] ${PROJECT}.elf flasheado correctamente"
```

### 8.3 — Dar permisos de ejecución

```bash
chmod +x ~/Documents/tuusuario/RPPicoProjects/scripts/flash.sh
```

### 8.4 — Crear alias global

Para poder usar el script desde cualquier proyecto con una sola palabra:

```bash
echo "alias pico-flash='/home/tuusuario/Documents/tuusuario/RPPicoProjects/scripts/flash.sh'" >> ~/.bashrc
source ~/.bashrc
```

Verifica que quedó registrado:

```bash
alias | grep pico-flash
```

### 8.5 — Uso

Desde la carpeta raíz de cualquier proyecto:

```bash
cd ~/Documents/tuusuario/RPPicoProjects/mi_proyecto
pico-flash
```

El script hace todo automáticamente en orden:

| Paso | Qué hace |
|---|---|
| 1 | Lee el nombre del proyecto desde `CMakeLists.txt` |
| 2 | Si no existe `build/`, corre `cmake -S . -B build` |
| 3 | Compila solo lo que cambió con `cmake --build build` |
| 4 | Si falla la compilación, limpia y reintenta desde cero |
| 5 | Verifica que el `.elf` fue generado correctamente |
| 6 | Flashea por OpenOCD via CMSIS-DAP |

### Comportamiento inteligente

| Situación | Comportamiento |
|---|---|
| Primera vez, sin `build/` | Crea el build desde cero y compila |
| Ya existe `build/`, código cambiado | Solo recompila lo que cambió |
| Ya existe `build/`, nada cambió | No recompila, va directo a flashear |
| Falla la compilación | Limpia el build y reintenta automáticamente |
| No encuentra el `.elf` | Aborta con mensaje claro antes de flashear |

> **Nota:** el script detecta automáticamente el nombre del proyecto desde
> `CMakeLists.txt` — no necesitas modificarlo para cada proyecto nuevo.
> Solo copia el alias, ve a la carpeta del proyecto y corre `pico-flash`.

---

## Paso 9 — Scaffolding CLI con `pico-new`

Para no crear manualmente la carpeta, el `CMakeLists.txt`, el `.c` y copiar
el `pico_sdk_import.cmake` en cada proyecto nuevo, se usa una herramienta CLI
interactiva que hace todo eso con un menú visual en la terminal.

### ¿Qué hace `pico-new`?

1. Pide el nombre del proyecto y lo valida (sin espacios, sin caracteres inválidos para C)
2. Pide una descripción breve para el encabezado Doxygen
3. Muestra un selector del board objetivo (pico / pico2 / pico_w)
4. Muestra checkboxes con todos los periféricos disponibles (GPIO, PWM, I2C, SPI, UART, ADC, TIMER, IRQ)
5. Genera automáticamente:
   - `CMakeLists.txt` con las librerías seleccionadas
   - `nombre_proyecto.c` con includes, init code y estructura Doxygen
   - `pico_sdk_import.cmake` copiado directo del SDK

### 9.1 — Estructura de archivos de la herramienta

```
RPPicoProjects/
└── scripts/
    ├── flash.sh                        ← automatiza build y flasheo
    ├── pico-new.py                     ← CLI de scaffolding
    └── templates/
        ├── CMakeLists.txt.template     ← plantilla del CMakeLists
        └── main.c.template             ← plantilla del archivo .c
```

### 9.2 — Instalar dependencia Python

`pico-new` usa la librería `questionary` para los menús interactivos:

```bash
pip install questionary --break-system-packages
```

Verifica:

```bash
python3 -c "import questionary; print('OK')"
```

### 9.3 — Colocar los archivos en su lugar

Copia `pico-new.py` y la carpeta `templates/` dentro de tu carpeta `scripts/`:

```
scripts/
├── flash.sh
├── pico-new.py
└── templates/
    ├── CMakeLists.txt.template
    └── main.c.template
```

### 9.4 — Crear alias global

```bash
echo "alias pico-new='python3 /home/tuusuario/Documents/tuusuario/RPPicoProjects/scripts/pico-new.py'" >> ~/.bashrc
source ~/.bashrc
```

Verifica:

```bash
alias | grep pico-new
```

### 9.5 — Uso

Desde cualquier lugar de la terminal:

```bash
pico-new
```

El CLI muestra una interfaz visual interactiva:

```
╔══════════════════════════════════════════════════════╗
║           pico-new — Scaffolding CLI                 ║
║     Raspberry Pi Pico SDK Project Generator          ║
║              Obviousfancy Lab                        ║
╚══════════════════════════════════════════════════════╝

? Nombre del proyecto: sensor_i2c
? Descripción breve: Lectura de sensor por I2C
? Board objetivo: (Use arrow keys)
 ❯ pico
   pico2
   pico_w
? Selecciona los periféricos: (Space para marcar, Enter para confirmar)
 ❯ ○ GPIO — Control de pines digitales
   ○ PWM  — Señal de ancho de pulso variable
   ◉ I2C  — Bus serial de 2 hilos (SDA/SCL)
   ○ SPI  — Bus serial de 4 hilos
   ○ UART — Comunicación serial asíncrona
   ○ ADC  — Lectura analógica
   ○ TIMER — Temporizadores y alarmas
   ○ IRQ  — Interrupciones de hardware
```

Al confirmar, genera el proyecto completo:

```
  ✓ /home/tuusuario/RPPicoProjects/sensor_i2c/CMakeLists.txt
  ✓ /home/tuusuario/RPPicoProjects/sensor_i2c/sensor_i2c.c
  ✓ /home/tuusuario/RPPicoProjects/sensor_i2c/pico_sdk_import.cmake

  Para compilar y flashear:
    cd /home/tuusuario/RPPicoProjects/sensor_i2c
    pico-flash
```

### 9.6 — Ejemplo de archivos generados

**`CMakeLists.txt` generado (con I2C seleccionado):**

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

# ─────────────────────────────────────────────
# CONFIGURACIÓN DEL PROYECTO
# Generado automáticamente por pico-new
# ─────────────────────────────────────────────
set(PROJECT_NAME    "sensor_i2c")
set(PROJECT_SOURCES "sensor_i2c.c")
set(PICO_BOARD      "pico")

set(PROJECT_LIBS
    pico_stdlib
    hardware_i2c
)
# ─────────────────────────────────────────────
# NO MODIFICAR DE AQUÍ EN ADELANTE
# ─────────────────────────────────────────────

project(${PROJECT_NAME})
pico_sdk_init()
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
pico_add_extra_outputs(${PROJECT_NAME})
```

**`sensor_i2c.c` generado:**

```c
/**
 * @file sensor_i2c.c
 * @brief Lectura de sensor por I2C
 *
 * @author obviousfancy
 * @date 2026-06-28
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* ─── Defines ──────────────────────────────────────────── */
// #define MY_PIN 0

/* ─── Main ─────────────────────────────────────────────── */
int main() {
    // i2c_init(i2c0, 100 * 1000);
    // gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    while (1) {
        // TODO: lógica principal del proyecto
    }
}
```

### 9.7 — Periféricos disponibles

| Periférico | Librería SDK | Header incluido |
|---|---|---|
| GPIO | `hardware_gpio` | `hardware/gpio.h` |
| PWM | `hardware_pwm` | `hardware/pwm.h` |
| I2C | `hardware_i2c` | `hardware/i2c.h` |
| SPI | `hardware_spi` | `hardware/spi.h` |
| UART | `hardware_uart` | `hardware/uart.h` |
| ADC | `hardware_adc` | `hardware/adc.h` |
| TIMER | `hardware_timer` | `hardware/timer.h` |
| IRQ | `hardware_irq` | `hardware/irq.h` |

---

## Flujo de trabajo completo (resumen final)

Una vez configurado todo, el flujo para cualquier proyecto nuevo es:

```
pico-new          ← crea la estructura completa del proyecto
      ↓
(editar el .c con tu lógica)
      ↓
pico-flash        ← compila y flashea al Pico automáticamente
```

### Estructura final de tu entorno

```
~/Documents/tuusuario/
├── pico-sdk/                      ← SDK compartido, no tocar
└── RPPicoProjects/
    ├── scripts/
    │   ├── flash.sh               ← pico-flash
    │   ├── pico-new.py            ← pico-new
    │   └── templates/
    │       ├── CMakeLists.txt.template
    │       └── main.c.template
    ├── hello_world/               ← proyectos generados
    ├── blink/
    ├── button/
    └── ...
```

### Alias configurados en `~/.bashrc`

```bash
export PICO_SDK_PATH=/home/tuusuario/Documents/tuusuario/pico-sdk
alias pico-flash='...scripts/flash.sh'
alias pico-new='python3 ...scripts/pico-new.py'
```
