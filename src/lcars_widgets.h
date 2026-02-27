#pragma once

#include <TFT_eSPI.h>
#include "lcars_theme.h"
#include "lcars_font.h"

// ============================================================
// LCARS Widgets
// Reusable UI components for LCARS screens
// ============================================================

namespace LcarsWidgets {

    // ── Label ───────────────────────────────────────────────
    // Text label (auto-uppercase)
    void drawLabel(TFT_eSprite& spr, int16_t x, int16_t y,
                   const char* text, uint16_t color,
                   LcarsFontSize size = LCARS_FONT_SM,
                   uint8_t datum = TL_DATUM);

    // ── Value Display ───────────────────────────────────────
    // Large value with small label underneath
    void drawValueLabel(TFT_eSprite& spr, int16_t x, int16_t y,
                        const char* value, const char* label,
                        uint16_t valueColor, uint16_t labelColor,
                        LcarsFontSize valueSize = LCARS_FONT_LG,
                        LcarsFontSize labelSize = LCARS_FONT_SM);

    // ── Progress Bar ────────────────────────────────────────
    // Horizontal progress bar with LCARS styling
    void drawProgressBar(TFT_eSprite& spr, int16_t x, int16_t y,
                         int16_t w, int16_t h, float percent,
                         uint16_t fgColor, uint16_t bgColor);

    // ── Status Row ──────────────────────────────────────────
    // "LABEL .................. VALUE" with dot leader
    void drawStatusRow(TFT_eSprite& spr, int16_t x, int16_t y,
                       int16_t w, const char* label, const char* value,
                       uint16_t valueColor, uint16_t labelColor,
                       LcarsFontSize size = LCARS_FONT_SM);

    // ── Separator ───────────────────────────────────────────
    // Thin horizontal line
    void drawSeparator(TFT_eSprite& spr, int16_t x, int16_t y,
                       int16_t w, uint16_t color);

    // ── Donut Gauge ─────────────────────────────────────────
    // Circular gauge (like RITOS ring indicators)
    void drawDonutGauge(TFT_eSprite& spr, int16_t cx, int16_t cy,
                        int16_t outerR, int16_t thickness,
                        float percent, uint16_t fgColor, uint16_t bgColor);

    // ── Pill Button ─────────────────────────────────────────
    // LCARS pill-shaped button with label
    void drawPillButton(TFT_eSprite& spr, int16_t x, int16_t y,
                        int16_t w, int16_t h,
                        const char* text, uint16_t color,
                        uint16_t textColor = 0x0000,
                        LcarsFontSize size = LCARS_FONT_SM);

    // ── Data Cascade ────────────────────────────────────────
    // Streaming random hex/numbers (visual flair)
    void drawDataCascade(TFT_eSprite& spr, int16_t x, int16_t y,
                         int16_t w, int16_t h, uint16_t color,
                         uint32_t seed = 0);

    // ── Blinking Indicator ──────────────────────────────────
    // Small circle that blinks at LCARS_BLINK_INTERVAL_MS
    void drawIndicator(TFT_eSprite& spr, int16_t x, int16_t y,
                       int16_t radius, uint16_t color, bool active = true);

    // ── Number Formatting ───────────────────────────────────
    // Format large numbers: 1234567 → "1.23M"
    void formatCount(uint64_t value, char* out, size_t outLen);

    // Format cost: 1234.56 → "$1,234.56"
    void formatCost(float value, char* out, size_t outLen);
}
