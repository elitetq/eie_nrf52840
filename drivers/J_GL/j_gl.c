#include "J_GL.h"
static struct J_CONTAINER J_CONTAINER_t = {0};
static uint8_t color_data[LCD_MAX_LENGTH*3];

void J_init(const struct device* dev_spi, const struct device* dev_i2c, const struct spi_config* spi_cfg, const struct gpio_dt_spec* dcx_gpio, uint16_t* bounds) {
  J_CONTAINER_t.dev_spi = dev_spi;
  J_CONTAINER_t.dev_i2c = dev_i2c;
  J_CONTAINER_t.spi_cfg = spi_cfg;
  J_CONTAINER_t.dcx_gpio = dcx_gpio;
  J_CONTAINER_t.bounds = bounds;
}

uint16_t cnv_float(float num) {
  if(num > 65535.0)
    return 0xFFFF;

  if(num < 0)
    return (uint16_t)(-1*num);

  return (uint16_t)num;
}

uint32_t get_pos() {
    uint16_t x_pos, y_pos;
    uint8_t x_pos_l, y_pos_l, x_pos_h, y_pos_h;
    touch_control_cmd_rsp(P1_XL,&x_pos_l);
    touch_control_cmd_rsp(P1_YL,&y_pos_l);
    touch_control_cmd_rsp(P1_XH,&x_pos_h);
    touch_control_cmd_rsp(P1_YH,&y_pos_h);
    y_pos = (uint16_t)(((x_pos_h & TOUCH_POS_MSB_MASK) << 8) + x_pos_l);
    x_pos = LCD_MAX_LENGTH - (uint16_t)(((y_pos_h & TOUCH_POS_MSB_MASK) << 8) + y_pos_l);
    return ((uint32_t)(x_pos) << 16) + (uint32_t)(y_pos);
}

int draw_color_fs(uint8_t* RGB666_COLOR) {

  for(int i = 0; i < LCD_MAX_LENGTH*3; i += 3) {
    color_data[i] = RGB666_COLOR[2]; // Blue
    color_data[i+1] = RGB666_COLOR[1]; // Green
    color_data[i+2] = RGB666_COLOR[0]; // Red
  }
  struct spi_buf color_data_buf = {.buf = color_data, .len = LCD_MAX_LENGTH*3};
  struct spi_buf_set color_data_set = {.buffers = &color_data_buf, .count = 1};
  lcd_cmd(CMD_MEMORY_WRITE,NULL);
  
  gpio_pin_set_dt(J_CONTAINER_t.dcx_gpio,1);
  for(int i = 0; i < 320; i++) {
    spi_write(J_CONTAINER_t.dev_spi,J_CONTAINER_t.spi_cfg,&color_data_set);
  }
  return 0;
}

void set_bounds(uint16_t* user_list) {
  for(int i = 0; i < 4; i++) {
    J_CONTAINER_t.bounds[i] = user_list[i];
  }
  struct spi_buf column_data = {
    .buf = (uint8_t[]){(J_CONTAINER_t.bounds[0]>>8),J_CONTAINER_t.bounds[0],(J_CONTAINER_t.bounds[1]>>8),J_CONTAINER_t.bounds[1]},
    .len = 4
  };
  struct spi_buf row_data = {
    .buf = (uint8_t[]){(J_CONTAINER_t.bounds[2]>>8),J_CONTAINER_t.bounds[2],(J_CONTAINER_t.bounds[3]>>8),J_CONTAINER_t.bounds[3]},
    .len = 4
  };
  lcd_cmd(CMD_ROW_ADDRESS_SET,&row_data);
  lcd_cmd(CMD_COLUMN_ADDRESS_SET,&column_data);
}

void touch_control_cmd_rsp(uint8_t cmd, uint8_t* rsp) {
  struct i2c_msg cmd_rsp_msg[2] = {
    {&cmd, 1, I2C_MSG_WRITE},
    {rsp, 1, I2C_MSG_RESTART | I2C_MSG_READ | I2C_MSG_STOP}
  };
  i2c_transfer(J_CONTAINER_t.dev_i2c,cmd_rsp_msg,2,TD_ADDR);
}

void lcd_cmd(uint8_t cmd, struct spi_buf * data) {
  struct spi_buf cmd_buf[1] = {[0] = {
    .buf=&cmd,
    .len=1}
  };
  struct spi_buf_set cmd_set = {
    .buffers=cmd_buf,
    .count=1,
  };
  
  gpio_pin_set_dt(J_CONTAINER_t.dcx_gpio,0); // Set pin to low to let SPI know its a command being sent
  spi_write(J_CONTAINER_t.dev_spi,J_CONTAINER_t.spi_cfg,&cmd_set);

  if(data != NULL) {
    struct spi_buf_set data_set = {data,1};
    gpio_pin_set_dt(J_CONTAINER_t.dcx_gpio,1); // Set pin to high to let SPI know its data being sent
    spi_write(J_CONTAINER_t.dev_spi,J_CONTAINER_t.spi_cfg,&data_set);
  }
}

void draw_square(uint16_t x, uint16_t y, uint16_t size) {
  set_bounds((uint16_t[]){y-size/2,y+size/2,x-size/2,x+size/2});
  draw_color_fs((uint8_t[]){0xFF,0x00,0x00});
}

void draw_circle(uint16_t x, uint16_t y, uint16_t radius) {
  uint16_t diameter = radius*2 + 1;
  uint8_t local_color_data[diameter * 3];
  for(int i = 0; i < sizeof(local_color_data); i += 3) {
    local_color_data[i] = 0x00;
    local_color_data[i + 1] = 0x00;
    local_color_data[i + 2] = 0x00;
  }
  set_bounds((uint16_t[]){y-radius,y+radius,x-radius,x+radius});
  struct spi_buf color_data_buf = {.buf = local_color_data, .len = sizeof(local_color_data)};
  struct spi_buf_set color_data_set = {.buffers = &color_data_buf, .count = 1};
  lcd_cmd(CMD_MEMORY_WRITE,NULL);
  
  gpio_pin_set_dt(J_CONTAINER_t.dcx_gpio,1);
  uint16_t mid = (diameter - 1) / 2 + 1;
  for(int i = 0; i < diameter; i++) {
    uint16_t local_y = abs(mid - i);
    uint16_t local_x = radius + sqrt(radius*radius - local_y*local_y);
    printf("local x and y: %d %d\n", local_y, local_x);
    k_msleep(1);
    for(int j = 0; j < sizeof(local_color_data); j+= 3) {
      if(((j/3) > local_x) || ((j/3) < (diameter - local_x))) {
        local_color_data[j] = 0x00;
        local_color_data[j + 1] = 0x00;
        local_color_data[j + 2] = 0x00;
      } else {
        local_color_data[j] = 0x00;
        local_color_data[j + 1] = 0x00;
        local_color_data[j + 2] = 0xFF;
      }
    }
    spi_write(J_CONTAINER_t.dev_spi,J_CONTAINER_t.spi_cfg,&color_data_set);
  }
  
}