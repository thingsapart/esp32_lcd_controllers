/**
 * @file encoder.hpp
 * @brief Rotary encoder driver using ESP32 PCNT hardware peripheral.
 *
 * Exposes a global `encoder` object (only when ENCODER_PIN_X and ENCODER_PIN_Y
 * are defined) and a weak `encoder_indev_read()` LVGL callback.
 *
 * The weak callback can be overridden in the application to apply custom
 * scaling, acceleration, or button logic.
 */
#ifndef __MACHINE_ENC_HPP__
#define __MACHINE_ENC_HPP__

#include "Arduino.h"
#include "contrib/rotary_encoder.hpp"

class Encoder {
 public:
  Encoder(uint8_t pin_x, uint8_t pin_y, int divisor = 1);

  /** Read accumulated delta since last call, reset counter. */
  int readAndReset();

  /** Current absolute position (only valid in encoder mode). */
  int position();

  /** Switch to UI mode: delta is passed to LVGL, absolute position unused. */
  void setUiMode();

  /** Switch to encoder mode: delta contributes to absolute position. */
  void setEncoderMode();

  bool isUiMode() { return uiMode; }

 private:
  bool uiMode;
  int uiModeCount;
  int encModeCount;
  int encModePosition;
  int divisor;
#ifdef ESP32_HW
  RotaryEncoderPCNT enc;
#else
  /* Stub for non-ESP32 targets (e.g. POSIX simulator). */
  class Enc {
   public:
    Enc(uint8_t /*pin_x*/, uint8_t /*pin_y*/) {}
    int position() { return 0; }
    void zero() {}
  } enc;
#endif
};

/** Global encoder instance.  Defined in encoder.cpp; present only when
 *  ENCODER_PIN_X and ENCODER_PIN_Y are both defined. */
extern Encoder encoder;

#endif  // __MACHINE_ENC_HPP__
