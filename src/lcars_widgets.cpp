#include "lcars_widgets.h"
#include "lcars_colors.h"
#include "lcars_config.h"

// ============================================================
// Label
// ============================================================

void LcarsWidgets::drawLabel(TFT_eSprite& spr, int16_t x, int16_t y,
                              const char* text, uint16_t color,
                              LcarsFontSize size, uint8_t datum) {
    LcarsFont::drawTextUpper(spr, text, x, y, size, color, LCARS_BLACK, datum);
}

// ============================================================
// Value + Label
// ============================================================

void LcarsWidgets::drawValueLabel(TFT_eSprite& spr, int16_t x, int16_t y,
                                   const char* value, const char* label,
                                   uint16_t valueColor, uint16_t labelColor,
                                   LcarsFontSize valueSize,
                                   LcarsFontSize labelSize) {
    LcarsFont::drawText(spr, value, x, y, valueSize, valueColor);
    int16_t labelY = y + LcarsFont::getHeight(valueSize) + 6;
    LcarsFont::drawTextUpper(spr, label, x, labelY, labelSize, labelColor);
}

// ============================================================
// Progress Bar
// ============================================================

void LcarsWidgets::drawProgressBar(TFT_eSprite& spr, int16_t x, int16_t y,
                                    int16_t w, int16_t h, float percent,
                                    uint16_t fgColor, uint16_t bgColor) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    int16_t capR = h / 2;

    // Background track (full width, rounded)
    spr.fillSmoothRoundRect(x, y, w, h, capR, bgColor, LCARS_BLACK);

    // Foreground fill
    int16_t fillW = (int16_t)(w * percent);
    if (fillW > h) {  // Only draw if wider than a circle
        spr.fillSmoothRoundRect(x, y, fillW, h, capR, fgColor, LCARS_BLACK);
    } else if (fillW > 0) {
        spr.fillSmoothCircle(x + capR, y + capR, capR, fgColor, LCARS_BLACK);
    }
}

// ============================================================
// Status Row
// ============================================================

void LcarsWidgets::drawStatusRow(TFT_eSprite& spr, int16_t x, int16_t y,
                                  int16_t w, const char* label, const char* value,
                                  uint16_t valueColor, uint16_t labelColor,
                                  LcarsFontSize size) {
    // Label on left
    LcarsFont::drawTextUpper(spr, label, x, y, size, labelColor);

    // Value on right
    LcarsFont::drawTextUpper(spr, value, x + w, y, size, valueColor,
                              LCARS_BLACK, TR_DATUM);
}

// ============================================================
// Separator
// ============================================================

void LcarsWidgets::drawSeparator(TFT_eSprite& spr, int16_t x, int16_t y,
                                  int16_t w, uint16_t color) {
    spr.drawFastHLine(x, y, w, color);
}

// ============================================================
// Donut Gauge
// ============================================================

void LcarsWidgets::drawDonutGauge(TFT_eSprite& spr, int16_t cx, int16_t cy,
                                   int16_t outerR, int16_t thickness,
                                   float percent, uint16_t fgColor,
                                   uint16_t bgColor) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    int16_t innerR = outerR - thickness;
    if (innerR < 1) innerR = 1;

    // Background ring (full circle)
    spr.drawSmoothArc(cx, cy, outerR, innerR, 0, 360, bgColor, LCARS_BLACK, false);

    // Foreground arc (partial)
    if (percent > 0.01f) {
        // Start from top (270°), go clockwise
        uint32_t endAngle = 270 + (uint32_t)(percent * 360.0f);
        if (endAngle >= 360) {
            // Wraps around: draw in two segments
            spr.drawSmoothArc(cx, cy, outerR, innerR, 270, 360, fgColor, LCARS_BLACK, false);
            uint32_t remainder = endAngle - 360;
            if (remainder > 0) {
                spr.drawSmoothArc(cx, cy, outerR, innerR, 0, remainder, fgColor, LCARS_BLACK, false);
            }
        } else {
            spr.drawSmoothArc(cx, cy, outerR, innerR, 270, endAngle, fgColor, LCARS_BLACK, false);
        }
    }
}

// ============================================================
// Pill Button
// ============================================================

void LcarsWidgets::drawPillButton(TFT_eSprite& spr, int16_t x, int16_t y,
                                   int16_t w, int16_t h,
                                   const char* text, uint16_t color,
                                   uint16_t textColor, LcarsFontSize size) {
    int16_t capR = h / 2;
    spr.fillSmoothRoundRect(x, y, w, h, capR, color, LCARS_BLACK);

    // Center text
    LcarsFont::drawTextUpper(spr, text, x + w / 2, y + h / 2,
                              size, textColor, color, MC_DATUM);
}

// ============================================================
// Data Cascade
// ============================================================

void LcarsWidgets::drawDataCascade(TFT_eSprite& spr, int16_t x, int16_t y,
                                    int16_t w, int16_t h, uint16_t color,
                                    uint32_t seed) {
    // Generate pseudo-random hex digits that change each frame
    uint32_t t = (millis() / LCARS_CASCADE_SPEED_MS) + seed;
    char hexChars[] = "0123456789ABCDEF";
    int16_t charW = 7;
    int16_t charH = 10;
    int cols = w / charW;
    int rows = h / charH;

    spr.setTextFont(1);  // Built-in 8px font
    spr.setTextDatum(TL_DATUM);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            // Simple hash for pseudo-random appearance
            uint32_t hash = (t * 2654435761u) ^ (row * 31 + col * 17 + seed);
            char ch = hexChars[hash & 0x0F];
            // Vary brightness by position
            uint8_t fade = 128 + (hash >> 8) % 128;
            uint16_t c = color;  // Could vary intensity here
            spr.setTextColor(c, LCARS_BLACK);
            spr.drawChar(ch, x + col * charW, y + row * charH);
        }
    }
}

// ============================================================
// Blinking Indicator
// ============================================================

void LcarsWidgets::drawIndicator(TFT_eSprite& spr, int16_t x, int16_t y,
                                  int16_t radius, uint16_t color, bool active) {
    if (active) {
        bool blink = (millis() / LCARS_BLINK_INTERVAL_MS) % 2;
        if (blink) {
            spr.fillSmoothCircle(x, y, radius, color, LCARS_BLACK);
        } else {
            spr.drawSmoothArc(x, y, radius, radius - 1, 0, 360, color, LCARS_BLACK, false);
        }
    } else {
        spr.fillSmoothCircle(x, y, radius, color, LCARS_BLACK);
    }
}

// ============================================================
// Number Formatting
// ============================================================

void LcarsWidgets::formatCount(uint64_t value, char* out, size_t outLen) {
    if (value >= 1000000000ULL) {
        snprintf(out, outLen, "%.2fB", value / 1000000000.0);
    } else if (value >= 1000000ULL) {
        snprintf(out, outLen, "%.2fM", value / 1000000.0);
    } else if (value >= 1000ULL) {
        snprintf(out, outLen, "%.1fK", value / 1000.0);
    } else {
        snprintf(out, outLen, "%llu", (unsigned long long)value);
    }
}

void LcarsWidgets::formatCost(float value, char* out, size_t outLen) {
    if (value >= 1000.0f) {
        snprintf(out, outLen, "$%.0f", value);
    } else if (value >= 1.0f) {
        snprintf(out, outLen, "$%.2f", value);
    } else {
        snprintf(out, outLen, "$%.4f", value);
    }
}
