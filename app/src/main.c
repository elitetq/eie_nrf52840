
/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
/**
 * #include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
 */

#include "BTN.h"
#include "LED.h"

//Increment an array of size 4 representing bits
int increment(int *arr) {
    int i = 0;
    int j = 0;
    while(4 != i) {
        if(0 == arr[i]) {
            arr[i] = 1;
            for(j = 0; i > j; j++) {
                arr[j] = 0;
            }
            return 0;
        }
        i++;
    }
    for(i = 0; 4 > i; i++) { // Case where all are 1's
        arr[i] = 0;
    }
}

int main(void) {
    int counter[] = {0,0,0,0};
    if(0 > BTN_init()) {
        return 0;
    }
    if(0 > LED_init()) {
        return 0;
    }

    LED_set(LED0,LED_OFF);
    LED_set(LED1,LED_OFF);
    LED_set(LED2,LED_OFF);
    LED_set(LED3,LED_OFF);

    while(1) {
        if(BTN_check_clear_pressed(BTN0)) {
            increment(counter);
            for(int i = 0; NUM_LEDS > i; i++) {
                if(counter[i] == 1) {
                    LED_set(i,LED_ON);
                } else {
                    LED_set(i,LED_OFF);
                }
            }
            printk("Pressed!");
        }
        k_msleep(500);
    }

    return 0;
}
