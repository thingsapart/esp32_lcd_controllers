#ifndef LCD_CTRL_DRIVER_INTERFACE_HPP
#define LCD_CTRL_DRIVER_INTERFACE_HPP

/**
 * @file driver/driver_interface.hpp
 * @brief Board-driver interface contract.
 *
 * Every board-specific driver file (e.g. guition_4848s040.cpp) must implement
 * these three functions.  The application layer calls them in sequence:
 *
 *   display_alloc ()              -- early PSRAM buffer reservation
 *   lv_init ()                   -- (handled by lcd_controllers.cpp)
 *   lv_display_create / indev    -- (handled by lcd_controllers.cpp)
 *   display_setup (disp, indev)  -- wires the flush + touch callbacks
 *
 * Only the driver matching the selected board (#define via -D flag) will
 * provide real implementations; all others are compiled empty due to their
 * top-level #ifdef guards.
 */

#ifdef TFT_WIDTH

#include "lvgl.h"

/**
 * @brief Allocate LVGL draw buffers.
 *
 * Called BEFORE lv_init() so that large PSRAM buffers are reserved before
 * the heap becomes fragmented.  Each driver decides buffer size and location
 * (internal SRAM vs PSRAM) based on the target board's constraints.
 *
 * Drivers that use the esp_lvgl_port adapter (MIPI-DSI boards) may implement
 * this as a no-op if the adapter handles its own buffer allocation.
 */
void display_alloc();

/**
 * @brief Configure the LVGL display and input device for the target board.
 *
 * Called after lv_display_create() and lv_indev_create().  The driver must:
 *   – Initialise the physical display hardware (panel init, backlight on).
 *   – Register an lv_display flush callback that writes pixel data.
 *   – Register an lv_indev read callback that reports touch coordinates.
 *
 * @param disp   LVGL display handle (already created by the caller).
 * @param indev  LVGL input-device handle (already created by the caller).
 */
void display_setup(lv_display_t *disp, lv_indev_t *indev);

#endif /* TFT_WIDTH */

#endif /* LCD_CTRL_DRIVER_INTERFACE_HPP */
