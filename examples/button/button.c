#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25 
#define BUTTON_PIN 24

int main(){

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    gpio_pull_up(BUTTON_PIN); // Pull up the button to avoid floating state the Chinese Board doesn't have pull up resistor in the circuit

    while(1){
        if(gpio_get(BUTTON_PIN)){
            gpio_put(LED_PIN, 0);
        }else{
            gpio_put(LED_PIN, 1);
        }
    }
    
    return 0;
}