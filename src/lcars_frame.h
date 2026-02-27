#pragma once

#include <TFT_eSPI.h>
#include "lcars_theme.h"
#include "lcars_config.h"

// ============================================================
// LCARS Frame Primitives
// Procedural rendering of LCARS structural elements:
//   - Elbows (4 orientations, anti-aliased inner curve)
//   - Bars (horizontal, with optional pill-cap terminations)
//   - Sidebars (vertical, segmented with rounded ends)
//   - Standard frame (full LCARS border around content area)
// ============================================================

// Elbow orientation: which corner of the screen
enum LcarsElbowPos {
    LCARS_ELBOW_TL = 0,   // Top-left: sidebar goes down, bar goes right
    LCARS_ELBOW_TR,        // Top-right: sidebar goes down, bar goes left
    LCARS_ELBOW_BL,        // Bottom-left: sidebar goes up, bar goes right
    LCARS_ELBOW_BR,        // Bottom-right: sidebar goes up, bar goes left
};

// Pill cap style for bar terminations
enum LcarsBarCap {
    LCARS_CAP_NONE = 0,   // Flat end
    LCARS_CAP_PILL,        // Semicircle (radius = height/2)
    LCARS_CAP_ROUND,       // Small rounding
};

namespace LcarsFrame {

    // ── Elbow ───────────────────────────────────────────────
    // Draws an L-shaped elbow connecting a sidebar to a bar.
    // The inner curve is anti-aliased using drawSmoothArc.
    //
    //   x, y:     top-left corner of the elbow bounding box
    //   sideW:    sidebar width
    //   barH:     bar height
    //   innerR:   inner curve radius
    //   color:    fill color
    //   pos:      which corner orientation
    //
    void drawElbow(TFT_eSprite& spr, int16_t x, int16_t y,
                   int16_t sideW, int16_t barH, int16_t innerR,
                   uint16_t color, LcarsElbowPos pos);

    // ── Bar ─────────────────────────────────────────────────
    // Horizontal bar with optional pill caps at each end.
    //
    void drawBar(TFT_eSprite& spr, int16_t x, int16_t y,
                 int16_t w, int16_t h, uint16_t color,
                 LcarsBarCap leftCap = LCARS_CAP_NONE,
                 LcarsBarCap rightCap = LCARS_CAP_PILL);

    // ── Sidebar ─────────────────────────────────────────────
    // Vertical sidebar composed of colored segments.
    // Each segment has a slightly rounded right edge.
    //
    //   x, y:     top-left of sidebar area
    //   w:        sidebar width
    //   h:        total sidebar height
    //   colors:   array of segment colors
    //   count:    number of segments
    //   gap:      gap between segments (px)
    //
    void drawSidebar(TFT_eSprite& spr, int16_t x, int16_t y,
                     int16_t w, int16_t h,
                     const uint16_t* colors, uint8_t count,
                     int16_t gap = 2);

    // ── Standard LCARS Frame ────────────────────────────────
    // Draws the complete LCARS border:
    //   - Top-left elbow + top bar
    //   - Bottom-left elbow + bottom bar
    //   - Left sidebar segments
    //
    // Returns the content area rect (x, y, w, h) inside the frame.
    //
    struct Rect { int16_t x, y, w, h; };

    Rect drawStandardFrame(TFT_eSprite& spr, const LcarsTheme& theme);

    // ── Partial bar drawing (for animations) ────────────────
    // Draw a bar that extends from startX to endX (for grow/shrink animation)
    void drawBarPartial(TFT_eSprite& spr, int16_t startX, int16_t endX,
                        int16_t y, int16_t h, uint16_t color,
                        LcarsBarCap endCap = LCARS_CAP_PILL);
}
