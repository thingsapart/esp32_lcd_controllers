#ifndef LCD_CTRL_TOUCH_CALIB_H
#define LCD_CTRL_TOUCH_CALIB_H

/**
 * @file touch_calib.h
 * @brief Touch-calibration API for esp32_lcd_controllers.
 *
 * This stub implementation performs NO calibration — raw touch coordinates
 * are passed through unchanged.  It satisfies all driver-layer calls to
 * touch_calib_apply_inplace() without pulling in NVS or a calibration wizard.
 *
 * To enable real calibration, replace src/touch_calib/touch_calib.cpp with
 * your own implementation (e.g., copy from cnc_interface/src/ui/touch_calib/).
 * The header contract must remain the same.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Returns true if a calibration matrix is currently loaded.
 * Stub always returns false.
 */
bool touch_calib_has(void);

/**
 * @brief Load a previously saved calibration matrix from persistent storage.
 * Stub always returns false (no calibration stored).
 */
bool touch_calib_load(void);

/**
 * @brief Apply a 3×3 homography matrix as the active calibration.
 * Stub accepts the call and discards the matrix.
 * @param H Row-major 3×3 homography matrix (length 9).
 */
bool touch_calib_apply(const float H[9]);

/**
 * @brief Save a 3×3 homography matrix to persistent storage.
 * Stub always returns false (not implemented).
 */
bool touch_calib_save(const float H[9]);

/**
 * @brief Clear any stored calibration.
 * Stub is a no-op.
 */
void touch_calib_clear(void);

/**
 * @brief Apply the stored homography to a touch point (in-place).
 *
 * Called by every driver's touch read callback.  The stub leaves
 * *x and *y unchanged (identity transform).
 *
 * @param x  Pointer to the raw touch X coordinate.
 * @param y  Pointer to the raw touch Y coordinate.
 */
void touch_calib_apply_inplace(float *x, float *y);

/**
 * @brief Compute a homography from 4 source (raw) to 4 destination points.
 * Stub always returns false.
 */
bool touch_calib_compute_homography(const float src[4][2],
                                    const float dst[4][2],
                                    float       H_out[9]);

#ifdef __cplusplus
}
#endif

#endif /* LCD_CTRL_TOUCH_CALIB_H */
