#include "lcars_audio.h"
#include <Arduino.h>

// ============================================================
// LCARS Audio Implementation
// ============================================================

static int8_t  _pin = -1;
static bool    _enabled = false;
static uint32_t _toneEndMs = 0;

// Multi-tone sequence state
static const uint16_t MAX_SEQ = 4;
static uint16_t _seqFreq[MAX_SEQ];
static uint16_t _seqDur[MAX_SEQ];
static uint8_t  _seqLen = 0;
static uint8_t  _seqIdx = 0;
static uint32_t _seqNextMs = 0;

void LcarsAudio::begin(int8_t pin) {
    _pin = pin;
    _enabled = (pin >= 0);
    if (_pin >= 0) {
        pinMode(_pin, OUTPUT);
        noTone(_pin);
    }
}

void LcarsAudio::setEnabled(bool enabled) {
    _enabled = enabled && (_pin >= 0);
    if (!_enabled && _pin >= 0) {
        noTone(_pin);
        _seqLen = 0;
    }
}

bool LcarsAudio::isEnabled() {
    return _enabled;
}

void LcarsAudio::update() {
    if (!_enabled || _pin < 0) return;

    uint32_t now = millis();

    // Handle tone-off
    if (_toneEndMs > 0 && now >= _toneEndMs) {
        noTone(_pin);
        _toneEndMs = 0;

        // Advance multi-tone sequence
        if (_seqIdx < _seqLen) {
            _seqNextMs = now + 30;  // 30ms gap between tones
        }
    }

    // Play next tone in sequence
    if (_seqNextMs > 0 && now >= _seqNextMs && _seqIdx < _seqLen) {
        _seqNextMs = 0;
        tone(_pin, _seqFreq[_seqIdx], _seqDur[_seqIdx]);
        _toneEndMs = now + _seqDur[_seqIdx];
        _seqIdx++;
    }
}

void LcarsAudio::playTone(uint16_t freqHz, uint16_t durationMs) {
    if (!_enabled || _pin < 0) return;
    _seqLen = 0;  // Cancel any sequence
    tone(_pin, freqHz, durationMs);
    _toneEndMs = millis() + durationMs;
}

static void _startSequence(const uint16_t* freqs, const uint16_t* durs, uint8_t count) {
    if (!_enabled || _pin < 0 || count == 0) return;

    // Play first tone immediately
    tone(_pin, freqs[0], durs[0]);
    _toneEndMs = millis() + durs[0];

    // Queue remaining tones
    _seqLen = (count > MAX_SEQ) ? MAX_SEQ : count;
    for (uint8_t i = 1; i < _seqLen; i++) {
        _seqFreq[i] = freqs[i];
        _seqDur[i] = durs[i];
    }
    _seqIdx = 1;
    _seqNextMs = 0;
}

// ── Beep Variants ──────────────────────────────────────────

void LcarsAudio::beepHigh() {
    // Quick high chirp — like TNG button press
    playTone(1760, 60);
}

void LcarsAudio::beepLow() {
    // Lower tone — navigation / screen change
    playTone(880, 80);
}

void LcarsAudio::beepConfirm() {
    // Two-tone rising — confirmation
    static const uint16_t freqs[] = { 1200, 1800 };
    static const uint16_t durs[]  = { 60, 80 };
    _startSequence(freqs, durs, 2);
}

void LcarsAudio::beepAlert() {
    // Three rapid beeps — alert
    static const uint16_t freqs[] = { 1400, 1400, 1400 };
    static const uint16_t durs[]  = { 50, 50, 50 };
    _startSequence(freqs, durs, 3);
}

void LcarsAudio::beepBoot() {
    // Ascending sweep — boot complete
    static const uint16_t freqs[] = { 880, 1320, 1760, 2200 };
    static const uint16_t durs[]  = { 60, 60, 60, 100 };
    _startSequence(freqs, durs, 4);
}
