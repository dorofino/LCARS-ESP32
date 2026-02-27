#pragma once

#include <TFT_eSPI.h>

// ============================================================
// LCARS Font System
// Antonio font at multiple sizes, VLW format for TFT_eSPI
// All LCARS text is uppercase by convention
// ============================================================

// Font size identifiers
enum LcarsFontSize {
    LCARS_FONT_SM  = 0,   // 12px — labels, small status
    LCARS_FONT_14,         // 14px — compact labels
    LCARS_FONT_16,         // 16px — body text
    LCARS_FONT_MD,         // 18px — values, titles
    LCARS_FONT_20,         // 20px — medium emphasis
    LCARS_FONT_LG,         // 28px — large numbers
    LCARS_FONT_XL,         // 36px — primary display
    LCARS_FONT_COUNT
};

namespace LcarsFont {
    // Get the VLW font data pointer for a given size
    const uint8_t* get(LcarsFontSize size);

    // Get pixel height for a given font size
    uint8_t getHeight(LcarsFontSize size);

    // Draw text using smooth font (loads, draws, unloads)
    void drawText(TFT_eSprite& spr, const char* text, int16_t x, int16_t y,
                  LcarsFontSize size, uint16_t color,
                  uint16_t bgColor = 0x0000, uint8_t datum = TL_DATUM);

    // Draw text and auto-uppercase (LCARS convention)
    void drawTextUpper(TFT_eSprite& spr, const char* text, int16_t x, int16_t y,
                       LcarsFontSize size, uint16_t color,
                       uint16_t bgColor = 0x0000, uint8_t datum = TL_DATUM);

    // Get text width in pixels (loads font temporarily)
    int16_t textWidth(TFT_eSprite& spr, const char* text, LcarsFontSize size);
}
