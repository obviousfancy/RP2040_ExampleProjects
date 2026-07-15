# Ejemplos para RP2040

Este directorio contiene ejemplos prácticos de programación para el microcontrolador RP2040.

## Estructura de Ejemplos

Cada ejemplo incluye:
- Código fuente en C o ensamblador
- CMakeLists.txt para compilación
- Documentación específica del ejemplo
- Instrucciones de uso

## Prácticas Disponibles

### GPIO - Entradas Digitales
Lectura de entradas digitales y control de salidas.

### UART - Comunicación Serial
Transmisión y recepción de datos por puerto serial.

### ADC - Sensor de Temperatura
Lectura del sensor de temperatura interno mediante ADC.

### I2C - Display OLED
Comunicación I²C con display OLED para visualización de datos.

## Requisitos

- Placa con RP2040 (Raspberry Pi Pico, etc.)
- SDK de Pico (pico-sdk)
- CMake 3.13 o superior
- Compilador ARM GCC

## Compilación

```bash
mkdir build
cd build
cmake ..
make
```

## Carga del Programa

1. Mantener presionado el botón BOOTSEL
2. Conectar la placa por USB
3. Soltar BOOTSEL
4. Copiar el archivo .uf2 al dispositivo de almacenamiento que aparece

O usar picotool:

```bash
picotool load -f example.uf2
picotool reboot
```

## Documentación

Consulta la [documentación completa](../docs/index.md) para más detalles sobre cada práctica.
