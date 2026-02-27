#include "lcars_font.h"
#include "fonts/smooth_font_12.h"
#include "fonts/smooth_font_14.h"
#include "fonts/smooth_font_16.h"
#include "fonts/smooth_font_18.h"
#include "fonts/smooth_font_20.h"
#include "fonts/smooth_font_28.h"
#include "fonts/smooth_font_36.h"

// ============================================================
// Font Data Table
// ============================================================

struct FontEntry {
    const uint8_t* data;
    uint8_t height;
};

static const FontEntry _fonts[LCARS_FONT_COUNT] = {
    { SmoothFont12, 12 },  // LCARS_FONT_SM
    { SmoothFont14, 14 },  // LCARS_FONT_14
    { SmoothFont16, 16 },  // LCARS_FONT_16
    { SmoothFont18, 18 },  // LCARS_FONT_MD
    { SmoothFont20, 20 },  // LCARS_FONT_20
    { SmoothFont28, 28 },  // LCARS_FONT_LG
    { SmoothFont36, 36 },  // LCARS_FONT_XL
};

// ============================================================
// Font API
// ============================================================

const uint8_t* LcarsFont::get(LcarsFontSize size) {
    if (size >= LCARS_FONT_COUNT) return _fonts[0].data;
    return _fonts[size].data;
}

uint8_t LcarsFont::getHeight(LcarsFontSize size) {
    if (size >= LCARS_FONT_COUNT) return _fonts[0].height;
    return _fonts[size].height;
}

void LcarsFont::drawText(TFT_eSprite& spr, const char* text,
                          int16_t x, int16_t y,
                          LcarsFontSize size, uint16_t color,
                          uint16_t bgColor, uint8_t datum) {
    spr.loadFont(get(size));
    spr.setTextDatum(datum);
    spr.setTextColor(color, bgColor);
    spr.drawString(text, x, y);
    spr.unloadFont();
}

void LcarsFont::drawTextUpper(TFT_eSprite& spr, const char* text,
                               int16_t x, int16_t y,
                               LcarsFontSize size, uint16_t color,
                               uint16_t bgColor, uint8_t datum) {
    // Convert to uppercase (LCARS convention: all text is uppercase)
    static char buf[128];
    size_t len = strlen(text);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    for (size_t i = 0; i < len; i++) {
        buf[i] = toupper((unsigned char)text[i]);
    }
    buf[len] = '\0';

    drawText(spr, buf, x, y, size, color, bgColor, datum);
}

int16_t LcarsFont::textWidth(TFT_eSprite& spr, const char* text,
                              LcarsFontSize size) {
    spr.loadFont(get(size));
    int16_t w = spr.textWidth(text);
    spr.unloadFont();
    return w;
}
