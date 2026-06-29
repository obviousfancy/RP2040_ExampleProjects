# Práctica 05 — Control de Servo Motor SG90 con PWM
### Raspberry Pi Pico SDK 2.2.0 — RP2040

> Control de posición de un servomotor SG90 mediante PWM de precisión,
> usando Phase Correct mode y calibración empírica del rango real del servo.
> Esta práctica introduce el cálculo de DIV y WRAP para frecuencias específicas,
> la API de configuración por objeto y el proceso de calibración de hardware real.

---

## Objetivo

Controlar la posición angular de un servomotor SG90 (0° a 180°) mediante
una señal PWM de exactamente 50 Hz generada por el periférico PWM del RP2040,
aplicando Phase Correct mode y calibración empírica del rango real del servo.

Conceptos aprendidos:
- Cálculo de DIV y WRAP para frecuencias de señal críticas y precisas
- Diferencia entre Free Running y Phase Correct mode
- API de configuración por objeto (`pwm_config`)
- Mapeo lineal de ángulos a microsegundos a counts del contador
- Calibración empírica de hardware — por qué el datasheet es punto de partida

---

## Especificaciones del SG90 (del datasheet)

> **Importante:** existen múltiples versiones y clones del SG90 con
> especificaciones ligeramente distintas. Los valores del datasheet son
> punto de partida — siempre se requiere calibración empírica.

| Parámetro | Valor |
|---|---|
| Peso | 9g |
| Voltaje de operación | 4.8V – 6V |
| Frecuencia de señal PWM | **50 Hz (período 20ms)** |
| Pulso mínimo (datasheet) | 500µs – 1000µs → 0° |
| Pulso centro (datasheet) | 1500µs → 90° |
| Pulso máximo (datasheet) | 2000µs – 2400µs → 180° |
| Dead band width | 10µs |
| Velocidad | 0.1 seg / 60° a 4.8V |
| Torque | 1.8 kg·cm a 4.8V |
| Corriente máxima | < 600mA |
| Cables | Naranja = señal / Rojo = VCC / Marrón = GND |

### Inconsistencia entre datasheets — lección de ingeniería

Los datasheets consultados muestran rangos distintos:

| Fuente | MIN | CENTRO | MAX |
|---|---|---|---|
| Luxorparts | 1000µs | 1500µs | 2000µs |
| TowerPro original | 500µs | — | 2400µs |
| Datasheet genérico | 1000µs | 1500µs | 2000µs |

Esto ocurre porque el SG90 es ampliamente clonado. La unidad usada
en esta práctica resultó tener un rango calibrado de **665µs – 2400µs**,
diferente a todos los datasheets — comportamiento completamente normal
para servos de gama baja.

---

## Hardware necesario

- Raspberry Pi Pico (RP2040) con cristal externo X12.000 (12 MHz)
- Servomotor SG90
- **Fuente externa de 5V** (el servo no debe alimentarse del Pico)
- CH552 Multiprotocol CMSIS-DAP (para flashear por SWD con OpenOCD)
- 3 cables dupont para SWD (SWDIO, SWDCLK, GND)
- Cables dupont para conexión del servo

### ⚠️ Advertencia de alimentación

```
Corriente pico del SG90: hasta 600mA
Pin 3.3V del Pico:        ~300mA máximo
Pin VBUS (5V):            limitado por el host USB

NUNCA alimentes el servo desde los pines del Pico.
Usa siempre una fuente externa de 5V con GND común.
```

### Conexión física

```
SG90 cable rojo   (VCC)    → 5V externo
SG90 cable marrón (GND)    → GND externo + GND del Pico (GND común)
SG90 cable naranja (señal) → GP22 del Pico

CH552 SWDIO  → Pin 34 del Pico
CH552 SWDCLK → Pin 32 del Pico
CH552 GND    → GND común
```

```
         ┌─────────────────────────────────┐
         │         Raspberry Pi Pico        │
         │                                  │
         │  GP22 ──────────────────── naranja│──► SG90 señal
         │  GND  ──────────────────── marrón│──► SG90 GND ──┐
         │                                  │               │
         └─────────────────────────────────┘               │
                                                            │
         Fuente 5V externa ─────────────────────── rojo ──► SG90 VCC
         Fuente 5V GND     ─────────────────────────────────┘
```

> La señal PWM del Pico es de 3.3V — el SG90 acepta señal de 3.3V
> aunque opere a 5V. No necesitas level shifter para la señal.

---

## Conceptos matemáticos — desarrollo completo

### El oscilador del sistema

El RP2040 en este board tiene un cristal externo de **12 MHz (X12.000)**.
El PLL interno multiplica esta referencia para generar el clock del sistema:

```
XOSC = 12 MHz  (cristal externo)
PLL multiplica internamente
F_clk = 125 MHz  (clock del sistema por defecto)

El SDK configura el PLL automáticamente al arrancar.
Para el cálculo del PWM siempre usamos F_clk = 125 MHz.
```

---

### Paso 1 — Determinar la frecuencia objetivo

El SG90 requiere exactamente **50 Hz** (período de 20ms).
Esta frecuencia es crítica — no es como el LED donde cualquier valor
por encima de 60Hz es válido. Un servo que no recibe 50 Hz pierde
referencia o se daña.

```
F_pwm = 50 Hz

Período = 1 / F_pwm = 1 / 50 = 0.02 s = 20 ms = 20,000 µs
```

---

### Paso 2 — Elegir la resolución del contador (estrategia de diseño)

**Objetivo:** que 1 count del contador = 1 microsegundo.

¿Por qué? Porque el SG90 se controla con pulsos en µs.
Si 1 count = 1µs, el mapeo de ángulos a counts es directo:

```
500µs → 500 counts   (sin conversión)
1500µs → 1500 counts
2400µs → 2400 counts
```

Para que 1 count = 1µs, el contador debe incrementar
**1,000,000 veces por segundo (1 MHz)**:

```
F_contador_deseado = 1,000,000 Hz = 1 MHz
```

---

### Paso 3 — Calcular DIV

```
F_contador = F_clk / DIV

Despejando DIV:
DIV = F_clk / F_contador_deseado
DIV = 125,000,000 / 1,000,000
DIV = 125.0

✓ Con DIV = 125.0 el contador incrementa a 1 MHz → 1 count = 1µs
```

---

### Paso 4 — Calcular WRAP en Phase Correct mode

En **Phase Correct** el contador sube de 0 a WRAP y luego **baja** de WRAP a 0.
El período completo es `WRAP × 2` counts:

```
Período deseado = 20,000 µs = 20,000 counts (porque 1 count = 1µs)

En Phase Correct:
  Período = WRAP × 2
  20,000  = WRAP × 2
  WRAP    = 10,000

Ajuste por indexación desde 0 (el SDK cuenta de 0 a WRAP inclusive):
  WRAP = 10,000 - 1 = 9,999
```

**Verificación cruzada:**
```
F_pwm = F_clk / (DIV × WRAP × 2)
F_pwm = 125,000,000 / (125.0 × 10,000 × 2)
F_pwm = 125,000,000 / 2,500,000
F_pwm = 50 Hz  ✓
```

---

### Paso 5 — Escalar los pulsos en Phase Correct

En Phase Correct el WRAP es la mitad del modo Free Running.
Por lo tanto los counts de los pulsos también se escalan a la mitad:

```
Free Running (WRAP=19999):        Phase Correct (WRAP=9999):
  0°   → 500  counts               0°   → 250  counts
  90°  → 1500 counts               90°  → 750  counts
  180° → 2400 counts               180° → 1200 counts
```

**¿Por qué se dividen entre 2?**
Porque en Phase Correct el contador llega a WRAP y regresa — el pulso
dura el doble de counts para producir el mismo ancho de tiempo:

```
Queremos un pulso de 1500µs:

Free Running:  LEVEL = 1500 counts × 1µs/count = 1500µs  ✓
Phase Correct: LEVEL = 750 counts × 1µs/count × 2 (ida y vuelta) = 1500µs  ✓
```

---

### Paso 6 — Fórmula de mapeo ángulo → counts

Interpolación lineal entre el rango mínimo y máximo:

```
counts = SERVO_MIN_US + (angulo / 180.0) × (SERVO_MAX_US - SERVO_MIN_US)

donde (en Phase Correct, WRAP=9999):
  SERVO_MIN_US = valor calibrado para 0°   (en counts escalados)
  SERVO_MAX_US = valor calibrado para 180° (en counts escalados)
  angulo       = 0.0 a 180.0 grados

Verificación con valores calibrados (MIN=332, MAX=1200):
  0°:   332 + (0   / 180) × (1200 - 332) = 332   ✓
  90°:  332 + (90  / 180) × (1200 - 332) = 332 + 434 = 766
  180°: 332 + (180 / 180) × (1200 - 332) = 1200  ✓
```

---

### Free Running vs Phase Correct — comparación visual

```
FREE RUNNING (WRAP = 19999):
Contador: 0 ──────────────────────────► 19999 ──► 0
Señal:    ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
          ↑ pulso al INICIO del período

PHASE CORRECT (WRAP = 9999):
Contador: 0 ──────────► 9999 ◄────────── 0
Señal:    ░░░░░░░░░░░░░████░░░░░░░░░░░░░░░░░░░░
                       ↑ pulso CENTRADO en el período

Ambos generan: F_pwm = 50 Hz, Período = 20ms
Diferencia: forma y posición del pulso dentro del período
```

---

## Calibración empírica del SG90

### ¿Por qué es necesaria?

El SG90 es ampliamente clonado con tolerancias de fabricación amplias:
- Potenciómetro interno con variación de ±15%
- Circuito de control interno varía entre fabricantes
- Dead band de 10µs — el servo no distingue diferencias < 10µs

**Resultado:** dos SG90 del mismo lote pueden tener rangos reales distintos.

### Procedimiento de calibración del mínimo (0°)

```
1. Fijar el servo en 0° con while(1) { set_servo_angle(slice, ch, 0); }
2. Observar posición del brazo
3. Ajustar SERVO_MIN_US en pasos de 25µs:
   - Brazo no llega al extremo → bajar SERVO_MIN_US
   - Brazo hace ruido (grinding) → subir SERVO_MIN_US
4. Repetir hasta que quede en el extremo físico sin ruido
```

**Resultado de calibración de esta unidad:**
```
Datasheet sugería:  500µs
Valor calibrado:    665µs  (en Phase Correct: 332 counts)
Diferencia:         165µs  ← tolerancia de fabricación del clon
```

### Procedimiento de calibración del máximo (180°)

```
1. Fijar el servo en 180° con while(1) { set_servo_angle(slice, ch, 180); }
2. Observar posición del brazo
3. Ajustar SERVO_MAX_US en pasos de 25µs:
   - Brazo no llega al extremo → subir SERVO_MAX_US
   - Brazo hace ruido (grinding) → bajar SERVO_MAX_US
4. Repetir hasta que quede en el extremo físico sin ruido
```

### Verificación del centro (90°)

Una vez calibrados MIN y MAX, verificar que 90° es simétrico:

```
Centro teórico = (SERVO_MIN_US + SERVO_MAX_US) / 2

Ejemplo con MIN=665, MAX=2400:
Centro = (665 + 2400) / 2 = 1532µs

Idealmente debería ser 1500µs.
Una diferencia de 32µs es aceptable para un SG90 genérico.
```

---

## Estructura del proyecto

```
RPPicoProjects/
└── PWMServo/
    ├── pico_sdk_import.cmake
    ├── CMakeLists.txt
    ├── PWMServo.c
    └── build/
        ├── PWMServo.uf2
        └── PWMServo.elf
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

set(PROJECT_NAME    "PWMServo")
set(PROJECT_SOURCES "PWMServo.c")
set(PICO_BOARD      "pico")

set(PROJECT_LIBS
    pico_stdlib
    hardware_pwm
)

project(${PROJECT_NAME})
pico_sdk_init()
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})
pico_add_extra_outputs(${PROJECT_NAME})
```

> `hardware_gpio` no es necesario en el CMakeLists — el SDK lo incluye
> transitivamente desde `hardware_pwm`. Se puede incluir explícitamente
> para mayor claridad, pero no es obligatorio.

---

## Código fuente completo

```c
/**
 * @file PWMServo.c
 * @brief Control de posición de servomotor SG90 mediante PWM Phase Correct
 *
 * @details
 * Genera una señal PWM de exactamente 50Hz usando Phase Correct mode.
 * El periférico PWM del RP2040 se configura con:
 *   - DIV  = 125.0  → contador a 1MHz (1 count = 1µs)
 *   - WRAP = 9999   → período = WRAP × 2 = 20000µs = 20ms = 50Hz
 *
 * Los valores SERVO_MIN_US y SERVO_MAX_US están calibrados empíricamente
 * para esta unidad específica del SG90. Ajustar según el servo real.
 *
 * @author   obviousfancy
 * @date     2026-06-28
 * @board    pico (RP2040)
 * @sdk      Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

/* ─── Defines ──────────────────────────────────────────── */
#define PWM_SERVO_PIN   22          // GPIO de señal PWM al servo

// Configuración del PWM
// DIV = 125.0 → F_contador = 125MHz / 125 = 1MHz → 1 count = 1µs
// WRAP = 9999 → Período = 9999 × 2 = ~20000µs = 50Hz (Phase Correct)
#define PWM_DIV         125.0f
#define PWM_WRAP        9999

// Pulsos calibrados empíricamente para este SG90 específico
// Nota: el datasheet sugiere 500–2400µs pero este clon responde a 665–2400µs
// En Phase Correct los counts se dividen entre 2 respecto a Free Running:
//   665µs  / 2 = 332 counts  → 0°
//   2400µs / 2 = 1200 counts → 180°
#define SERVO_MIN_US    332         // counts para 0°   (665µs reales)
#define SERVO_MAX_US    1200        // counts para 180° (2400µs reales)

/* ─── Functions ─────────────────────────────────────────── */

/**
 * @brief Posiciona el servo en el ángulo especificado.
 *
 * @details
 * Convierte el ángulo en grados a counts del contador PWM mediante
 * interpolación lineal entre SERVO_MIN_US y SERVO_MAX_US:
 *
 *   counts = MIN + (angulo / 180.0) × (MAX - MIN)
 *
 * @param slice   Número de slice PWM (obtenido con pwm_gpio_to_slice_num)
 * @param channel Canal PWM (obtenido con pwm_gpio_to_channel)
 * @param angle   Ángulo deseado en grados (0.0 a 180.0)
 */
void set_servo_angle(uint slice, uint channel, float angle) {
    uint counts = SERVO_MIN_US + (uint)((angle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US));
    pwm_set_chan_level(slice, channel, counts);
}

/* ─── Main ─────────────────────────────────────────────── */
int main() {

    // 1. Obtener slice y canal del GPIO
    uint slice   = pwm_gpio_to_slice_num(PWM_SERVO_PIN);
    uint channel = pwm_gpio_to_channel(PWM_SERVO_PIN);

    // 2. Conectar GPIO al periférico PWM
    gpio_set_function(PWM_SERVO_PIN, GPIO_FUNC_PWM);

    // 3. Configurar PWM por objeto (necesario para Phase Correct)
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_DIV);           // DIV = 125.0 → 1MHz
    pwm_config_set_wrap(&config, PWM_WRAP);             // WRAP = 9999 → 50Hz
    pwm_config_set_phase_correct(&config, true);        // Phase Correct ON

    // 4. Aplicar configuración y activar el slice
    pwm_init(slice, &config, true);

    // 5. Posición inicial: centro (90°) y esperar estabilización
    set_servo_angle(slice, channel, 90);
    sleep_ms(1000);

    // 6. Barrer de 0° a 180° y viceversa
    while (1) {
        // Subida: 0° → 180°
        for (int i = 0; i <= 180; i++) {
            set_servo_angle(slice, channel, (float)i);
            sleep_ms(15);   // SG90: 0.1 seg/60° → ~2.5ms/° mínimo
        }                   // 15ms da movimiento suave y controlado

        sleep_ms(500);      // pausa en el extremo

        // Bajada: 180° → 0°
        for (int j = 180; j >= 0; j--) {
            set_servo_angle(slice, channel, (float)j);
            sleep_ms(15);
        }

        sleep_ms(500);      // pausa en el extremo
    }
}
```

---

## Por qué se usa la API por objeto aquí

En la práctica del LED se usó la API directa:
```c
pwm_set_wrap(slice, 255);
pwm_set_clkdiv(slice, 1.0f);
pwm_set_enabled(slice, true);
```

Para el servo se necesita la **API por objeto** porque `phase_correct`
solo existe en `pwm_config_set_phase_correct()` — no hay función directa
equivalente para activarlo después de que el slice está corriendo:

```c
pwm_config config = pwm_get_default_config();   // obtener defaults
pwm_config_set_clkdiv(&config, PWM_DIV);        // modificar config
pwm_config_set_wrap(&config, PWM_WRAP);         // modificar config
pwm_config_set_phase_correct(&config, true);    // modificar config
pwm_init(slice, &config, true);                 // aplicar TODO junto
```

`pwm_init()` aplica toda la configuración atómicamente y activa el slice.

---

## Tabla de diferencias vs práctica anterior (LED)

| Parámetro | Práctica LED | Práctica Servo |
|---|---|---|
| Frecuencia objetivo | cualquiera > 60Hz | **50 Hz exactos** |
| DIV | 1.0 (default) | **125.0 (calculado)** |
| WRAP | 255 | **9999** |
| Phase Correct | no | **sí** |
| API de init | directa | **por objeto** |
| LEVEL cambia | en el loop (brillo) | según ángulo |
| Función auxiliar | ninguna | **set_servo_angle()** |
| Calibración | no requerida | **siempre requerida** |

---

## Compilar y flashear

```bash
cd ~/Documents/tuusuario/RPPicoProjects/PWMServo
pico-flash
```

---

## Errores comunes

### El servo vibra o hace ruido constante
La frecuencia no es exactamente 50Hz — verificar DIV y WRAP:
```
F_pwm = 125,000,000 / (125.0 × 10,000 × 2) = 50 Hz  ← correcto
```

### El servo no llega a los extremos físicos
Los valores MIN/MAX del datasheet no coinciden con tu unidad:
```
→ Calibrar empíricamente en pasos de 25µs
→ Normal en clones del SG90
```

### El servo va al extremo pero hace ruido de grinding
El pulso supera el límite mecánico del servo:
```
SERVO_MIN_US demasiado bajo → aumentarlo
SERVO_MAX_US demasiado alto → disminuirlo
```

### Error de compilación: DIV no definido
Nombre incorrecto del define:
```c
// MAL
pwm_config_set_clkdiv(&config, DIV);

// BIEN — usar el nombre exacto del define
pwm_config_set_clkdiv(&config, PWM_DIV);
```

### El for de bajada no funciona
Error de syntax en la condición:
```c
// MAL — "i j" no es sintaxis válida
for(int j = 0; i j > 180; j--)

// BIEN — empieza en 180 y decrementa
for(int j = 180; j >= 0; j--)
```

---

## Resumen matemático completo

```
DATOS:
  F_clk  = 125 MHz  (PLL desde XOSC de 12 MHz)
  F_pwm  = 50 Hz    (requerimiento del SG90)
  Período = 20,000 µs

OBJETIVO: 1 count = 1µs
  F_contador = 1 MHz

CÁLCULO DIV:
  DIV = F_clk / F_contador
  DIV = 125,000,000 / 1,000,000
  DIV = 125.0

CÁLCULO WRAP (Phase Correct):
  Período = WRAP × 2
  WRAP = Período / 2 - 1
  WRAP = 20,000 / 2 - 1
  WRAP = 9,999

VERIFICACIÓN:
  F_pwm = F_clk / (DIV × WRAP × 2)
  F_pwm = 125,000,000 / (125 × 10,000 × 2)
  F_pwm = 50 Hz ✓

MAPEO ÁNGULO → COUNTS:
  counts = MIN + (angle / 180.0) × (MAX - MIN)

CALIBRACIÓN EMPÍRICA (esta unidad):
  SERVO_MIN_US = 332 counts  (665µs reales, datasheet sugería 500µs)
  SERVO_MAX_US = 1200 counts (2400µs reales)
  Diferencia mínimo: 165µs → tolerancia de fabricación del clon
```

---

## Lección clave de esta práctica

> En sistemas embebidos los datasheets son el punto de partida,
> no la verdad absoluta. El hardware real siempre requiere calibración
> empírica, especialmente en componentes de bajo costo con tolerancias
> de fabricación amplias. Un ingeniero embedded siempre valida con
> el hardware real en mano.

---

## Siguiente práctica sugerida

- **Práctica 06** — Buzzer Musical: frecuencia variable para generar notas
- **Práctica 07** — Motor DC: control de velocidad y dirección
- **Práctica 08** — ADC: controlar el servo con un potenciómetro
