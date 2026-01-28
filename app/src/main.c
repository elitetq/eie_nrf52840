#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>


#include "BTN.h"
#include "LED.h"

#include <math.h>

#define CMD_SOFTWARE_RESET 0x01
#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_ON 0x29
#define CMD_COLUMN_ADDRESS_SET 0x2A
#define CMD_ROW_ADDRESS_SET 0x2B
#define CMD_MEMORY_WRITE 0x2C

#define CNV_8_TO_6(x) ((uint8_t)x << 2u)

#define SLEEP_MS 100
#define ARDUINO_SPI_NODE DT_NODELABEL(arduino_spi)
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static uint8_t color_data[240 * 3];

static const struct gpio_dt_spec dcx_gpio = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE,dcx_gpios);
static const struct spi_cs_control cs_ctrl = (struct spi_cs_control) {
  .gpio = GPIO_DT_SPEC_GET(ARDUINO_SPI_NODE,cs_gpios),
  .delay = 0u,
};

static const struct device * dev = DEVICE_DT_GET(ARDUINO_SPI_NODE);
static const struct spi_config spi_cfg = {
  .frequency = 6000000,
  .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
  .slave = 0,
  .cs = cs_ctrl
};

static void lcd_cmd(uint8_t cmd, struct spi_buf * data) {
  struct spi_buf cmd_buf[1] = {[0] = {
    .buf=&cmd,
    .len=1}
  };
  struct spi_buf_set cmd_set = {
    .buffers=cmd_buf,
    .count=1,
  };
  
  gpio_pin_set_dt(&dcx_gpio,0); // Set pin to low to let SPI know its a command being sent
  spi_write(dev,&spi_cfg,&cmd_set);

  if(data != NULL) {
    struct spi_buf_set data_set = {data,1};
    gpio_pin_set_dt(&dcx_gpio,1); // Set pin to high to let SPI know its data being sent
    spi_write(dev,&spi_cfg,&data_set);
  }
}

int draw_color_fs(uint8_t* RGB666_COLOR) {
  uint8_t R = sys_rand8_get();
  uint8_t G = sys_rand8_get();
  uint8_t B = sys_rand8_get();
  for(int i = 0; i < sizeof(color_data); i += 3) {
    color_data[i] = B; // Blue
    color_data[i+1] = G; // Green
    color_data[i+2] = R; // Red
  }
  struct spi_buf color_data_buf = {.buf = color_data, .len = sizeof(color_data)};
  struct spi_buf_set color_data_set = {.buffers = &color_data_buf, .count = 1};
  lcd_cmd(CMD_MEMORY_WRITE,NULL);
  
  gpio_pin_set_dt(&dcx_gpio,1);
  for(int i = 0; i < 320; i++) {
    spi_write(dev,&spi_cfg,&color_data_set);
  }
}

int main(void) {
  if(!gpio_is_ready_dt(&dcx_gpio))
    return 0;
  if(gpio_pin_configure_dt(&dcx_gpio,GPIO_OUTPUT_LOW))
    return 0;
  

  if(0 > BTN_init())
    return 0;

  if(0 > LED_init())
    return 0;
  
  lcd_cmd(CMD_SOFTWARE_RESET,NULL);
  k_msleep(120);
  lcd_cmd(CMD_SLEEP_OUT,NULL);
  lcd_cmd(CMD_DISPLAY_ON,NULL);
  
  
  uint16_t height = 239;
  uint16_t length = 319;

  uint8_t column_array[4] = {0x00,0x00,0x00,0xEF};
  uint8_t row_array[4] = {0x00,0x00,0x01,0x3F};

  
  struct spi_buf column_data = {.buf = column_array, .len = 4};
  struct spi_buf row_data = {.buf = row_array, .len = 4};
  
  uint8_t color[3] = {0xFF,0xFF,0x00};
  


  lcd_cmd(CMD_ROW_ADDRESS_SET,&row_data);
  lcd_cmd(CMD_COLUMN_ADDRESS_SET,&column_data);


  while(1) {
    draw_color_fs(color);
    k_msleep(100);
  }
  return 0;
}