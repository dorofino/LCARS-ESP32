#pragma once

#include <stdint.h>
#include "lcars_colors.h"

// ============================================================
// LCARS Theme System
// A theme defines the color mapping for all LCARS elements
// ============================================================

struct LcarsTheme {
    const char* name;

    // Structural frame colors
    uint16_t elbowTop;          // Top elbow color
    uint16_t elbowBottom;       // Bottom elbow color
    uint16_t barTop;            // Top bar extension color
    uint16_t barBottom;         // Bottom bar extension color

    // Sidebar segment colors (up to 4 segments)
    uint16_t sidebar[4];
    uint8_t  sidebarCount;      // How many sidebar segments to use

    // Content colors
    uint16_t text;              // Primary text
    uint16_t textDim;           // Secondary/label text
    uint16_t textOnBar;         // Text rendered on colored bars
    uint16_t accent;            // Highlight / accent

    // Widget colors
    uint16_t progressFg;        // Progress bar foreground
    uint16_t progressBg;        // Progress bar background
    uint16_t gaugeColor;        // Donut gauge default color

    // Status colors
    uint16_t statusOk;          // OK / online
    uint16_t statusWarn;        // Warning
    uint16_t statusErr;         // Error / critical
    uint16_t alert;             // Red alert overlay

    // Background (always black for LCARS)
    uint16_t background;
};

// ── Preset Themes ───────────────────────────────────────────

enum LcarsThemeId {
    LCARS_THEME_TNG = 0,       // Classic TNG orange/amber/violet
    LCARS_THEME_NEMESIS,       // Star Trek Nemesis blue palette
    LCARS_THEME_RED_ALERT,     // All red, flashing
    LCARS_THEME_COUNT
};

namespace LcarsThemes {
    const LcarsTheme& get(LcarsThemeId id);
    const LcarsTheme& tng();
    const LcarsTheme& nemesis();
    const LcarsTheme& redAlert();
}
