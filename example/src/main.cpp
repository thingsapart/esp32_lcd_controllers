/**
 * @file main.cpp  —  esp32_lcd_controllers Hello World example
 *
 * Builds for all supported boards.  The board-specific behaviour is selected
 * entirely through build_flags defined in platformio.ini — no #if blocks in
 * this file are needed.
 *
 * Step-by-step:
 *  1. lcd_controllers_init_and_start() runs MCU setup, allocates draw buffers,
 *     initialises LVGL, wires the board-specific display + touch driver, and
 *     starts the LVGL FreeRTOS task.
 *  2. my_ui() is called once from within the LVGL task, before the render
 *     loop starts.  Create your screens here.
 *  3. Arduino loop() is left empty; all UI work happens in the LVGL task.
 */

#include "lcd_controllers.h"
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* ── Optional: override the weak mcu_setup / mcu_startup hooks ────────────
 *
 * lcd_controllers.cpp provides default weak implementations that:
 *   mcu_setup()   → Serial.begin(115200) + log PSRAM size
 *   mcu_startup() → no-op
 *
 * Uncomment to customise (e.g. mount LittleFS, connect WiFi, read NVS).
 */
/*
extern "C" void mcu_setup() {
    Serial.begin(115200);
    // nvs_flash_init();
    // LittleFS.begin();
}

extern "C" void mcu_startup() {
    // wifi_manager_connect("SSID", "password", 10000);
}
*/

/* ── Optional: override the weak per-frame hook ───────────────────────────
 *
 * lvgl_task.cpp calls lvgl_loop_hook() once per render iteration.
 * Use it for data-binding updates, sensor reads, or other lightweight work.
 */
/*
extern "C" void lvgl_loop_hook() {
    // my_data_binding_tick();
}
*/

/* ── UI initialisation callback ───────────────────────────────────────────
 *
 * Called once from the LVGL task before the first render.
 * @p disp   Active LVGL display (TFT_WIDTH × TFT_HEIGHT).
 * @p touch  Touch indev, or NULL if the board has no touch.
 */
static void my_ui(lv_display_t *disp, lv_indev_t *touch) {
    (void)touch;  /* not used in this simple example */

    /* ── Background ─────────────────────────────────────────────────── */
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_hex(0x1a1a2e), LV_PART_MAIN);

    /* ── Centred "Hello, LVGL!" label ───────────────────────────────── */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xe94560), LV_PART_MAIN);
    lv_obj_center(label);

    /* ── Board info label (bottom of screen) ────────────────────────── */
    lv_obj_t *info = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(info, "%d x %d",
                          lv_display_get_horizontal_resolution(disp),
                          lv_display_get_vertical_resolution(disp));
    lv_obj_set_style_text_color(info, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -10);

    /* ── A pulsing circle animation ──────────────────────────────────── */
    lv_obj_t *circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle, 60, 60);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0x0f3460), LV_PART_MAIN);
    lv_obj_set_style_border_width(circle, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(circle, lv_color_hex(0xe94560), LV_PART_MAIN);
    lv_obj_align(circle, LV_ALIGN_TOP_MID, 0, 40);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, circle);
    lv_anim_set_values(&a, 40, 80);
    lv_anim_set_duration(&a, 800);
    lv_anim_set_playback_duration(&a, 800);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
        lv_obj_set_size((lv_obj_t *)obj, v, v);
        lv_obj_align((lv_obj_t *)obj, LV_ALIGN_TOP_MID, 0, 40);
    });
    lv_anim_start(&a);
}

/* ── Arduino entry points ─────────────────────────────────────────────────── */

void setup() {
    /*
     * One call to rule them all:
     *   1. mcu_setup() / mcu_startup()
     *   2. display_alloc() + lv_init() + display_setup()
     *   3. Spawn the LVGL task (calls my_ui before entering the render loop)
     */
    lcd_controllers_init_and_start(my_ui);
}

void loop() {
    /* All rendering happens in the LVGL FreeRTOS task.
     * Keep loop() free for your own background work, or just idle. */
    vTaskDelay(portMAX_DELAY);
}
