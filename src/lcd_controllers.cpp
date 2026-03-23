/**
 * @file lcd_controllers.cpp
 * @brief Implementation of the lcd_controllers init/start API.
 *
 * This file wires together:
 *   - Board-specific display_alloc() / display_setup() from the driver layer.
 *   - LVGL initialisation (lv_init, lv_display_create, lv_indev_create).
 *   - Starting the LVGL FreeRTOS task.
 *
 * Weak symbols mcu_setup() and mcu_startup() let user code override MCU-level
 * initialization without modifying this library file.
 */

#include "lcd_controllers.h"

#ifdef TFT_WIDTH

#include <Arduino.h>
#include <lvgl.h>

#ifdef CONFIG_ESP32
#include <esp_heap_caps.h>
#endif

#include "debug.h"
#include "driver/driver_interface.hpp"
#include "lvgl_task.h"

static const char *TAG = "lcd_ctrl";

static lv_display_t *s_disp         = NULL;
static lv_indev_t   *s_touch_indev  = NULL;

/* ── Overridable hooks ───────────────────────────────────────────────────── */

/**
 * Weak default MCU setup.  Override in your project to add custom
 * peripheral init (NVS, WiFi credentials, custom serial baud, etc.).
 */
__attribute__((weak)) void mcu_setup(void) {
    Serial.begin(115200);
    LOGI(TAG, "mcu_setup (default)");

#ifdef CONFIG_SPIRAM
    size_t psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    if (psram > 0) {
        LOGI(TAG, "PSRAM: %u bytes", psram);
    } else {
        LOGW(TAG, "PSRAM: not detected or not enabled");
    }
#endif
}

/**
 * Weak post-MCU hook.  Called after mcu_setup() but before LVGL init.
 * Override to start WiFi, load NVS settings, etc.
 */
__attribute__((weak)) void mcu_startup(void) {
    /* default: nothing */
}

/**
 * Weak default RAM usage debug helper.
 *
 * User project can override this function to provide custom logging.
 */
__attribute__((weak)) void ram_usage(void) {
#ifdef ARDUINO
    (void)0; // keep compiler happy when no print support available
#endif
#ifdef CONFIG_SPIRAM
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t total_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    LOGI("ram_usage", "SRAM free: %u/%u, PSRAM free: %u/%u", (unsigned)free_sram, (unsigned)total_sram, (unsigned)free_psram, (unsigned)total_psram);
#else
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    LOGI("ram_usage", "SRAM free: %u", (unsigned)free_sram);
#endif
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void lcd_controllers_init(void) {
    mcu_setup();
    mcu_startup();

    LOGI(TAG, "display_alloc");
    display_alloc();

    LOGI(TAG, "lv_init");
    lv_init();

    s_disp        = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
    s_touch_indev = lv_indev_create();

    LOGI(TAG, "display_setup");
    display_setup(s_disp, s_touch_indev);

    LOGI(TAG, "lcd_controllers_init done  (display %d x %d)",
         TFT_WIDTH, TFT_HEIGHT);
}

void lcd_controllers_start(lcd_ui_init_fn_t ui_init_fn) {
    lvgl_task_start(ui_init_fn, s_disp, s_touch_indev);
}

void lcd_controllers_init_and_start(lcd_ui_init_fn_t ui_init_fn) {
    lcd_controllers_init();
    lcd_controllers_start(ui_init_fn);
}

lv_display_t *lcd_controllers_get_display(void)      { return s_disp; }
lv_indev_t   *lcd_controllers_get_touch_indev(void)  { return s_touch_indev; }

#endif /* TFT_WIDTH */
