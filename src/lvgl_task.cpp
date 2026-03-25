/**
 * @file lvgl_task.cpp
 * @brief Minimal FreeRTOS LVGL render task.
 *
 * Intentionally lightweight – it does NOT include machine-interface ticks,
 * jog-encoder processing, or deferred loaders.  Those belong in application
 * code.  Call lcd_controllers_register_loop_hook() to inject per-iteration
 * work into the render loop without modifying this file.
 *
 * Supported tunable build flags:
 *   LVGL_TASK_STACK_KB     task stack in KiB   (default: 10 / 24 for P4)
 *   LVGL_TASK_PRIORITY     FreeRTOS priority   (default: tskIDLE_PRIORITY+2)
 *   LVGL_TASK_CORE         core affinity 0|1   (default: 0)
 *   LVGL_TASK_PSRAM        allocate stack from PSRAM instead of SRAM
 */

#include "lvgl_task.h"

#ifdef TFT_WIDTH

#include <Arduino.h>
#include <lvgl.h>
#include "debug.h"

#ifdef ESP32_HW
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#ifdef LVGL_TASK_PSRAM
#include "esp_heap_caps.h"
#endif
#endif

static const char *TAG = "lvgl_task";

/* ── Tunables ────────────────────────────────────────────────────────────── */

#ifndef LVGL_TASK_PRIORITY
#define LVGL_TASK_PRIORITY  (tskIDLE_PRIORITY + 2)
#endif

#ifndef LVGL_TASK_CORE
#define LVGL_TASK_CORE  0
#endif

#ifndef LVGL_TASK_STACK_KB
#  ifdef ESP32P4_HW
#    define LVGL_TASK_STACK_KB  24
#  elif __riscv
#    define LVGL_TASK_STACK_KB  6
#  else
#    define LVGL_TASK_STACK_KB  10
#  endif
#endif

/* ── Optional user loop hook ─────────────────────────────────────────────── */

/**
 * Weak per-frame hook.  Override in your project to run work every LVGL
 * iteration (e.g. data-binding updates, input polling).
 * Keep it fast: > ~10 ms will cause perceived UI jank.
 */
__attribute__((weak)) void lvgl_loop_hook(void) { /* default: no-op */ }

/* ── Task context ────────────────────────────────────────────────────────── */

typedef struct {
    lcd_ui_init_fn_t ui_init_fn;
    lv_display_t    *disp;
    lv_indev_t      *touch;
} lvgl_task_params_t;

static TaskHandle_t s_lvgl_task_handle = NULL;

/* ── LVGL tick source ────────────────────────────────────────────────────── */

#if defined(configUSE_TICK_HOOK) || defined(CONFIG_USE_TICK_HOOK)
#ifdef ESP32_HW
#include "esp_freertos_hooks.h"
/* Installed as a per-CPU FreeRTOS tick hook (1 kHz). */
static void lv_tick_cb(void) {
    lv_tick_inc(1000 / CONFIG_FREERTOS_HZ);
}
#endif
#else
/* Fallback: dedicated 1-ms tick task (wastes a little SRAM for its stack). */
static void lv_tick_task(void *pv) {
    for (;;) {
        lv_tick_inc(1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
#endif

/* ── Main render task ────────────────────────────────────────────────────── */

static void lvgl_task_fn(void *pv) {
    lvgl_task_params_t *p = (lvgl_task_params_t *)pv;

    LOGI(TAG, "LVGL task started (stack %d KiB, core %d)",
         LVGL_TASK_STACK_KB, LVGL_TASK_CORE);

    /* Subscribe to the task watchdog so we detect real hangs even when
     * software-render workers starve IDLE0 during heavy render passes. */
#ifdef ESP32_HW
    LOGI(TAG, "registering task watchdog");
    esp_task_wdt_add(NULL);
    LOGI(TAG, "task watchdog registered");
#endif

    /* Call user UI initialisation before first render.
     * Reset the watchdog first — create_ui() + I2C init can take several
     * seconds on PSRAM-heavy builds and would otherwise trip the WDT. */
    if (p->ui_init_fn) {
        LOGI(TAG, "calling ui_init_fn");
#ifdef ESP32_HW
        LOGI(TAG, "resetting task watchdog before ui_init_fn");
        esp_task_wdt_reset(); /* keep WDT happy during slow UI init */
#endif
        p->ui_init_fn(p->disp, p->touch);
        LOGI(TAG, "ui_init_fn completed");
    }

    vTaskDelay(1);

    while (true) {
        static uint32_t loop_count = 0;
        if (++loop_count % 100 == 0) {
            LOGI(TAG, "LVGL task loop #%u", loop_count);
        }
#ifdef ESP32_HW
        esp_task_wdt_reset();
#endif
        /* ── [B] LVGL render + flush ─────────────────────────────────────── */
        uint32_t time_start  = (uint32_t)millis();
        uint32_t sleep_ms    = lv_task_handler();

        /* ── [C] Sleep until next LVGL event / timer ─────────────────────── */
        TickType_t delay_ticks = pdMS_TO_TICKS(sleep_ms ? sleep_ms : 1);
        if (delay_ticks == 0) delay_ticks = 1;
        vTaskDelay(delay_ticks);

        /* ── [D] User-supplied per-frame work (data-binding, etc.) ───────── */
        lvgl_loop_hook();

        /* ── [E] lv_tick_inc fallback (when no tick hook is available) ───── */
#if !defined(configUSE_TICK_HOOK) && !defined(CONFIG_USE_TICK_HOOK)
        lv_tick_inc((uint32_t)millis() - time_start);
#endif
    }
}

/* ── Public entry point ──────────────────────────────────────────────────── */

void lvgl_task_start(lcd_ui_init_fn_t ui_init_fn,
                     lv_display_t   *disp,
                     lv_indev_t     *touch) {
    /* Install tick source. */
#if defined(configUSE_TICK_HOOK) || defined(CONFIG_USE_TICK_HOOK)
#ifdef ESP32_HW
    esp_register_freertos_tick_hook_for_cpu(&lv_tick_cb, 0);
    LOGI(TAG, "tick hook installed on CPU0");
#endif
#else
    xTaskCreatePinnedToCore(lv_tick_task, "lv_tick", 512, NULL,
                            tskIDLE_PRIORITY + 3, NULL, LVGL_TASK_CORE);
    LOGI(TAG, "lv_tick_task started");
#endif

    /* Heap-allocate the param block so it survives until the task reads it. */
    static lvgl_task_params_t s_params;   /* single task: static is fine */
    s_params.ui_init_fn = ui_init_fn;
    s_params.disp       = disp;
    s_params.touch      = touch;

    const uint32_t stack_bytes = LVGL_TASK_STACK_KB * 1024;
    BaseType_t     result      = pdFAIL;

#ifdef LVGL_TASK_PSRAM
    /* Allocate task stack from PSRAM to preserve scarce internal SRAM. */
    result = xTaskCreateWithCaps(
        lvgl_task_fn, "lvgl_task", stack_bytes, &s_params,
        LVGL_TASK_PRIORITY, &s_lvgl_task_handle,
        MALLOC_CAP_SPIRAM);
    LOGI(TAG, "lvgl_task stack in PSRAM");
#else
    result = xTaskCreatePinnedToCore(
        lvgl_task_fn, "lvgl_task", stack_bytes, &s_params,
        LVGL_TASK_PRIORITY, &s_lvgl_task_handle,
        LVGL_TASK_CORE);
    LOGI(TAG, "lvgl_task pinned to core %d", LVGL_TASK_CORE);
#endif

    if (result == pdPASS) {
        LOGI(TAG, "lvgl_task created (%u bytes stack)", stack_bytes);
    } else {
        LOGE(TAG, "lvgl_task CREATE FAILED (result=%d)  "
             "– out of stack memory?  Try -D LVGL_TASK_PSRAM or "
             "reduce LVGL_TASK_STACK_KB", (int)result);
        /* Abort early with a visible message rather than a silent hang. */
        while (true) { vTaskDelay(portMAX_DELAY); }
    }
}

#endif /* TFT_WIDTH */
