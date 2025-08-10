#include "display_panel.h"

// Display Configuration - Match SquareLine Studio export settings
const uint16_t screenWidth = 360;
const uint16_t screenHeight = 360;

// Pin definitions
const int TFT_BLK = 15;
const int TFT_RST = 47;
const int TFT_CS = 10;
const int TFT_SCK = 9;
const int TFT_SDA0 = 11;
const int TFT_SDA1 = 12;
const int TFT_SDA2 = 13;
const int TFT_SDA3 = 14;
const int TOUCH_PIN_NUM_I2C_SCL = 8;
const int TOUCH_PIN_NUM_I2C_SDA = 7;
const int TOUCH_PIN_NUM_INT = 41;
const int TOUCH_PIN_NUM_RST = 40;

// ESP_Panel objects
ESP_PanelBacklight *backlight = NULL;
ESP_PanelLcd *lcd = NULL;
ESP_PanelTouch *touch = NULL;

// LVGL display and input objects
lv_disp_draw_buf_t draw_buf;
// Use double buffering to eliminate artifacts
lv_color_t buf1[screenWidth * screenHeight / 10];
lv_color_t buf2[screenWidth * screenHeight / 10];

#define TFT_SPI_FREQ_HZ (50 * 1000 * 1000)

// Initialize display buffers
void initDisplayBuffers() {
    // Clear both display buffers to prevent artifacts
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    
    // Initialize display buffer with double buffering to eliminate artifacts
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 10);
}

/* Display flushing - Keep original color handling but fix artifacts with full buffer */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    ESP_PanelLcd *lcd = (ESP_PanelLcd *)disp->user_data;
    
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    uint32_t pixel_count = w * h;
    
    // Create a temporary buffer to avoid modifying the original data
    static uint16_t temp_buffer[360 * 36]; // Buffer for largest possible area
    uint16_t *pixel_data = (uint16_t *)color_p;
    uint16_t *temp_data = temp_buffer;
    
    // Copy and convert colors in the temporary buffer
    for (uint32_t i = 0; i < pixel_count; i++) {
        uint16_t pixel = pixel_data[i];
        temp_data[i] = (pixel >> 8) | (pixel << 8);  // Swap high and low bytes
    }
    
    // Use ESP_Panel LCD drawing function
    if (lcd != NULL) {
        lcd->drawBitmap(area->x1, area->y1, w, h, (const uint8_t *)temp_data);
    }
    
    lv_disp_flush_ready(disp);
}

IRAM_ATTR bool onRefreshFinishCallback(void *user_data)
{
  lv_disp_drv_t *drv = (lv_disp_drv_t *)user_data;
  lv_disp_flush_ready(drv);
  return false;
}

/*Read the touchpad - ESP_Panel implementation to replace TFT_eSPI touch*/
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    ESP_PanelTouch *tp = (ESP_PanelTouch *)indev_driver->user_data;
    ESP_PanelTouchPoint point;

    int read_touch_result = tp->readPoints(&point, 1);
    if (read_touch_result > 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;
        
        /*Set the coordinates*/
        data->point.x = point.x;
        data->point.y = point.y;

        Serial.print("Touch x: ");
        Serial.print(point.x);
        Serial.print(", y: ");
        Serial.println(point.y);
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

#if TOUCH_PIN_NUM_INT >= 0
IRAM_ATTR bool onTouchInterruptCallback(void *user_data)
{
  return false;
}
#endif

// Screen rotation function
void setRotation(uint8_t rot)
{
  if (rot > 3)
    return;
  if (lcd == NULL || touch == NULL)
    return;

  switch (rot)
  {
  case 1: // 顺时针90度
    lcd->swapXY(true);
    lcd->mirrorX(true);
    lcd->mirrorY(false);
    touch->swapXY(true);
    touch->mirrorX(true);
    touch->mirrorY(false);
    break;
  case 2:
    lcd->swapXY(false);
    lcd->mirrorX(true);
    lcd->mirrorY(true);
    touch->swapXY(false);
    touch->mirrorX(true);
    touch->mirrorY(true);
    break;
  case 3:
    lcd->swapXY(true);
    lcd->mirrorX(false);
    lcd->mirrorY(true);
    touch->swapXY(true);
    touch->mirrorX(false);
    touch->mirrorY(true);
    break;
  default:
    lcd->swapXY(false);
    lcd->mirrorX(false);
    lcd->mirrorY(false);
    touch->swapXY(false);
    touch->mirrorX(false);
    touch->mirrorY(false);
    break;
  }
}

// Screen control functions
void screen_switch(bool on)
{
  if (NULL == backlight)
    return;
  if (on)
    backlight->on();
  else
    backlight->off();
}

void set_brightness(uint8_t bri)
{
  if (NULL == backlight)
    return;
  backlight->setBrightness(bri);
}

// Initialize ESP_Panel components (replaces TFT_eSPI initialization)
void esp_panel_init()
{
  // Initialize LEDC for backlight
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_13_BIT,
      .timer_num = LEDC_TIMER_0,
      .freq_hz = 5000,
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledc_channel_config_t ledc_channel = {
      .gpio_num = (TFT_BLK),
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0};

  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  // Initialize backlight
  backlight = new ESP_PanelBacklight(ledc_timer, ledc_channel);
  backlight->begin();
  backlight->off();

  // Initialize touch
  ESP_PanelBus_I2C *touch_bus = new ESP_PanelBus_I2C(TOUCH_PIN_NUM_I2C_SCL, TOUCH_PIN_NUM_I2C_SDA, ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG());
  touch_bus->configI2cFreqHz(400000);
  touch_bus->begin();

  touch = new ESP_PanelTouch_CST816S(touch_bus, screenWidth, screenHeight, TOUCH_PIN_NUM_RST, TOUCH_PIN_NUM_INT);
  touch->init();
  touch->begin();

#if TOUCH_PIN_NUM_INT >= 0
  touch->attachInterruptCallback(onTouchInterruptCallback, NULL);
#endif

  // Initialize LCD
  ESP_PanelBus_QSPI *panel_bus = new ESP_PanelBus_QSPI(TFT_CS, TFT_SCK, TFT_SDA0, TFT_SDA1, TFT_SDA2, TFT_SDA3);
  panel_bus->configQspiFreqHz(TFT_SPI_FREQ_HZ);
  panel_bus->begin();

  lcd = new ESP_PanelLcd_ST77916(panel_bus, 16, TFT_RST);
  
  // Configure color format before initialization
  esp_lcd_panel_io_color_trans_done_cb_t color_trans_done = NULL;
  
  lcd->init();
  lcd->reset();
  lcd->begin();

  // Keep your original color inversion setting that was working
  lcd->invertColor(true);   // Keep the original setting that worked for you
  // setRotation(0);  // Set screen orientation if needed
  lcd->displayOn();

  static uint16_t black_frame[screenWidth * screenHeight];
  memset(black_frame, 0, sizeof(black_frame));
  lcd->drawBitmap(0, 0, screenWidth, screenHeight, (uint8_t *)black_frame);

  screen_switch(true);
  backlight->setBrightness(100); // Set brightness
}

// Register display driver with LVGL
lv_disp_t* registerDisplayDriver() {
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;  // ESP_Panel flush function with artifact fix
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = (void *)lcd;   // Pass LCD instance to flush function
    
    // Force full refresh to eliminate partial update artifacts
    disp_drv.full_refresh = 1;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    // Attach refresh callback for non-RGB displays
    if (lcd->getBus()->getType() != ESP_PANEL_BUS_TYPE_RGB)
    {
        lcd->attachRefreshFinishCallback(onRefreshFinishCallback, (void *)disp->driver);
    }
    
    return disp;
}

// Register touch input driver with LVGL
void registerTouchDriver() {
    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;  // ESP_Panel touch function
    indev_drv.user_data = (void *)touch;   // Pass touch instance to read function
    lv_indev_drv_register(&indev_drv);
}