/**
 * @file PWMServo.c
 * @brief Proyecto PWMServo para Raspberry Pi Pico
 *
 * @author obviousfancy
 * @date 2026-06-28
 *
 * @board pico
 * @sdk Raspberry Pi Pico SDK 2.2.0
 */

/* ─── Includes ─────────────────────────────────────────── */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

/* ─── Defines ──────────────────────────────────────────── */
#define PWM_SERVO_PIN   22
#define PWM_DIV     125.0f
#define PWM_WRAP    9999
#define SERVO_MIN_US    665
#define SERVO_MAX_US   2350

/* ─── Functions ───────────────────────────────────────────── */
void set_servo_angle(uint slice, uint channel, float angle) {
    uint counts = SERVO_MIN_US + (angle / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);
    pwm_set_chan_level(slice, channel, (uint16_t)counts);
}
/* ─── Main ─────────────────────────────────────────────── */
int main() {

    uint slice = pwm_gpio_to_slice_num(PWM_SERVO_PIN);
    uint channel = pwm_gpio_to_channel(PWM_SERVO_PIN);
    gpio_set_function(PWM_SERVO_PIN, GPIO_FUNC_PWM);

    //Con phase correct
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_DIV); // Divisor de clock para PWM
    pwm_config_set_wrap(&config,PWM_WRAP);  // Cantidad máxima de ticks en el ciclo de trabajo
    pwm_config_set_phase_correct(&config,true);
    pwm_init(slice,&config,true);

    //set_servo_angle(slice, channel, 90); // Inicializa el servo a 0 grados
    sleep_ms(100); // Espera 2 segundos para que el servo se estabilice
    while (1) {

        set_servo_angle(slice, channel,90); // Inicializa el servo a 0 grados
        /*
        // TODO: lógica principal del proyecto
        for(int i = 0; i <= 180; i++){
            set_servo_angle(slice, channel, i); // Establece el ángulo del servo en grados
            sleep_ms(15); // Espera 50 milisegundos entre cada cambio de ángulo
        }
        for(int j = 180; j >= 0; j--){
            set_servo_angle(slice, channel, j); // Establece el ángulo del servo en grados
            sleep_ms(15); // Espera 50 milisegundos entre cada cambio de ángulo
        }*/
    }
}
