/**
 * @file lcd_controllers.cpp
 * @brief Implementation of the lcd_controllers init/start API.
 *
 * This file wires together:
 *   - Board-specific display_alloc() / display_setup() from the driver layer.
 *   - LVGL initialisation (lv_init, lv_display_create, lv_indev_create).
 *   - The optional encoder indev.
 *   - Starting the LVGL FreeRTOS task.
 *
 * Weak symbols mcu_setup() and mcu_startup() let user code override MCU-level
 * initialization without modifying this library file.
 */

#include "lcd_controllers.h"

#ifdef TFT_WIDTH

#include <Arduino.h>
#include <lvgl.h>

#include "debug.h"
#include "driver/driver_interface.hpp"
#include "lvgl_task.h"

#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)
#include "driver/encoder.hpp"
/* encoder_indev_read is defined as a weak symbol in encoder.cpp.
 * Applications may override it without modifying this file. */
extern "C" void encoder_indev_read(lv_indev_t *indev, lv_indev_data_t *data);
#endif

static const char *TAG = "lcd_ctrl";

static lv_display_t *s_disp         = NULL;
static lv_indev_t   *s_touch_indev  = NULL;

#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)
static lv_indev_t   *s_enc_indev    = NULL;
static lv_group_t   *s_default_group = NULL;
#endif

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

#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)
    s_enc_indev = lv_indev_create();
#endif

    LOGI(TAG, "display_setup");
    display_setup(s_disp, s_touch_indev);

#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)
    /* Wire the encoder indev and attach a default group so that
     * scroll-wheel / value-change events reach focused widgets. */
    lv_indev_set_type(s_enc_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(s_enc_indev, encoder_indev_read);

    s_default_group = lv_group_create();
    lv_indev_set_group(s_enc_indev, s_default_group);
    lv_group_set_editing(s_default_group, true);
    lv_group_set_default(s_default_group);
    LOGI(TAG, "encoder indev registered");
#endif

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

#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)
lv_indev_t   *lcd_controllers_get_encoder_indev(void) { return s_enc_indev; }
#endif

#endif /* TFT_WIDTH */
