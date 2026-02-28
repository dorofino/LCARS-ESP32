#pragma once

#include <TFT_eSPI.h>
#include "lcars_theme.h"
#include "lcars_screen.h"
#include "lcars_config.h"

// ============================================================
// LCARS Engine
// Core singleton that manages display, sprite buffer,
// screen lifecycle, theme, and frame rendering.
// ============================================================

class LcarsEngine {
public:
    LcarsEngine();

    // Initialize the engine with a TFT display instance
    // Creates a full-screen sprite in PSRAM
    void begin(TFT_eSPI& tft, int16_t width = SCR_WIDTH, int16_t height = SCR_HEIGHT);

    // Set the active color theme
    void setTheme(LcarsThemeId themeId);
    void setTheme(const LcarsTheme& theme);
    const LcarsTheme& theme() const { return *_theme; }

    // Set the active screen (triggers teardown of old, setup of new)
    void setScreen(LcarsScreen* screen);
    LcarsScreen* currentScreen() const { return _screen; }

    // Call in loop() — handles update, draw, push to display
    void update();

    // Direct access to sprite for advanced use
    TFT_eSprite& sprite() { return *_spr; }
    TFT_eSPI& tft() { return *_tft; }

    // Display dimensions
    int16_t width() const { return _width; }
    int16_t height() const { return _height; }

    // Frame timing
    uint32_t fps() const { return _fps; }
    uint32_t frameTime() const { return _frameTimeMs; }

    // Backlight control
    void setBacklight(uint8_t brightness);
    void setBLPin(int8_t pin) { _blPin = pin; }

    // Transition state
    bool isTransitioning() const { return _transitioning; }

private:
    TFT_eSPI*    _tft = nullptr;
    TFT_eSprite* _spr = nullptr;
    LcarsScreen* _screen = nullptr;
    LcarsScreen* _nextScreen = nullptr;
    const LcarsTheme* _theme = nullptr;

    int16_t _width = 320;
    int16_t _height = 170;

    // Frame timing
    uint32_t _lastFrameMs = 0;
    uint32_t _lastRefreshMs = 0;
    uint32_t _frameTimeMs = 0;
    uint32_t _fps = 0;
    uint32_t _fpsCounter = 0;
    uint32_t _fpsTimerMs = 0;

    // Transition animation
    bool     _transitioning = false;
    uint32_t _transStartMs = 0;

    // Backlight
    int8_t   _blPin = -1;

    void _doTransition();
    void _drawTransitionFrame(float t, LcarsScreen* screen);
    void _renderFrame();
};
