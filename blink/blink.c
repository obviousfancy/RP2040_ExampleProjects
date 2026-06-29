#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25


int main(){

    gpio_init(LED_PIN); // Initialize GPIO pin 25
    gpio_set_dir(LED_PIN, GPIO_OUT); // Set GPIO pin 25 as output
    
    while(1){
        gpio_put(LED_PIN, 1); // Turn on LED
        sleep_ms(1000); // Delay for 1 second
        gpio_put(LED_PIN, 0); // Turn off LED
        sleep_ms(1000); // Delay for 1 second
    }
    return 0;
}