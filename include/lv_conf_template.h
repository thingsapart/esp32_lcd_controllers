/**
 * @file lv_conf_template.h
 * @brief Starter LVGL v9 configuration for esp32_lcd_controllers projects.
 *
 * USAGE:
 *   1. Copy this file to your project root and rename it  lv_conf.h
 *   2. Your platformio.ini already sets  -D LV_CONF_INCLUDE_SIMPLE  and  -I .
 *      so LVGL picks up  lv_conf.h  from the project root automatically.
 *   3. Adjust the sections marked  ← CONFIGURE  for your application.
 */

/* clang-format off */
#if 1  /* Set this to "1" to enable the content */

#ifndef LV_CONF_H
#define LV_CONF_H

/* ══════════════════════════════════════════════════════════════════════════
   COLOR
   ══════════════════════════════════════════════════════════════════════════ */

/** Color depth: 16 = RGB565 (default for all supported boards). */
#define LV_COLOR_DEPTH 16

/* ══════════════════════════════════════════════════════════════════════════
   STDLIB / MEMORY
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN

#ifdef ESP32P4_HW
    #define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB
#else
    #define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB
#endif

#define LV_STDINT_INCLUDE   <stdint.h>
#define LV_STDDEF_INCLUDE   <stddef.h>
#define LV_STDBOOL_INCLUDE  <stdbool.h>
#define LV_INTTYPES_INCLUDE <inttypes.h>
#define LV_LIMITS_INCLUDE   <limits.h>
#define LV_STDARG_INCLUDE   <stdarg.h>

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /** ← CONFIGURE: LVGL internal heap size.
     *  For PSRAM boards enlarge freely; for no-PSRAM (C3) keep it small. */
    #if defined(__riscv) && !defined(CONFIG_SPIRAM)
        #define LV_MEM_SIZE (48 * 1024U)    /* ESP32-C3 no-PSRAM */
    #elif defined(ESP32P4_HW)
        #define LV_MEM_SIZE (164 * 1024U)
    #else
        #define LV_MEM_SIZE (164 * 1024U)   /* S3 with PSRAM */
    #endif

    #define LV_MEM_POOL_EXPAND_SIZE (64 * 1024U)
    #define LV_MEM_ADR 0

    /* Route LVGL allocations to PSRAM on boards that have it. */
    #if LV_MEM_ADR == 0
        #if defined(ESP32_HW) && defined(BOARD_HAS_PSRAM)
            #define LV_MEM_POOL_INCLUDE <esp32-hal-psram.h>
            #define LV_MEM_POOL_ALLOC   ps_malloc
        #endif
    #endif
#endif

/* ══════════════════════════════════════════════════════════════════════════
   HAL / REFRESH RATE
   ══════════════════════════════════════════════════════════════════════════ */

/** ← CONFIGURE: display refresh period in ms.
 *  50 ms = 20 FPS (safe default).  33 ms = 30 FPS (good for S3/P4). */
#ifdef ESP32P4_HW
    #define LV_DEF_REFR_PERIOD  33
#else
    #define LV_DEF_REFR_PERIOD  33
#endif

/** Default DPI — used to scale dp units to pixels.
 *  ← CONFIGURE: set to your panel's actual DPI for correct sizing. */
#define LV_DPI_DEF  130

/* ══════════════════════════════════════════════════════════════════════════
   DRAW ENGINE
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_DRAW_SW  1

#ifdef ESP32S3_HW
    /** Enable Xtensa S3 SIMD blending (requires driver/esp_lvgl/simd/*.S). */
    #define LV_USE_DRAW_SW_ASM   LV_DRAW_SW_ASM_CUSTOM
    #define LV_DRAW_SW_ASM_CUSTOM_INCLUDE "lv_blend_esp32s3.h"
#elif defined(ESP32P4_HW)
    /** Enable P4 PPA hardware blending (requires driver/esp_lvgl/ppa/). */
    #define LV_USE_GPU_ESP32_P4_PPA  1
#endif

/** Number of software draw units per display flush.
 *  For dual-core S3/P4 set to 2–4 to enable parallel rendering. */
#ifdef ESP32P4_HW
    #define LV_DRAW_SW_DRAW_UNIT_CNT  2
#elif defined(ESP32S3_HW)
    #define LV_DRAW_SW_DRAW_UNIT_CNT  2
#else
    #define LV_DRAW_SW_DRAW_UNIT_CNT  1
#endif

/* ══════════════════════════════════════════════════════════════════════════
   DISPLAY DRIVER COMPATIBILITY
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_DISPLAY   1
#define LV_USE_INDEV     1
#define LV_USE_SYSMON    0

/* ══════════════════════════════════════════════════════════════════════════
   FONTS  ← CONFIGURE: enable only the fonts you actually use to save flash.
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_FONT_MONTSERRAT_12   1
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_MONTSERRAT_16   1
#define LV_FONT_MONTSERRAT_24   1
#define LV_FONT_MONTSERRAT_32   0
#define LV_FONT_MONTSERRAT_48   0

#define LV_FONT_DEFAULT  &lv_font_montserrat_14

/* ══════════════════════════════════════════════════════════════════════════
   WIDGETS  ← CONFIGURE: disable widgets you don't need to save flash.
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_ARC          1
#define LV_USE_BAR          1
#define LV_USE_BUTTON       1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CANVAS       1
#define LV_USE_CHART        1
#define LV_USE_CHECKBOX     1
#define LV_USE_DROPDOWN     1
#define LV_USE_IMAGE        1
#define LV_USE_IMAGEBUTTON  1
#define LV_USE_KEYBOARD     0
#define LV_USE_LABEL        1
#define LV_USE_LED          1
#define LV_USE_LINE         1
#define LV_USE_LIST         1
#define LV_USE_MENU         0
#define LV_USE_MSGBOX       0
#define LV_USE_ROLLER       1
#define LV_USE_SCALE        1
#define LV_USE_SLIDER       1
#define LV_USE_SPAN         0
#define LV_USE_SPINBOX      1
#define LV_USE_SPINNER      1
#define LV_USE_SWITCH       1
#define LV_USE_TABLE        1
#define LV_USE_TABVIEW      1
#define LV_USE_TEXTAREA     1
#define LV_USE_TILEVIEW     0
#define LV_USE_WIN          0

/* ══════════════════════════════════════════════════════════════════════════
   LOGGING  ← CONFIGURE: enable for debugging, disable for release.
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_LOG           0

/* ══════════════════════════════════════════════════════════════════════════
   OS / THREADING
   ══════════════════════════════════════════════════════════════════════════ */

#ifdef ESP32P4_HW
    /** P4 with FreeRTOS parallel render workers needs LV_OS_FREERTOS. */
    #define LV_USE_OS   LV_OS_FREERTOS
#else
    #define LV_USE_OS   LV_OS_NONE
#endif

/* ══════════════════════════════════════════════════════════════════════════
   FILESYSTEM  ← CONFIGURE: enable if you need lv_fs_ API.
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_FS_ARDUINO_ESP_LITTLEFS  0
#define LV_USE_FS_POSIX                 0

/* ══════════════════════════════════════════════════════════════════════════
   IMAGE DECODERS
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_PNG   0
#define LV_USE_BMP   0
#define LV_USE_TJPGD 0
#define LV_USE_GIF   0

/* ══════════════════════════════════════════════════════════════════════════
   MISC
   ══════════════════════════════════════════════════════════════════════════ */

#define LV_USE_OBSERVER     1
#define LV_USE_XML          0
#define LV_USE_PROFILER     0

#endif /* LV_CONF_H */
#endif /* Guard */
