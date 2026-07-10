.syntax unified

@ ============================================================
@  Direcciones de registros (RP2040 datasheet, secciones 2.2 y 2.19)
@ ============================================================
.equ IO_BANK0_BASE,   0x40014000
.equ GPIO25_CTRL,     IO_BANK0_BASE + (25 * 8) + 4   @ = 0x400140CC
.equ FUNCSEL_SIO,     5

.equ SIO_BASE,        0xd0000000
.equ GPIO_OE_SET,     SIO_BASE + 0x24                @ = 0xd0000024
.equ GPIO_OUT_SET,    SIO_BASE + 0x14                @ = 0xd0000014
.equ GPIO_OUT_CLR,    SIO_BASE + 0x18                @ = 0xd0000018

.equ LED_MASK,        (1 << 25)                      @ = 0x02000000
.equ DELAY_COUNT,     40000000                        @ = 4 segundos

.thumb_func
.global main
main:
    @ 1. Pin muxing: conectar el GPIO25 al bloque SIO
    ldr r0, =GPIO25_CTRL
    movs r1, #FUNCSEL_SIO
    str r1, [r0]

    @ 2. Habilitar GPIO25 como salida
    ldr r0, =GPIO_OE_SET
    ldr r1, =LED_MASK
    str r1, [r0]

loop:
    @ 3. Encender el LED
    ldr r0, =GPIO_OUT_SET
    ldr r1, =LED_MASK
    str r1, [r0]
    bl delay

    @ 4. Apagar el LED
    ldr r0, =GPIO_OUT_CLR
    ldr r1, =LED_MASK
    str r1, [r0]
    bl delay

    b loop

@ ============================================================
@  Subrutina de retardo (busy-wait)
@ ============================================================
.thumb_func
delay:
    ldr r2, =DELAY_COUNT
delay_loop:
    subs r2, r2, #1
    bne delay_loop
    bx lr

.data