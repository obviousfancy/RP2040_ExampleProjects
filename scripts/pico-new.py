#!/usr/bin/env python3
"""
╔══════════════════════════════════════════════════════╗
║           pico-new — Scaffolding CLI                 ║
║     Raspberry Pi Pico SDK Project Generator          ║
║              Obviousfancy Lab                        ║
╚══════════════════════════════════════════════════════╝

Uso:
    python3 pico-new.py
    o si está en PATH con alias:
    pico-new
"""

import os
import re
import sys
import shutil
import subprocess
from datetime import date
from pathlib import Path

try:
    import questionary
    from questionary import Style
except ImportError:
    print("[ERROR] Falta instalar questionary:")
    print("        pip install questionary --break-system-packages")
    sys.exit(1)

# ─────────────────────────────────────────────────────────────
# CONFIGURACIÓN GLOBAL
# ─────────────────────────────────────────────────────────────

SCRIPTS_DIR  = Path(__file__).resolve().parent
TEMPLATES_DIR = SCRIPTS_DIR / "templates"
PICO_SDK_PATH = Path(os.environ.get("PICO_SDK_PATH", ""))

# Nota: el directorio base donde se crean los proyectos YA NO es fijo.
# Se calcula dinámicamente en main() como la carpeta actual (Path.cwd())
# desde donde el usuario ejecuta pico-new, para que el default tenga
# sentido sin importar dónde esté instalado el script.

# ─────────────────────────────────────────────────────────────
# MAPA DE PERIFÉRICOS
# Cada entrada define:
#   - lib:      nombre de la librería del SDK para CMakeLists
#   - include:  header que se agrega al .c
#   - init:     código de inicialización que va dentro de main()
#   - brief:    descripción para el menú
# ─────────────────────────────────────────────────────────────

PERIPHERALS = {
    "GPIO": {
        "lib":     "hardware_gpio",
        "include": '#include "hardware/gpio.h"',
        "init":    "    // gpio_init(PIN);\n    // gpio_set_dir(PIN, GPIO_OUT);",
        "brief":   "GPIO — Control de pines digitales (entrada/salida)",
    },
    "PWM": {
        "lib":     "hardware_pwm",
        "include": '#include "hardware/pwm.h"',
        "init":    "    // uint slice = pwm_gpio_to_slice_num(PIN);\n    // pwm_set_enabled(slice, true);",
        "brief":   "PWM — Señal de ancho de pulso variable",
    },
    "I2C": {
        "lib":     "hardware_i2c",
        "include": '#include "hardware/i2c.h"',
        "init":    "    // i2c_init(i2c0, 100 * 1000);\n    // gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);\n    // gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);",
        "brief":   "I2C — Bus serial de 2 hilos (SDA/SCL)",
    },
    "SPI": {
        "lib":     "hardware_spi",
        "include": '#include "hardware/spi.h"',
        "init":    "    // spi_init(spi0, 1000 * 1000);\n    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);\n    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);\n    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);",
        "brief":   "SPI — Bus serial de 4 hilos (SCK/MOSI/MISO/CS)",
    },
    "UART": {
        "lib":     "hardware_uart",
        "include": '#include "hardware/uart.h"',
        "init":    "       // uart_init(uart0, 115200);",
        "brief":   "UART — Comunicación serial asíncrona",
    },
    "ADC": {
        "lib":     "hardware_adc",
        "include": '#include "hardware/adc.h"',
        "init":    "    // adc_init();\n    // adc_gpio_init(26); // GP26 = ADC0",
        "brief":   "ADC — Lectura analógica (conversor A/D)",
    },
    "TIMER": {
        "lib":     "hardware_timer",
        "include": '#include "hardware/timer.h"',
        "init":    "    // Usa sleep_ms() / sleep_us() de pico_stdlib\n    // o configura alarmas con hardware_alarm_*",
        "brief":   "TIMER — Temporizadores y alarmas de hardware",
    },
    "IRQ": {
        "lib":     "hardware_irq",
        "include": '#include "hardware/irq.h"',
        "init":    "    // irq_set_exclusive_handler(IRQ_NUM, handler);\n    // irq_set_enabled(IRQ_NUM, true);",
        "brief":   "IRQ — Interrupciones de hardware",
    },
    "MultiCore": {
        "lib":     "pico_multicore",
        "include" : '#include "pico/multicore.h"',
        "init":    "    // multicore_launch_core1(core1_entry);\n    // multicore_fifo_push_blocking(&data);",
        "brief":   "MultiCore — Multiples núcleos de procesador",
    },
    "PIO":{
        "lib":     "hardware_pio",
        "include" : '#include "hardware/pio.h\n #include "hardware/clocks.h"\n"',
        "init":    " ",
        "brief":   "PIO — Programación de periféricos",
    },
}

BOARDS = ["pico", "pico2", "pico_w"]

# ─────────────────────────────────────────────────────────────
# ESTILO VISUAL DEL CLI
# ─────────────────────────────────────────────────────────────

CLI_STYLE = Style([
    ("qmark",        "fg:#00d7af bold"),
    ("question",     "fg:#ffffff bold"),
    ("answer",       "fg:#00d7af bold"),
    ("pointer",      "fg:#00d7af bold"),
    ("highlighted",  "fg:#00d7af bold"),
    ("selected",     "fg:#00d7af"),
    ("separator",    "fg:#444444"),
    ("instruction",  "fg:#888888"),
    ("text",         "fg:#ffffff"),
    ("disabled",     "fg:#444444 italic"),
])

# ─────────────────────────────────────────────────────────────
# VALIDACIONES
# ─────────────────────────────────────────────────────────────

def validate_project_name(name: str) -> bool | str:
    """Valida que el nombre sea válido para C y para el sistema de archivos."""
    if not name:
        return "El nombre no puede estar vacío"
    if not re.match(r'^[a-zA-Z0-9][a-zA-Z0-9_]*$', name):
        return "Solo letras, números y guión bajo. Sin espacios. No puede empezar con guión bajo."
    return True


def validate_project_path(path_str: str, project_name: str) -> bool | str:
    """Valida que la ruta sea válida y que el proyecto no exista ya ahí."""
    if not path_str.strip():
        return "La ruta no puede estar vacía"
    path = Path(path_str).expanduser().resolve()
    if (path / project_name).exists():
        return f"Ya existe '{project_name}' en {path}"
    return True

# ─────────────────────────────────────────────────────────────
# GENERACIÓN DE ARCHIVOS
# ─────────────────────────────────────────────────────────────

def generate_cmake(project_name: str, board: str, selected_libs: list[str]) -> str:
    """Genera el contenido del CMakeLists.txt desde el template."""
    template = (TEMPLATES_DIR / "CMakeLists.txt.template").read_text()

    # Construir bloque de librerías (sin pico_stdlib que ya está en el template)
    if selected_libs:
        libs_block = "\n".join(f"    {lib}" for lib in selected_libs)
    else:
        libs_block = "    # (sin periféricos adicionales)"

    return (template
        .replace("{{PROJECT_NAME}}", project_name)
        .replace("{{PICO_BOARD}}",   board)
        .replace("{{PROJECT_LIBS}}", libs_block)
    )


def generate_main_c(project_name: str, board: str, selected_peripherals: list[str], brief: str) -> str:
    """Genera el contenido del archivo .c principal desde el template."""
    template = (TEMPLATES_DIR / "main.c.template").read_text()

    # Construir bloque de includes
    includes = []
    init_blocks = []

    for p in selected_peripherals:
        if p in PERIPHERALS:
            includes.append(PERIPHERALS[p]["include"])
            init_blocks.append(PERIPHERALS[p]["init"])

    includes_str  = "\n".join(includes) if includes else ""
    init_code_str = "\n".join(init_blocks) + "\n" if init_blocks else ""

    return (template
        .replace("{{PROJECT_NAME}}", project_name)
        .replace("{{PROJECT_BRIEF}}", brief)
        .replace("{{DATE}}",         date.today().isoformat())
        .replace("{{PICO_BOARD}}",   board)
        .replace("{{INCLUDES}}",     includes_str)
        .replace("{{INIT_CODE}}",    init_code_str)
    )


def copy_pico_sdk_import(project_path: Path) -> bool:
    """Copia pico_sdk_import.cmake desde el SDK al proyecto."""
    if not PICO_SDK_PATH or not PICO_SDK_PATH.exists():
        print(f"\n  [WARN] PICO_SDK_PATH no encontrado. Copia manualmente:")
        print(f"         cp $PICO_SDK_PATH/external/pico_sdk_import.cmake {project_path}/")
        return False

    src = PICO_SDK_PATH / "external" / "pico_sdk_import.cmake"
    if not src.exists():
        print(f"\n  [WARN] No se encontró pico_sdk_import.cmake en el SDK.")
        return False

    shutil.copy2(src, project_path / "pico_sdk_import.cmake")
    return True


# ─────────────────────────────────────────────────────────────
# FLUJO PRINCIPAL DEL CLI
# ─────────────────────────────────────────────────────────────

def main():
    print("\n╔══════════════════════════════════════════════════════╗")
    print("║           pico-new — Scaffolding CLI                 ║")
    print("║     Raspberry Pi Pico SDK Project Generator          ║")
    print("║              Obviousfancy Lab                        ║")
    print("╚══════════════════════════════════════════════════════╝\n")

    # 1. Nombre del proyecto
    project_name = questionary.text(
        "Nombre del proyecto:",
        validate=validate_project_name,
        style=CLI_STYLE,
    ).ask()

    if project_name is None:
        print("\n[CANCELADO]")
        sys.exit(0)

    # 2. Descripción breve (para Doxygen)
    brief = questionary.text(
        "Descripción breve del proyecto:",
        default=f"Proyecto {project_name} para Raspberry Pi Pico",
        style=CLI_STYLE,
    ).ask()

    if brief is None:
        print("\n[CANCELADO]")
        sys.exit(0)

    # 3. Board objetivo
    board = questionary.select(
        "Board objetivo:",
        choices=BOARDS,
        default="pico",
        style=CLI_STYLE,
    ).ask()

    if board is None:
        print("\n[CANCELADO]")
        sys.exit(0)

    # 4. Periféricos (checkboxes)
    peripheral_choices = [
        questionary.Choice(
            title=PERIPHERALS[p]["brief"],
            value=p,
        )
        for p in PERIPHERALS
    ]

    selected_peripherals = questionary.checkbox(
        "Selecciona los periféricos que usarás (Space para marcar, Enter para confirmar):",
        choices=peripheral_choices,
        style=CLI_STYLE,
    ).ask()

    if selected_peripherals is None:
        print("\n[CANCELADO]")
        sys.exit(0)

    # Mapear periféricos seleccionados a sus librerías
    selected_libs = [PERIPHERALS[p]["lib"] for p in selected_peripherals]

    # 5. Ruta del proyecto
    # Default = carpeta actual desde donde se ejecuta pico-new (no una ruta
    # fija de instalación). Así, si el usuario está parado en, por ejemplo,
    # ~/Documents/Docs/RP2040_ExampleProjects/examples, ese es el default
    # que ve — y si escribe algo encima (ej. le agrega "/i2cprojects"),
    # esa subcarpeta se crea sola gracias al mkdir(parents=True) de abajo.
    default_path = str(Path.cwd())

    ruta_str = questionary.text(
        "Ruta donde crear el proyecto (Enter para usar la carpeta actual):",
        default=default_path,
        style=CLI_STYLE,
    ).ask()

    if ruta_str is None:
        print("\n[CANCELADO]")
        sys.exit(0)

    # Resolver la ruta — expandir ~ y hacer absoluta
    base_path = Path(ruta_str).expanduser().resolve()

    # Validar ruta y nombre combinados
    path_check = validate_project_path(str(base_path), project_name)
    if path_check is not True:
        print(f"\n[ERROR] {path_check}")
        sys.exit(1)

    project_path = base_path / project_name

    # 6. Confirmación
    print(f"\n  Proyecto   : {project_name}")
    print(f"  Board      : {board}")
    print(f"  Periféricos: {', '.join(selected_peripherals) if selected_peripherals else 'ninguno'}")
    print(f"  Destino    : {project_path}\n")

    confirm = questionary.confirm(
        "¿Crear el proyecto?",
        default=True,
        style=CLI_STYLE,
    ).ask()

    if not confirm:
        print("\n[CANCELADO]")
        sys.exit(0)

    # ─── Crear estructura del proyecto ───────────────────────

    # Crear carpeta base si no existe (ruta custom puede no existir)
    project_path.mkdir(parents=True, exist_ok=False)

    # CMakeLists.txt
    cmake_content = generate_cmake(project_name, board, selected_libs)
    (project_path / "CMakeLists.txt").write_text(cmake_content)

    # archivo .c principal (siempre main.c, independiente del nombre del proyecto)
    main_c_content = generate_main_c(project_name, board, selected_peripherals, brief)
    (project_path / "main.c").write_text(main_c_content)

    # pico_sdk_import.cmake
    sdk_copied = copy_pico_sdk_import(project_path)

    # ─── Resumen final ───────────────────────────────────────

    print(f"\n  ✓ {project_path / 'CMakeLists.txt'}")
    print(f"  ✓ {project_path / 'main.c'}")
    if sdk_copied:
        print(f"  ✓ {project_path / 'pico_sdk_import.cmake'}")

    print(f"\n╔══════════════════════════════════════════════════╗")
    print(f"║  Proyecto '{project_name}' creado correctamente  ")
    print(f"╚══════════════════════════════════════════════════╝")
    print(f"\n  Para compilar y flashear:")
    print(f"    cd {project_path}")
    print(f"    pico-flash\n")


if __name__ == "__main__":
    main()
