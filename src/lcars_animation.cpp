#include "lcars_animation.h"
#include "lcars_frame.h"
#include "lcars_font.h"
#include "lcars_config.h"
#include "lcars_colors.h"

// ============================================================
// Boot Sequence Diagnostic Messages
// ============================================================

const char* LcarsBootScreen::_diagMessages[] = {
    "LCARS INTERFACE SYSTEM",
    "BUILD 47391.2",
    "",
    "INITIALIZING DISPLAY CORE..........OK",
    "LOADING FONT DATABASE..............OK",
    "THEME ENGINE ONLINE................OK",
    "WIDGET SUBSYSTEM READY.............OK",
    "SENSOR ARRAY CALIBRATING...........OK",
    "COMMUNICATIONS ARRAY...............STANDBY",
    "",
    "ALL SYSTEMS NOMINAL",
    "TRANSFERRING CONTROL TO PRIMARY DISPLAY",
};

const int LcarsBootScreen::_diagCount = sizeof(_diagMessages) / sizeof(_diagMessages[0]);

// ============================================================
// Boot Screen Implementation
// ============================================================

void LcarsBootScreen::onSetup() {
    _phase = BOOT_DIAG_TEXT;
    _phaseStartMs = millis();
    _elapsed = 0;
    _diagLine = 0;
    _complete = false;
}

void LcarsBootScreen::onUpdate(uint32_t deltaMs) {
    _elapsed = millis() - _phaseStartMs;

    switch (_phase) {
        case BOOT_DIAG_TEXT:
            if (_elapsed >= LCARS_BOOT_DIAG_MS) {
                _phase = BOOT_FRAME_BUILD;
                _phaseStartMs = millis();
                _elapsed = 0;
            }
            break;

        case BOOT_FRAME_BUILD:
            if (_elapsed >= LCARS_BOOT_FRAME_MS) {
                _phase = BOOT_DONE;
                _complete = true;
            }
            break;

        case BOOT_DONE:
            break;
    }
}

void LcarsBootScreen::onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& content) {
    spr.fillSprite(LCARS_BLACK);

    switch (_phase) {
        case BOOT_DIAG_TEXT:
            _drawDiagPhase(spr);
            break;
        case BOOT_FRAME_BUILD:
            _drawFrameBuildPhase(spr);
            break;
        case BOOT_DONE:
            _drawFrameBuildPhase(spr);
            break;
    }
}

void LcarsBootScreen::_drawDiagPhase(TFT_eSprite& spr) {
    // Reveal diagnostic lines progressively
    float progress = (float)_elapsed / LCARS_BOOT_DIAG_MS;
    int showLines = (int)(progress * (_diagCount + 2));
    if (showLines > _diagCount) showLines = _diagCount;

    int16_t lineH = 13;
    int16_t startY = 8;
    int16_t x = 8;

    for (int i = 0; i < showLines; i++) {
        const char* msg = _diagMessages[i];
        if (msg[0] == '\0') continue;

        uint16_t color = LCARS_AMBER;
        // Highlight "OK" lines with a different color
        if (strstr(msg, "OK")) {
            // Draw the "OK" part in green
            LcarsFont::drawText(spr, msg, x, startY + i * lineH,
                                LCARS_FONT_SM, LCARS_AMBER);
        } else if (strstr(msg, "NOMINAL")) {
            LcarsFont::drawText(spr, msg, x, startY + i * lineH,
                                LCARS_FONT_SM, LCARS_GREEN);
        } else if (i == 0) {
            // Title line — larger
            LcarsFont::drawText(spr, msg, x, startY + i * lineH,
                                LCARS_FONT_MD, LCARS_GOLD);
        } else {
            LcarsFont::drawText(spr, msg, x, startY + i * lineH,
                                LCARS_FONT_SM, color);
        }
    }

    // Blinking cursor
    if ((millis() / 300) % 2) {
        int16_t cursorY = startY + showLines * lineH;
        spr.fillRect(x, cursorY, 6, 10, LCARS_AMBER);
    }
}

void LcarsBootScreen::_drawFrameBuildPhase(TFT_eSprite& spr) {
    if (!_theme) return;

    float t = (float)_elapsed / LCARS_BOOT_FRAME_MS;
    if (t > 1.0f) t = 1.0f;
    t = LcarsEasing::easeOutCubic(t);

    const int16_t W = SCR_WIDTH;
    const int16_t H = SCR_HEIGHT;
    const int16_t SW = LCARS_SIDEBAR_W;
    const int16_t R  = LCARS_ELBOW_R;
    const int16_t TH = LCARS_TOPBAR_H;
    const int16_t BH = LCARS_BOTBAR_H;
    const int16_t GAP = LCARS_BAR_GAP;
    const int16_t barX = SW + R + GAP;

    // Phase 1 (t 0-0.4): Elbows appear (fade in via drawing)
    if (t > 0.0f) {
        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
    }

    // Phase 2 (t 0.2-0.7): Bars extend from elbows
    float barT = (t - 0.2f) / 0.5f;
    if (barT < 0.0f) barT = 0.0f;
    if (barT > 1.0f) barT = 1.0f;
    barT = LcarsEasing::easeOutQuad(barT);

    int16_t barEnd = barX + (int16_t)((W - barX) * barT);
    if (barEnd > barX) {
        LcarsFrame::drawBarPartial(spr, barX, barEnd, 0, TH,
                                   _theme->barTop, LCARS_CAP_PILL);
        LcarsFrame::drawBarPartial(spr, barX, barEnd, H - BH, BH,
                                   _theme->barBottom, LCARS_CAP_PILL);
    }

    // Phase 3 (t 0.4-1.0): Sidebar segments fill in
    float sideT = (t - 0.4f) / 0.6f;
    if (sideT < 0.0f) sideT = 0.0f;
    if (sideT > 1.0f) sideT = 1.0f;

    int16_t sideTop = TH + R + 2;
    int16_t sideBot = H - BH - R - 2;
    int16_t sideH = sideBot - sideTop;

    if (sideH > 0 && sideT > 0.0f && _theme->sidebarCount > 0) {
        // Reveal segments one by one
        int showSegs = (int)(sideT * _theme->sidebarCount) + 1;
        if (showSegs > _theme->sidebarCount) showSegs = _theme->sidebarCount;

        LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                _theme->sidebar, showSegs, 2);
    }
}

// ============================================================
// Easing Functions
// ============================================================

float LcarsEasing::linear(float t) {
    return t;
}

float LcarsEasing::easeInQuad(float t) {
    return t * t;
}

float LcarsEasing::easeOutQuad(float t) {
    return t * (2.0f - t);
}

float LcarsEasing::easeInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float LcarsEasing::easeOutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}
