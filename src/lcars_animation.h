#pragma once

#include <TFT_eSPI.h>
#include "lcars_screen.h"
#include "lcars_theme.h"

// ============================================================
// LCARS Animation System
// Built-in screens for boot sequences and visual effects
// ============================================================

// ── Boot Sequence Screen ────────────────────────────────────
// Animated startup sequence:
//   1. Black → diagnostic text scrolls up
//   2. Frame assembles: elbows appear, bars extend
//   3. Transitions to the first user screen
//
class LcarsBootScreen : public LcarsScreen {
public:
    // Set the screen to transition to after boot completes
    void setNextScreen(LcarsScreen* next) { _nextScreen = next; }

    // LcarsScreen overrides
    void onSetup() override;
    void onUpdate(uint32_t deltaMs) override;
    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& content) override;
    const char* title() const override { return ""; }
    bool wantsFrame() const override { return false; }  // Custom full-screen rendering

    bool isComplete() const { return _complete; }
    LcarsScreen* nextScreen() const { return _nextScreen; }

private:
    enum BootPhase {
        BOOT_DIAG_TEXT,     // Scrolling diagnostic text
        BOOT_FRAME_BUILD,   // Frame elements assemble
        BOOT_DONE
    };

    BootPhase    _phase = BOOT_DIAG_TEXT;
    uint32_t     _phaseStartMs = 0;
    uint32_t     _elapsed = 0;
    int16_t      _diagLine = 0;
    bool         _complete = false;
    LcarsScreen* _nextScreen = nullptr;

    static const char* _diagMessages[];
    static const int   _diagCount;

    void _drawDiagPhase(TFT_eSprite& spr);
    void _drawFrameBuildPhase(TFT_eSprite& spr);
};

// ── Utility: Easing Functions ───────────────────────────────

namespace LcarsEasing {
    float linear(float t);
    float easeInQuad(float t);
    float easeOutQuad(float t);
    float easeInOutQuad(float t);
    float easeOutCubic(float t);
    float easeOutBack(float t);    // Overshoot then settle — "lock into place"
}
