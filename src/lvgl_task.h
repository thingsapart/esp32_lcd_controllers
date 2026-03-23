#pragma once

/**
 * @file lvgl_task.h
 * @brief Minimal FreeRTOS LVGL render task for esp32_lcd_controllers.
 *
 * lvgl_task_start() spawns a pinned FreeRTOS task that:
 *  1. Calls the optional ui_init_fn callback once (with display + touch).
 *  2. Runs lv_task_handler() in a tight loop, sleeping for however long
 *     LVGL reports until the next timer/animation fires.
 *
 * Compile-time tunables (define via build_flags):
 *
 *   LVGL_TASK_STACK_KB    Stack depth in KiB.
 *                         Default: 10 (S3/C3), 24 (P4).
 *   LVGL_TASK_PRIORITY    FreeRTOS priority.  Default: tskIDLE_PRIORITY+2.
 *   LVGL_TASK_CORE        Core affinity: 0 or 1.  Default: 0.
 *   LVGL_TASK_PSRAM       If defined, allocate the task stack from PSRAM.
 */

#ifdef TFT_WIDTH

#include "lvgl.h"
#include "lcd_controllers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Spawn the LVGL render task.
 *
 * @param ui_init_fn  Called once before the render loop.  May be NULL.
 * @param disp        LVGL display handle from lv_display_create().
 * @param touch       LVGL touch indev handle (may be NULL).
 */
void lvgl_task_start(lcd_ui_init_fn_t ui_init_fn,
                     lv_display_t   *disp,
                     lv_indev_t     *touch);

#ifdef __cplusplus
}
#endif

#endif /* TFT_WIDTH */
