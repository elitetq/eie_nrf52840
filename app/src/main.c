
/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>

#include "BTN.h"
#include "LED.h"

#include "my_state_machine.h"

#define SLEEP_TIME_MS 1

int main(void) {
    if(0 > LED_init()) {
        return 0;
    }
    if(0 > BTN_init()) {
        return 0;
    }

    uint8_t current_duty_cycle = 0;

    LED_pwm(LED0,current_duty_cycle);

    while(1) {
        if(BTN_check_clear_pressed(BTN0)) {
            current_duty_cycle = current_duty_cycle > 100 ? 0 : (current_duty_cycle + 10);
            printk("Set LED to %d%% brightness",current_duty_cycle);
            LED_pwm(LED0,current_duty_cycle);
        }
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
