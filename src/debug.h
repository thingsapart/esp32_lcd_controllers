#ifndef LCD_CTRL_DEBUG_H
#define LCD_CTRL_DEBUG_H

/**
 * @file debug.h
 * @brief Lightweight, portable logging macros for esp32_lcd_controllers.
 *
 * Works on both ESP32 hardware (Arduino Serial) and POSIX (printf).
 * Log levels can be overridden globally with -D UI_DEBUG_LEVEL=D_VERBOSE
 * or per-file with #define UI_DEBUG_LOCAL_LEVEL before including this header.
 */

/* ── Level constants ─────────────────────────────────────────────────────── */
#define D_NONE    -1
#define D_ERROR    0
#define D_WARN     1
#define D_INFO     2
#define D_DEBUG    3
#define D_VERBOSE  4

/* ── Global default ──────────────────────────────────────────────────────── */
#ifndef UI_DEBUG_LEVEL
#define UI_DEBUG_LEVEL D_INFO
#endif

/* ── Backend ─────────────────────────────────────────────────────────────── */
#if defined(ESP32_HW) && defined(ARDUINO)
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/* Fixed stack-buffer logger.
 * Avoid heap allocation inside tracing because early boot and LVGL task logs
 * are emitted from multiple tasks and malloc+printf was destabilizing startup. */
#define LOG_BACKEND(fmt, ...)                                                  \
    do {                                                                       \
        char __b[256];                                                         \
        int __n = snprintf(__b, sizeof(__b), fmt __VA_OPT__(,) ##__VA_ARGS__);\
        if (__n > 0) {                                                         \
            size_t __len = (size_t)__n;                                        \
            if (__len >= sizeof(__b) - 1) __len = sizeof(__b) - 2;            \
            __b[__len++] = '\n';                                              \
            (void)write(STDOUT_FILENO, __b, __len);                           \
        }                                                                      \
    } while (0)
#elif defined(ESP32_HW)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/* Fixed stack-buffer logger.
 * Avoid heap allocation inside tracing because early boot and LVGL task logs
 * are emitted from multiple tasks and malloc+printf was destabilizing startup. */
#define LOG_BACKEND(fmt, ...)                                                  \
    do {                                                                       \
        char __b[256];                                                         \
        int __n = snprintf(__b, sizeof(__b), fmt __VA_OPT__(,) ##__VA_ARGS__);\
        if (__n > 0) {                                                         \
            size_t __len = (size_t)__n;                                        \
            if (__len >= sizeof(__b) - 1) __len = sizeof(__b) - 2;            \
            __b[__len++] = '\n';                                              \
            (void)write(STDOUT_FILENO, __b, __len);                           \
        }                                                                      \
    } while (0)
#else
#include <stdio.h>
#define LOG_BACKEND(fmt, ...)                                                  \
    do {                                                                       \
        printf(fmt "\n" __VA_OPT__(,) ##__VA_ARGS__);                         \
        fflush(stdout);                                                        \
    } while (0)
#endif

/* ── Filtered macros ─────────────────────────────────────────────────────── */
#if EFFECTIVE_LOG_LEVEL >= D_ERROR
#define LOGE(tag, fmt, ...) LOG_BACKEND("[E][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#define LOGE(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_WARN
#define LOGW(tag, fmt, ...) LOG_BACKEND("[W][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#define LOGW(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_INFO
#define LOGI(tag, fmt, ...) LOG_BACKEND("[I][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#define LOGI(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_DEBUG
#define LOGD(tag, fmt, ...) LOG_BACKEND("[D][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#define LOGD(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_VERBOSE
#define LOGV(tag, fmt, ...) LOG_BACKEND("[V][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#define LOGV(tag, fmt, ...) do {} while (0)
#endif

#endif /* LCD_CTRL_DEBUG_H */

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-file level override — OUTSIDE the include guard so that:
 *   #define UI_DEBUG_LOCAL_LEVEL D_DEBUG
 *   #include "debug.h"
 * re-evaluates and redefines LOGI/LOGE/etc. for the including file.
 * ══════════════════════════════════════════════════════════════════════════ */
#undef EFFECTIVE_LOG_LEVEL
#ifdef UI_DEBUG_LOCAL_LEVEL
#  define EFFECTIVE_LOG_LEVEL UI_DEBUG_LOCAL_LEVEL
#  undef  UI_DEBUG_LOCAL_LEVEL
#else
#  define EFFECTIVE_LOG_LEVEL UI_DEBUG_LEVEL
#endif

#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV

#if EFFECTIVE_LOG_LEVEL >= D_ERROR
#  define LOGE(tag, fmt, ...) LOG_BACKEND("[E][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#  define LOGE(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_WARN
#  define LOGW(tag, fmt, ...) LOG_BACKEND("[W][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#  define LOGW(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_INFO
#  define LOGI(tag, fmt, ...) LOG_BACKEND("[I][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#  define LOGI(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_DEBUG
#  define LOGD(tag, fmt, ...) LOG_BACKEND("[D][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#  define LOGD(tag, fmt, ...) do {} while (0)
#endif

#if EFFECTIVE_LOG_LEVEL >= D_VERBOSE
#  define LOGV(tag, fmt, ...) LOG_BACKEND("[V][%s] " fmt, tag __VA_OPT__(,) ##__VA_ARGS__)
#else
#  define LOGV(tag, fmt, ...) do {} while (0)
#endif
