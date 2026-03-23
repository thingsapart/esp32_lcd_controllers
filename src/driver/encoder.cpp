/**
 * @file encoder.cpp
 * @brief Rotary encoder driver implementation + default LVGL indev callback.
 *
 * Compiled only when ENCODER_PIN_X and ENCODER_PIN_Y are defined in the
 * project's build_flags (e.g. -D ENCODER_PIN_X=5 -D ENCODER_PIN_Y=6).
 */
#if defined(ENCODER_PIN_X) && defined(ENCODER_PIN_Y)

#include "encoder.hpp"
#include "debug.h"

#include <assert.h>
#include <lvgl.h>

static const char *TAG = "encoder";

/* Global encoder instance (divisor=4 for quadrature step-per-detent). */
Encoder encoder(ENCODER_PIN_X, ENCODER_PIN_Y, 4);

Encoder::Encoder(uint8_t pin_x, uint8_t pin_y, int divisor)
    : enc(pin_x, pin_y),
      uiMode(false),
      uiModeCount(0),
      encModeCount(0),
      divisor(divisor),
      encModePosition(0) {}

int Encoder::readAndReset() {
    int val = enc.position() + (uiMode ? uiModeCount : encModeCount);
    int rem = val % divisor;
    int res = val / divisor;
    if (rem < 0) { res -= 1; rem += divisor; }
    if (rem > 0) { res += 1; rem -= divisor; }
    enc.zero();
    if (uiMode) {
        if (res != 0)
            LOGI(TAG, "ENC delta %d (raw %d)", res, val);
        uiModeCount = rem;
    } else {
        encModePosition += res;
        encModeCount = rem;
    }
    return res;
}

int Encoder::position() {
    assert(!uiMode);
    return encModePosition;
}

void Encoder::setUiMode() {
    if (!uiMode) {
        encModeCount += enc.position();
        enc.zero();
        uiMode = true;
    }
}

void Encoder::setEncoderMode() {
    if (uiMode) {
        uiModeCount += enc.position();
        enc.zero();
        uiMode = false;
    }
}

/* ── C linkage helpers called from LVGL task / app code ─────────────────── */
extern "C" {
void encoder_set_ui_mode()      { encoder.setUiMode(); }
void encoder_set_encoder_mode() { encoder.setEncoderMode(); }
}

/* ── Default LVGL indev read callback ───────────────────────────────────── *
 *
 * This weak implementation passes the raw encoder delta directly to LVGL.
 * Override in your application to add acceleration, button press detection,
 * or any other custom logic.
 *
 * Example override in main.cpp:
 *
 *   void encoder_indev_read(lv_indev_t *indev, lv_indev_data_t *data) {
 *       data->enc_diff = encoder.readAndReset();
 *       data->state    = (digitalRead(ENCODER_BTN_PIN) == LOW)
 *                        ? LV_INDEV_STATE_PRESSED
 *                        : LV_INDEV_STATE_RELEASED;
 *   }
 */
__attribute__((weak))
void encoder_indev_read(lv_indev_t * /*indev*/, lv_indev_data_t *data) {
    data->enc_diff = encoder.readAndReset();
    data->state    = LV_INDEV_STATE_RELEASED;
}

#endif /* ENCODER_PIN_X && ENCODER_PIN_Y */
