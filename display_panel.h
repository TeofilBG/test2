#ifndef DISPLAY_PANEL_H
#define DISPLAY_PANEL_H

#include <lvgl.h>
#include <ESP_Panel_Library.h>
#include <Arduino.h>

// Display Configuration
extern const uint16_t screenWidth;
extern const uint16_t screenHeight;

// ESP_Panel objects
extern ESP_PanelBacklight *backlight;
extern ESP_PanelLcd *lcd;
extern ESP_PanelTouch *touch;

// LVGL display and input objects
extern lv_disp_draw_buf_t draw_buf;
extern lv_color_t buf1[];
extern lv_color_t buf2[];

// Pin definitions that display module needs
extern const int TFT_BLK;
extern const int TFT_RST;
extern const int TFT_CS;
extern const int TFT_SCK;
extern const int TFT_SDA0;
extern const int TFT_SDA1;
extern const int TFT_SDA2;
extern const int TFT_SDA3;
extern const int TOUCH_PIN_NUM_I2C_SCL;
extern const int TOUCH_PIN_NUM_I2C_SDA;
extern const int TOUCH_PIN_NUM_INT;
extern const int TOUCH_PIN_NUM_RST;

// Function declarations
void initDisplayBuffers();
void esp_panel_init();
void setRotation(uint8_t rot);
void screen_switch(bool on);
void set_brightness(uint8_t bri);

// LVGL callback functions
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data);

// Interrupt callbacks
IRAM_ATTR bool onTouchInterruptCallback(void *user_data);
IRAM_ATTR bool onRefreshFinishCallback(void *user_data);

// Display driver registration
lv_disp_t* registerDisplayDriver();
void registerTouchDriver();

#endif