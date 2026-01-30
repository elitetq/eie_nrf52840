#include <errno.h>
/**
 * @file main.c
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>





int main(void) {
  if (0 > BTN_init())
    return 0;
  if (0 > LED_init())
    return 0;

  while (1) 
    k_msleep(SLEEP_MS);
  return 0;
}
