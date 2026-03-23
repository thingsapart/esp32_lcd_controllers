/**
 * @file driver/esp32_2424s012.cpp
 * @brief LovyanGFX driver for the Waveshare / Sunton ESP32-2424S012C.
 *
 * Hardware:
 *   MCU    : ESP32-C3-MINI-1U (RISC-V single-core, 4 MB flash, no PSRAM)
 *   Display: 1.28" round IPS 240×240 GC9A01 via SPI2
 *   Touch  : CST816S capacitive via I2C0
 *
 * Pin mapping:
 *   GC9A01 SPI  : MOSI=7  SCLK=6  CS=10  DC=2  RST=NC  BL=3
 *   CST816S I2C : SDA=4   SCL=5   RST=1   INT=0
 *
 * Activated by build flag: -D ESP32_2424S012=1
 *
 * ── Migration note ─────────────────────────────────────────────────────────
 * The full implementation lives in cnc_interface/src/driver/esp32_2424s012.cpp.
 * Copy that file here, then replace:
 *   #include "ui/touch_calib/touch_calib.h"
 * with:
 *   #include "touch_calib/touch_calib.h"
 * (The src/ui/touch_calib/touch_calib.h redirect already handles this if
 *  you keep the original include unchanged.)
 * ───────────────────────────────────────────────────────────────────────────
 */

#ifdef ESP32_2424S012

/* ── Disable unused LovyanGFX subsystems ───────────────────────────────── */
#ifndef LGFX_NO_LFS
#define LGFX_NO_LFS
#endif
#ifndef LGFX_NO_SPIFFS
#define LGFX_NO_SPIFFS
#endif
#ifndef LGFX_NO_SD
#define LGFX_NO_SD
#endif
#ifndef LGFX_NO_HTTP
#define LGFX_NO_HTTP
#endif
#ifndef LGFX_NO_PNG
#define LGFX_NO_PNG
#endif
#ifndef LGFX_NO_JPG
#define LGFX_NO_JPG
#endif
#ifndef LGFX_NO_BMP
#define LGFX_NO_BMP
#endif
#ifndef LGFX_NO_QOI
#define LGFX_NO_QOI
#endif

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

#include "debug.h"
#include "touch_calib/touch_calib.h"

static const char *TAG = "ESP32_2424S012";

/* ── LovyanGFX device class ────────────────────────────────────────────── */

class LGFX_ESP32_2424S012 : public lgfx::LGFX_Device {
 public:
  lgfx::Panel_GC9A01  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Touch_CST816S _touch_instance;
  lgfx::Light_PWM     _light_instance;

  LGFX_ESP32_2424S012(void) {
    /* ── SPI bus ─────────────────────────────────────────────────────── */
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host    = SPI2_HOST;
      cfg.spi_mode    = 0;
      cfg.freq_write  = 80000000;
      cfg.freq_read   = 16000000;
      cfg.spi_3wire   = false;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk    = TFT_SCLK;
      cfg.pin_mosi    = TFT_MOSI;
      cfg.pin_miso    = -1;
      cfg.pin_dc      = TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    /* ── Panel ──────────────────────────────────────────────────────── */
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs      = TFT_CS;
      cfg.pin_rst     = TFT_RST;
      cfg.pin_busy    = -1;
      cfg.memory_width  = TFT_WIDTH;
      cfg.memory_height = TFT_HEIGHT;
      cfg.panel_width   = TFT_WIDTH;
      cfg.panel_height  = TFT_HEIGHT;
      cfg.offset_x        = 0;
      cfg.offset_y        = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = false;
      cfg.invert           = true;   /* IPS panel */
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;
      _panel_instance.config(cfg);
    }

    /* ── Backlight ──────────────────────────────────────────────────── */
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = TFT_BCKL;
      cfg.invert      = false;
      cfg.freq        = 44100;
      cfg.pwm_channel = 1;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    /* ── CST816S touch (I2C0) ───────────────────────────────────────── */
    {
      auto cfg = _touch_instance.config();
      cfg.x_min        = 0;
      cfg.x_max        = TFT_WIDTH  - 1;
      cfg.y_min        = 0;
      cfg.y_max        = TFT_HEIGHT - 1;
      cfg.pin_int      = TOUCH_INT;
      cfg.pin_rst      = TOUCH_RST;
      cfg.i2c_port     = I2C_TOUCH_PORT;
      cfg.i2c_addr     = I2C_TOUCH_ADDRESS;
      cfg.pin_sda      = TOUCH_SDA;
      cfg.pin_scl      = TOUCH_SCL;
      cfg.freq         = I2C_TOUCH_FREQUENCY;
      cfg.offset_rotation = 0;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

static LGFX_ESP32_2424S012 tft;

/* ── LVGL callback: flush ────────────────────────────────────────────────── */

static void display_flush_cb(lv_display_t *disp, const lv_area_t *area,
                              uint8_t *px_map) {
  if (tft.getStartCount() == 0) tft.startWrite();
  tft.pushImageDMA(area->x1, area->y1,
                   area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                   (lgfx::rgb565_t *)px_map);
  lv_disp_flush_ready(disp);
}

/* ── LVGL callback: touch read ───────────────────────────────────────────── */

static void touch_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    float fx = (float)touchX;
    float fy = (float)touchY;
    touch_calib_apply_inplace(&fx, &fy);
    data->point.x = (lv_coord_t)fx;
    data->point.y = (lv_coord_t)fy;
  }
}

/* ── LVGL draw buffers ───────────────────────────────────────────────────── */

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
#define BUF_LINES       20
#define DRAW_BUF_SIZE   (TFT_WIDTH * BUF_LINES * BYTES_PER_PIXEL)

static uint8_t draw_buf1[DRAW_BUF_SIZE];
static uint8_t draw_buf2[DRAW_BUF_SIZE];

/* ── display_alloc / display_setup ──────────────────────────────────────── */

void display_alloc() {
  LOGI(TAG, "display_alloc: %u + %u bytes (internal SRAM, C3 has no PSRAM)",
       (unsigned)sizeof(draw_buf1), (unsigned)sizeof(draw_buf2));
}

void display_setup(lv_display_t *disp, lv_indev_t *indev) {
  LOGI(TAG, "display_setup: GC9A01 240×240 on ESP32-C3");

  tft.init();
  tft.setRotation(0);
  tft.setBrightness(128);
  tft.fillScreen(TFT_BLACK);

  lv_display_set_flush_cb(disp, display_flush_cb);
  lv_display_set_buffers(disp, draw_buf1, draw_buf2, sizeof(draw_buf1),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_indev_read_cb);
  lv_indev_set_display(indev, disp);

  LOGI(TAG, "display_setup done.");
}

#endif /* ESP32_2424S012 */
