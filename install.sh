#!/bin/bash

# ╔══════════════════════════════════════════════════════════════╗
# ║          install.sh — Obviousfancy Pico Environment          ║
# ║     Setup completo del entorno de desarrollo para            ║
# ║     Raspberry Pi Pico SDK en Ubuntu 24.04 / 26.04            ║
# ║              Obviousfancy Lab                                 ║
# ╚══════════════════════════════════════════════════════════════╝
#
# Uso:
#   chmod +x install.sh
#   ./install.sh
#
# Lo que hace:
#   1. Instala dependencias del sistema (cmake, gcc-arm, openocd, etc.)
#   2. Instala questionary (Python) para pico-new
#   3. Clona el Raspberry Pi Pico SDK y sus submódulos
#   4. Crea la estructura de carpetas del entorno
#   5. Copia los scripts (flash.sh, pico-new.py, templates)
#   6. Configura variables de entorno y alias en ~/.bashrc
#   7. Configura reglas udev para el CH552 CMSIS-DAP

set -e  # detener si cualquier comando falla

# ─────────────────────────────────────────────────────────────
# COLORES PARA OUTPUT
# ─────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # sin color

ok()   { echo -e "${GREEN}  [OK]${NC} $1"; }
info() { echo -e "${CYAN}  [INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}  [WARN]${NC} $1"; }
fail() { echo -e "${RED}  [ERROR]${NC} $1"; exit 1; }
step() { echo -e "\n${BOLD}${CYAN}▶ $1${NC}"; }

# ─────────────────────────────────────────────────────────────
# CONFIGURACIÓN — ajusta estas rutas si lo necesitas
# ─────────────────────────────────────────────────────────────
INSTALL_DIR="$HOME/Documents/obviousfancy"
PROJECTS_DIR="$INSTALL_DIR/RPPicoProjects"
SCRIPTS_DIR="$PROJECTS_DIR/scripts"
SDK_DIR="$INSTALL_DIR/pico-sdk"

# Directorio donde está este script (fuente de los archivos a copiar)
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ─────────────────────────────────────────────────────────────
# BANNER
# ─────────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║       Obviousfancy Pico Environment — Installer              ║${NC}"
echo -e "${BOLD}${CYAN}║       Raspberry Pi Pico SDK Setup para Ubuntu                ║${NC}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "  ${BOLD}Directorio de instalación:${NC} $INSTALL_DIR"
echo -e "  ${BOLD}SDK:${NC}                       $SDK_DIR"
echo -e "  ${BOLD}Proyectos:${NC}                 $PROJECTS_DIR"
echo -e "  ${BOLD}Scripts:${NC}                   $SCRIPTS_DIR"
echo ""

# ─────────────────────────────────────────────────────────────
# VERIFICAR UBUNTU
# ─────────────────────────────────────────────────────────────
step "Verificando sistema operativo"

if [ ! -f /etc/os-release ]; then
    fail "No se puede determinar el sistema operativo"
fi

source /etc/os-release
if [[ "$ID" != "ubuntu" ]]; then
    warn "Este script está diseñado para Ubuntu. Sistema detectado: $ID"
    read -p "  ¿Continuar de todas formas? [s/N]: " yn
    [[ "$yn" =~ ^[sS]$ ]] || exit 0
fi

ok "Sistema: $PRETTY_NAME"

# ─────────────────────────────────────────────────────────────
# PASO 1 — DEPENDENCIAS DEL SISTEMA
# ─────────────────────────────────────────────────────────────
step "Paso 1 — Instalando dependencias del sistema"

info "Actualizando lista de paquetes..."
sudo apt-get update -qq

PACKAGES=(
    cmake
    python3
    python3-pip
    build-essential
    gcc-arm-none-eabi
    libnewlib-arm-none-eabi
    libstdc++-arm-none-eabi-newlib
    git
    openocd
    picocom
)

info "Instalando paquetes: ${PACKAGES[*]}"
sudo apt-get install -y "${PACKAGES[@]}"

# Verificar instalaciones críticas
for tool in cmake arm-none-eabi-gcc openocd git python3; do
    if command -v $tool &> /dev/null; then
        ok "$tool: $(${tool} --version 2>&1 | head -1)"
    else
        fail "$tool no se instaló correctamente"
    fi
done

# ─────────────────────────────────────────────────────────────
# PASO 2 — QUESTIONARY (Python)
# ─────────────────────────────────────────────────────────────
step "Paso 2 — Instalando dependencias Python"

info "Instalando questionary..."
pip install questionary --break-system-packages -q

if python3 -c "import questionary" 2>/dev/null; then
    ok "questionary instalado correctamente"
else
    fail "questionary no se instaló correctamente"
fi

# ─────────────────────────────────────────────────────────────
# PASO 3 — CLONAR EL SDK
# ─────────────────────────────────────────────────────────────
step "Paso 3 — Raspberry Pi Pico SDK"

if [ -d "$SDK_DIR" ]; then
    warn "El SDK ya existe en $SDK_DIR"
    read -p "  ¿Actualizar el SDK existente? [s/N]: " yn
    if [[ "$yn" =~ ^[sS]$ ]]; then
        info "Actualizando SDK..."
        cd "$SDK_DIR"
        git pull
        git submodule update --init
        ok "SDK actualizado"
    else
        ok "SDK existente conservado"
    fi
else
    info "Clonando pico-sdk..."
    git clone https://github.com/raspberrypi/pico-sdk.git "$SDK_DIR"

    info "Inicializando submódulos (TinyUSB, BTstack, lwIP, mbedtls)..."
    cd "$SDK_DIR"
    git submodule update --init

    ok "SDK clonado en $SDK_DIR"
fi

# Verificar que el SDK es válido
if [ ! -f "$SDK_DIR/pico_sdk_version.cmake" ]; then
    fail "El SDK no parece estar completo — falta pico_sdk_version.cmake"
fi

SDK_VERSION=$(grep "PICO_SDK_VERSION_STRING" "$SDK_DIR/pico_sdk_version.cmake" | \
    grep -v "if\|set.*VERSION_STRING" | head -1 | \
    sed 's/.*"\(.*\)".*/\1/')
ok "Versión del SDK detectada: $SDK_VERSION"

# ─────────────────────────────────────────────────────────────
# PASO 4 — ESTRUCTURA DE CARPETAS
# ─────────────────────────────────────────────────────────────
step "Paso 4 — Creando estructura de carpetas"

mkdir -p "$INSTALL_DIR"
mkdir -p "$PROJECTS_DIR"
mkdir -p "$SCRIPTS_DIR/templates"

ok "Estructura creada:"
ok "  $INSTALL_DIR"
ok "  $PROJECTS_DIR"
ok "  $SCRIPTS_DIR"
ok "  $SCRIPTS_DIR/templates"

# ─────────────────────────────────────────────────────────────
# PASO 5 — COPIAR SCRIPTS Y TEMPLATES
# ─────────────────────────────────────────────────────────────
step "Paso 5 — Copiando scripts y templates"

# flash.sh
if [ -f "$REPO_DIR/scripts/flash.sh" ]; then
    cp "$REPO_DIR/scripts/flash.sh" "$SCRIPTS_DIR/flash.sh"
    chmod +x "$SCRIPTS_DIR/flash.sh"
    ok "flash.sh copiado y con permisos de ejecución"
else
    warn "No se encontró scripts/flash.sh en $REPO_DIR — cópialo manualmente"
fi

# pico-new.py
if [ -f "$REPO_DIR/scripts/pico-new.py" ]; then
    cp "$REPO_DIR/scripts/pico-new.py" "$SCRIPTS_DIR/pico-new.py"
    chmod +x "$SCRIPTS_DIR/pico-new.py"
    ok "pico-new.py copiado"
else
    warn "No se encontró scripts/pico-new.py en $REPO_DIR — cópialo manualmente"
fi

# Templates
if [ -d "$REPO_DIR/scripts/templates" ]; then
    cp "$REPO_DIR/scripts/templates/"* "$SCRIPTS_DIR/templates/"
    ok "Templates copiados"
else
    warn "No se encontró scripts/templates/ en $REPO_DIR — cópialos manualmente"
fi

# ─────────────────────────────────────────────────────────────
# PASO 6 — VARIABLES DE ENTORNO Y ALIAS
# ─────────────────────────────────────────────────────────────
step "Paso 6 — Configurando variables de entorno y alias"

BASHRC="$HOME/.bashrc"
MARKER="# ── Obviousfancy Pico Environment ──"

# Verificar si ya existe la configuración
if grep -q "$MARKER" "$BASHRC" 2>/dev/null; then
    warn "La configuración ya existe en ~/.bashrc — omitiendo"
else
    cat >> "$BASHRC" << EOF

$MARKER
export PICO_SDK_PATH="$SDK_DIR"
alias pico-flash="$SCRIPTS_DIR/flash.sh"
alias pico-new="python3 $SCRIPTS_DIR/pico-new.py"
# ── Fin Obviousfancy Pico Environment ──
EOF
    ok "Variables y alias agregados a ~/.bashrc"
fi

# Cargar en la sesión actual
export PICO_SDK_PATH="$SDK_DIR"

# ─────────────────────────────────────────────────────────────
# PASO 7 — REGLAS UDEV PARA CH552 CMSIS-DAP
# ─────────────────────────────────────────────────────────────
step "Paso 7 — Configurando reglas udev para CH552 CMSIS-DAP"

UDEV_FILE="/etc/udev/rules.d/99-pico-dev.rules"

sudo tee "$UDEV_FILE" > /dev/null << 'UDEV'
# CH552 Multiprotocol CMSIS-DAP (QinHeng Electronics)
SUBSYSTEM=="usb", ATTR{idVendor}=="1a86", ATTR{idProduct}=="8011", MODE="0666", GROUP="plugdev", TAG+="uaccess"

# Raspberry Pi Pico en modo BOOTSEL (RP2040)
SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="0003", MODE="0666", GROUP="plugdev", TAG+="uaccess"

# Raspberry Pi Pico 2 en modo BOOTSEL (RP2350)
SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="000f", MODE="0666", GROUP="plugdev", TAG+="uaccess"

# Raspberry Pi Pico como dispositivo serial (CDC)
SUBSYSTEM=="tty", ATTRS{idVendor}=="2e8a", MODE="0666", GROUP="plugdev", TAG+="uaccess"
UDEV

sudo udevadm control --reload-rules
sudo udevadm trigger

# Agregar usuario al grupo plugdev si no está
if groups "$USER" | grep -q plugdev; then
    ok "Usuario $USER ya está en el grupo plugdev"
else
    sudo usermod -aG plugdev "$USER"
    warn "Usuario $USER agregado a plugdev — necesitas cerrar sesión y volver a entrar"
fi

ok "Reglas udev configuradas en $UDEV_FILE"

# ─────────────────────────────────────────────────────────────
# VERIFICACIÓN FINAL
# ─────────────────────────────────────────────────────────────
step "Verificación final del entorno"

ERRORS=0

# SDK
if [ -f "$SDK_DIR/pico_sdk_version.cmake" ]; then
    ok "SDK: $SDK_DIR"
else
    warn "SDK no encontrado en $SDK_DIR"
    ERRORS=$((ERRORS + 1))
fi

# PICO_SDK_PATH
if [ -n "$PICO_SDK_PATH" ]; then
    ok "PICO_SDK_PATH: $PICO_SDK_PATH"
else
    warn "PICO_SDK_PATH no está definido en esta sesión"
    ERRORS=$((ERRORS + 1))
fi

# Scripts
for script in "flash.sh" "pico-new.py"; do
    if [ -f "$SCRIPTS_DIR/$script" ]; then
        ok "$script: $SCRIPTS_DIR/$script"
    else
        warn "$script no encontrado en $SCRIPTS_DIR"
        ERRORS=$((ERRORS + 1))
    fi
done

# Templates
for tmpl in "CMakeLists.txt.template" "main.c.template"; do
    if [ -f "$SCRIPTS_DIR/templates/$tmpl" ]; then
        ok "$tmpl: encontrado"
    else
        warn "$tmpl no encontrado en $SCRIPTS_DIR/templates/"
        ERRORS=$((ERRORS + 1))
    fi
done

# Herramientas
for tool in cmake arm-none-eabi-gcc openocd python3; do
    if command -v $tool &> /dev/null; then
        ok "$tool: disponible"
    else
        warn "$tool: NO disponible"
        ERRORS=$((ERRORS + 1))
    fi
done

# ─────────────────────────────────────────────────────────────
# RESUMEN
# ─────────────────────────────────────────────────────────────
echo ""
if [ $ERRORS -eq 0 ]; then
    echo -e "${BOLD}${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}${GREEN}║            ✓ Instalación completada sin errores              ║${NC}"
    echo -e "${BOLD}${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
else
    echo -e "${BOLD}${YELLOW}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BOLD}${YELLOW}║    Instalación completada con $ERRORS advertencia(s)           ║${NC}"
    echo -e "${BOLD}${YELLOW}╚══════════════════════════════════════════════════════════════╝${NC}"
fi

echo ""
echo -e "  ${BOLD}Próximos pasos:${NC}"
echo -e "  1. Cierra y vuelve a abrir la terminal (o corre: source ~/.bashrc)"
echo -e "  2. Verifica con:  echo \$PICO_SDK_PATH"
echo -e "  3. Crea tu primer proyecto con:  pico-new"
echo -e "  4. Compila y flashea con:        pico-flash"
echo ""
echo -e "  ${BOLD}Estructura instalada:${NC}"
echo -e "  $INSTALL_DIR/"
echo -e "  ├── pico-sdk/              ← SDK de Raspberry Pi"
echo -e "  └── RPPicoProjects/"
echo -e "      └── scripts/"
echo -e "          ├── flash.sh       ← alias: pico-flash"
echo -e "          ├── pico-new.py    ← alias: pico-new"
echo -e "          └── templates/"
echo -e "              ├── CMakeLists.txt.template"
echo -e "              └── main.c.template"
echo ""
