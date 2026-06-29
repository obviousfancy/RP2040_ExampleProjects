#!/bin/bash

# ─────────────────────────────────────────────
# flash.sh — Build y flasheo automático
# Raspberry Pi Pico SDK + OpenOCD + CMSIS-DAP
# Obviousfancy Lab
# ─────────────────────────────────────────────
# Verificar que CMakeLists.txt existe en la carpeta actual
if [ ! -f "CMakeLists.txt" ]; then
    echo "[ERROR] No se encontró CMakeLists.txt en la carpeta actual."
    echo "[ERROR] Corre pico-flash desde la raíz del proyecto, no desde otra carpeta"
    echo "[ERROR] Carpeta actual: $(pwd)"
    exit 1
fi
# Detecta el nombre del proyecto desde CMakeLists.txt
PROJECT=$(grep "^project(" CMakeLists.txt | sed 's/project(//;s/)//' | tr -d ' ')

# Si el resultado es una variable CMake (ej. ${PROJECT_NAME}), buscar el set()
if [[ "$PROJECT" == *"{"* ]]; then
    PROJECT=$(grep 'set(PROJECT_NAME' CMakeLists.txt | sed 's/.*set(PROJECT_NAME//;s/)//' | tr -d ' "')
fi

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
