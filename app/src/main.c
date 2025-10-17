
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


int main(void) {
    if(0 > BTN_init()) {
        return 0;
    }
    if(0 > LED_init()) {
        return 0;
    }

    while(1) {
        if(BTN_check_clear_pressed(BTN0)) {
            LED_toggle(LED0);
            printk("Pressed!");
        }
        k_msleep(500);
    }

    return 0;
}
