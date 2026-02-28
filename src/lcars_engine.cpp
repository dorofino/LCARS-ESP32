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
    uint32_t halfDuration = LCARS_TRANSITION_MS / 2;

    if (elapsed < halfDuration) {
        float t = (float)elapsed / halfDuration;

        _spr->fillSprite(_theme->background);

        if (_screen && _screen->wantsFrame()) {
            _drawTransitionFrame(1.0f - t, _screen);
        }
        _spr->pushSprite(0, 0);

    } else if (elapsed < LCARS_TRANSITION_MS) {
        float t = (float)(elapsed - halfDuration) / halfDuration;

        if (_screen && _nextScreen && _screen != _nextScreen) {
            _screen->onTeardown();
            _screen = _nextScreen;
            _screen->_theme = _theme;
            _screen->onSetup();
            _nextScreen = nullptr;
        }

        _spr->fillSprite(_theme->background);

        if (_screen && _screen->wantsFrame()) {
            _drawTransitionFrame(t, _screen);
        }
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

    _spr->pushSprite(0, 0);
}

void LcarsEngine::_drawTransitionFrame(float t, LcarsScreen* screen) {
    const int16_t SW = LCARS_SIDEBAR_W;
    const int16_t R  = LCARS_ELBOW_R;
    const int16_t TH = LCARS_TOPBAR_H;
    const int16_t BH = LCARS_BOTBAR_H;
    const int16_t GAP = LCARS_BAR_GAP;
    const int16_t barX = SW + R + GAP;

    LcarsFrame::drawElbow(*_spr, 0, 0, SW, TH, R, _theme->elbowTop, LCARS_ELBOW_TL);
    LcarsFrame::drawElbow(*_spr, 0, _height - BH - R, SW, BH, R,
                          _theme->elbowBottom, LCARS_ELBOW_BL);

    int16_t sideTop = TH + R + 2;
    int16_t sideH = _height - TH - BH - 2 * R - 4;
    if (sideH > 0) {
        LcarsFrame::drawSidebar(*_spr, 0, sideTop, SW, sideH,
                                _theme->sidebar, _theme->sidebarCount, 2);
    }

    int16_t barEnd = barX + (int16_t)((_width - barX) * t);
    LcarsFrame::drawBarPartial(*_spr, barX, barEnd, 0, TH,
                               _theme->barTop, LCARS_CAP_PILL);
    LcarsFrame::drawBarPartial(*_spr, barX, barEnd, _height - BH, BH,
                               _theme->barBottom, LCARS_CAP_PILL);

    // Draw title on the extending bar (only when bar is long enough)
    const char* titleText = screen ? screen->title() : nullptr;
    int16_t titleX = barX + 4;
    if (titleText && titleText[0] && barEnd > titleX + 20) {
        LcarsFont::drawTextUpper(*_spr, titleText,
                                 titleX, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);
    }
}

void LcarsEngine::setBacklight(uint8_t brightness) {
    if (_blPin >= 0) {
        analogWrite(_blPin, brightness);
    }
}
