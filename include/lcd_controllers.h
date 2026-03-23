#pragma once

/**
 * @file lcd_controllers.h
 * @brief Single public header for the esp32_lcd_controllers library.
 *
 * Typical Arduino setup():
 * @code
 *   #include "lcd_controllers.h"
 *
 *   static void my_ui(lv_display_t *disp, lv_indev_t *touch) {
 *       lv_obj_t *label = lv_label_create(lv_screen_active());
 *       lv_label_set_text(label, "Hello, LVGL!");
 *       lv_obj_center(label);
 *   }
 *
 *   void setup() {
 *       lcd_controllers_init_and_start(my_ui);
 *   }
 *   void loop() { /* idle - LVGL runs in its own task * / }
 * @endcode
 *
 * For finer control, use the two-step form:
 * @code
 *   void setup() {
 *       lcd_controllers_init();    // MCU init, buffer alloc, lv_init, display setup
 *       // ... optional: create objects here, before starting the task ...
 *       lcd_controllers_start(my_ui);  // spawn LVGL FreeRTOS task
 *   }
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#ifdef TFT_WIDTH

#include "lvgl.h"

/**
 * @brief User UI-initialisation callback.
 *
 * Called ONCE from within the LVGL task, before the main render loop starts.
 * This is the right place to create screens, widgets, and start animations.
 *
 * @param disp   The active LVGL display (TFT_WIDTH × TFT_HEIGHT pixels).
 * @param touch  Active LVGL pointer indev, or NULL if no touch panel present.
 */
typedef void (*lcd_ui_init_fn_t)(lv_display_t *disp, lv_indev_t *touch);

/* ── Two-step API ──────────────────────────────────────────────────────────── */

/**
 * @brief Initialise the MCU, LVGL, and the board display + touch drivers.
 *
 * Must be called once from Arduino @p setup() BEFORE starting any LVGL tasks.
 *
 * Sequence performed:
 *  1. @c mcu_setup()      – Serial, PSRAM health, watchdog (overridable weak fn).
 *  2. @c mcu_startup()    – Post-MCU hook (overridable weak fn, default no-op).
 *  3. @c display_alloc()  – Allocates LVGL draw buffers early to avoid
 *                           heap fragmentation.
 *  4. @c lv_init()
 *  5. @c lv_display_create(TFT_WIDTH, TFT_HEIGHT)
 *  6. @c lv_indev_create()
 *  7. @c display_setup()  – Board-specific flush/touch callbacks.
 */
void lcd_controllers_init(void);

/**
 * @brief Start the LVGL FreeRTOS render task.
 *
 * Must be called after @c lcd_controllers_init().  The task will:
 *  1. Call @p ui_init_fn once (if non-NULL) with the active display and indev.
 *  2. Enter the @c lv_task_handler() loop, sleeping between renders.
 *
 * Task tunables (override via build flags):
 *  - @c LVGL_TASK_STACK_KB   – Stack in KiB (default: 10 for S3/C3, 24 for P4)
 *  - @c LVGL_TASK_PRIORITY   – FreeRTOS priority (default: tskIDLE_PRIORITY+2)
 *  - @c LVGL_TASK_CORE       – Core affinity: 0 (default; use -1 for unpinned)
 *  - @c LVGL_TASK_PSRAM      – Allocate stack from PSRAM instead of SRAM
 *
 * @param ui_init_fn  UI creation callback.  Pass NULL to start the loop
 *                    without any UI-init (useful when you create widgets
 *                    before calling lcd_controllers_start()).
 */
void lcd_controllers_start(lcd_ui_init_fn_t ui_init_fn);

/* ── One-call convenience ──────────────────────────────────────────────────── */

/**
 * @brief Combines lcd_controllers_init() + lcd_controllers_start().
 *
 * The simplest way to bring up a board with LVGL running:
 * @code
 *   lcd_controllers_init_and_start(my_ui_init_fn);
 * @endcode
 */
void lcd_controllers_init_and_start(lcd_ui_init_fn_t ui_init_fn);

/* ── Accessors (valid after lcd_controllers_init()) ──────────────────────── */

/** @brief Returns the LVGL display handle created during init. */
lv_display_t *lcd_controllers_get_display(void);

/** @brief Returns the LVGL touch indev handle (may be NULL if no touch). */
lv_indev_t   *lcd_controllers_get_touch_indev(void);

#endif /* TFT_WIDTH */

#ifdef __cplusplus
}
#endif
