#pragma once

#include <stdint.h>

// ============================================================
// LCARS Audio System
// Optional buzzer/speaker chirps for UI feedback.
// Based on Star Trek TNG interface sounds.
//
// Usage:
//   LcarsAudio::begin(BUZZER_PIN);
//   LcarsAudio::beepHigh();      // Button press
//   LcarsAudio::beepLow();       // Screen transition
//   LcarsAudio::beepConfirm();   // Confirmation / success
//   LcarsAudio::beepAlert();     // Alert / error
//
// All beeps are non-blocking — they use tone() which runs
// asynchronously on ESP32. Call update() in loop() to
// auto-stop tones after their duration.
// ============================================================

namespace LcarsAudio {

    // Initialize the audio system with a buzzer/speaker pin
    // Pass -1 to disable audio (default)
    void begin(int8_t pin = -1);

    // Enable/disable audio at runtime
    void setEnabled(bool enabled);
    bool isEnabled();

    // Update — call in loop() to handle tone-off timing
    void update();

    // ── Beep Variants ──────────────────────────────────────
    // All match the TNG LCARS sound aesthetic:
    //   - Short, clean tones (50-150ms)
    //   - Frequencies in the 800-2400Hz range
    //   - No complex waveforms (simple square wave from tone())

    void beepHigh();       // Quick high chirp  — button press / tap
    void beepLow();        // Lower tone        — screen transition / navigation
    void beepConfirm();    // Two-tone rising   — confirmation / success
    void beepAlert();      // Three rapid beeps — alert / warning
    void beepBoot();       // Ascending sweep   — boot sequence complete

    // Custom tone (frequency in Hz, duration in ms)
    void playTone(uint16_t freqHz, uint16_t durationMs);
}
