#include <errno.h>
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

static struct bt_conn* my_connection;

static struct bt_uuid_128 BLE_CUSTOM_SERVICE_UUID = 
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111,0x2222,0x3333,0x4444,0x0000'0000'0001));
  
static struct bt_uuid_128 BLE_CUSTOM_CHARACTER_UUID =
  BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x11111111,0x2222,0x3333,0x4444,0x0000'0000'0002));

static void ble_on_device_connected(struct bt_conn* conn, uint8_t err) {
  if(err != 0) {
    bt_conn_unref(my_connection);
    my_connection = NULL;
  }

  char addr[BT_ADDR_LE_STR_LEN];
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  printk("Connected: %s\n", addr);
}

static void ble_on_device_disconnected(struct bt_conn* conn, uint8_t reason) {
  char addr_my_con[BT_ADDR_LE_STR_LEN];
  char addr_this_con[BT_ADDR_LE_STR_LEN];
  char reason_msg = bt_hci_err_to_str(reason);

  bt_addr_le_to_str(bt_conn_get_dst(my_connection), addr_my_con, sizeof(addr_my_con));
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr_this_con, sizeof(addr_this_con));
  if(!strcmp(addr_my_con,addr_this_con)) {
    bt_conn_unref(conn);
    conn = NULL;
    printk("Disconnected: %s\n",reason_msg);
  }

}

static void ble_on_advertisement_received(const bt_addr_le_t* addr, int8_t rssi, uint8_t type, struct net_buf_simple* ad) {
  if(my_connection != NULL || (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND)) {
    return;
  }
  char name[32] = {0};
  bt_data_parse(ad,ble_get_adv_device_name_cb,name);
  printk("Device Name: %s\nDevice MAC Address: %s",name,bt_addr_le_to_str(addr));

  rssi < -50 ? return : bt_le_scan_stop();
  int ret = bt_conn_le_create(addr,BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT,&my_connection);
  if(ret != 0) {
    bt_conn_unref(my_connection);
    my_connection = NULL;
    printk("ERROR: Advertisement found, but connection unsuccessful.\n");
  }
  
}

static bool ble_get_adv_device_name_cb(struct bt_data* data, void* user_data) {
  char* name = user_data;

  if(data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {
    memcpy(name,data->data,data->data_len);
    name[data_len] = 0;
    return false;
  }
  
  return true;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
  .connected = ble_on_device_connected,
  .disconnected = ble_on_device_disconnected,
};




int main(void) {
  bt_le_scan_start(BT_LE_SCAN_PASSIVE,ble_on_advertisement_received);
  
}