#include "lcars_engine.h"
#include "lcars_frame.h"
#include "lcars_font.h"
#include "lcars_animation.h"

// ============================================================
// LcarsEngine Implementation
// ============================================================

LcarsEngine::LcarsEngine() {
    _theme = &LcarsThemes::tng();
}

void LcarsEngine::begin(TFT_eSPI& tft, int16_t width, int16_t height) {
    _tft = &tft;
    _width = width;
    _height = height;

    // Initialize display
    _tft->init();
    _tft->invertDisplay(true);  // ST7789 panels need inversion for correct colors
    _tft->setRotation(3);       // Landscape
    _tft->fillScreen(TFT_BLACK);

    // Create full-screen sprite in PSRAM
    _spr = new TFT_eSprite(_tft);
    _spr->setColorDepth(16);
    _spr->createSprite(_width, _height);
    _spr->fillSprite(TFT_BLACK);

    _lastFrameMs = millis();
    _fpsTimerMs = millis();

    Serial.println("[LCARS] Engine initialized");
    Serial.printf("[LCARS] Display: %dx%d, Sprite: %d bytes\n",
                  _width, _height, _width * _height * 2);
}

void LcarsEngine::setTheme(LcarsThemeId themeId) {
    _theme = &LcarsThemes::get(themeId);
}

void LcarsEngine::setTheme(const LcarsTheme& theme) {
    _theme = &theme;
}

void LcarsEngine::setScreen(LcarsScreen* screen) {
    if (_screen == screen) return;

    if (_transitioning) {
        if (_screen) _screen->onTeardown();
        _screen = nullptr;
        _transitioning = false;
    }

    _nextScreen = screen;
    if (_nextScreen) {
        _nextScreen->_theme = _theme;
    }

    _transitioning = true;
    _transStartMs = millis();
}

void LcarsEngine::update() {
    if (!_spr) return;

    uint32_t now = millis();
    _frameTimeMs = now - _lastFrameMs;
    _lastFrameMs = now;

    // FPS counter
    _fpsCounter++;
    if (now - _fpsTimerMs >= 1000) {
        _fps = _fpsCounter;
        _fpsCounter = 0;
        _fpsTimerMs = now;
    }

    if (_transitioning) {
        _doTransition();
        return;
    }

    if (_screen) {
        uint32_t interval = _screen->refreshIntervalMs();
        if (interval > 0 && (now - _lastRefreshMs) < interval) {
            return;
        }
        _lastRefreshMs = now;

        _screen->onUpdate(_frameTimeMs);
        _renderFrame();
    }
}

void LcarsEngine::_doTransition() {
    uint32_t elapsed = millis() - _transStartMs;

    // Three-phase transition: disassemble → black pause → assemble
    const uint32_t PAUSE_MS = 150;  // Brief dramatic black pause at midpoint
    uint32_t phaseDur = (LCARS_TRANSITION_MS - PAUSE_MS) / 2;
    uint32_t phase2Start = phaseDur;
    uint32_t phase3Start = phaseDur + PAUSE_MS;
    uint32_t totalDur = phase3Start + phaseDur;

    if (elapsed < phase2Start) {
        // Phase 1: old screen's frame disassembles (t goes 1.0 → 0.0)
        float t = (float)elapsed / phaseDur;

        _spr->fillSprite(_theme->background);
        if (_screen) {
            _screen->onDrawTransition(*_spr, 1.0f - t);
        }
        _drawVignette();
        _spr->pushSprite(0, 0);

    } else if (elapsed < phase3Start) {
        // Phase 2: Brief black pause — dramatic "reset" moment
        // Swap screens during this pause
        if (_screen && _nextScreen && _screen != _nextScreen) {
            _screen->onTeardown();
            _screen = _nextScreen;
            _screen->_theme = _theme;
            _screen->onSetup();
            _nextScreen = nullptr;
        }

        _spr->fillSprite(_theme->background);
        _drawVignette();
        _spr->pushSprite(0, 0);

    } else if (elapsed < totalDur) {
        // Phase 3: new screen's frame assembles (t goes 0.0 → 1.0)
        float t = (float)(elapsed - phase3Start) / phaseDur;

        // Ensure screen swap happened (in case phase 2 was skipped)
        if (_nextScreen && _screen != _nextScreen) {
            if (_screen) _screen->onTeardown();
            _screen = _nextScreen;
            _screen->_theme = _theme;
            _screen->onSetup();
            _nextScreen = nullptr;
        }

        _spr->fillSprite(_theme->background);
        if (_screen) {
            _screen->onDrawTransition(*_spr, t);
        }
        _drawVignette();
        _spr->pushSprite(0, 0);

    } else {
        if (_nextScreen) {
            if (_screen) _screen->onTeardown();
            _screen = _nextScreen;
            _screen->_theme = _theme;
            _screen->onSetup();
            _nextScreen = nullptr;
        }
        _transitioning = false;
        _renderFrame();
    }
}

void LcarsEngine::_renderFrame() {
    if (!_screen || !_spr) return;

    if (_screen->wantsFrame()) {
        LcarsFrame::Rect content = LcarsFrame::drawStandardFrame(*_spr, *_theme);

        const char* t = _screen->title();
        if (t && t[0]) {
            // Title sits in the top bar, rendered as dark text on the bar color
            LcarsFont::drawTextUpper(*_spr, t,
                                     LCARS_SIDEBAR_W + LCARS_ELBOW_R + LCARS_BAR_GAP + 4,
                                     LCARS_TOPBAR_H / 2,
                                     LCARS_FONT_SM, _theme->textOnBar,
                                     _theme->barTop, ML_DATUM);
        }

        _screen->onDraw(*_spr, content);
    } else {
        _spr->fillSprite(_theme->background);
        LcarsFrame::Rect full = { 0, 0, _width, _height };
        _screen->onDraw(*_spr, full);
    }

    _drawVignette();
    _spr->pushSprite(0, 0);
}

void LcarsEngine::_drawVignette() {
    if (!_spr) return;

    // Subtle screen-edge darkening — only on top and right edges where
    // no LCARS frame elements live. Left edge has the sidebar, bottom
    // has the bar, so skip those to keep the frame crisp.
    // 4 strips with cubic falloff — very gentle.
    const int DEPTH = 4;

    for (int s = 0; s < DEPTH; s++) {
        float norm = (float)(DEPTH - s) / DEPTH;  // 1.0 at edge → 0.0 inner
        uint8_t a = (uint8_t)(255 - 100 * norm * norm * norm);  // 155→255

        // Top edge (full width)
        for (int16_t x = 0; x < _width; x++) {
            uint16_t px = _spr->readPixel(x, s);
            if (px != 0) _spr->drawPixel(x, s, _spr->alphaBlend(a, px, TFT_BLACK));
        }
        // Bottom edge (full width)
        for (int16_t x = 0; x < _width; x++) {
            int16_t y = _height - 1 - s;
            uint16_t px = _spr->readPixel(x, y);
            if (px != 0) _spr->drawPixel(x, y, _spr->alphaBlend(a, px, TFT_BLACK));
        }
        // Right edge only (left has sidebar — leave it clean)
        int16_t rx = _width - 1 - s;
        for (int16_t y = DEPTH; y < _height - DEPTH; y++) {
            uint16_t px = _spr->readPixel(rx, y);
            if (px != 0) _spr->drawPixel(rx, y, _spr->alphaBlend(a, px, TFT_BLACK));
        }
    }
}

void LcarsEngine::setBacklight(uint8_t brightness) {
    if (_blPin >= 0) {
        analogWrite(_blPin, brightness);
    }
}
