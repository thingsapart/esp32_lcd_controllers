/**
 * @file touch_calib.cpp
 * @brief No-op touch calibration stub.
 *
 * All functions are identity stubs — no calibration is stored or applied.
 * Replace this file with a real implementation to enable touch calibration.
 */

#include "touch_calib.h"

bool touch_calib_has(void)                              { return false; }
bool touch_calib_load(void)                             { return false; }
bool touch_calib_apply(const float H[9])                { (void)H; return false; }
bool touch_calib_save(const float H[9])                 { (void)H; return false; }
void touch_calib_clear(void)                            { }

/* Identity transform — raw coordinates pass through unchanged. */
void touch_calib_apply_inplace(float *x, float *y)     { (void)x; (void)y; }

bool touch_calib_compute_homography(const float src[4][2],
                                    const float dst[4][2],
                                    float       H_out[9]) {
    (void)src; (void)dst; (void)H_out;
    return false;
}
