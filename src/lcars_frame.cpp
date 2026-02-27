#include "lcars_frame.h"
#include "lcars_font.h"

// ============================================================
// Internal: Anti-aliased quarter circle fill
// Uses scanline fill with single-pixel AA at the edge
// ============================================================

static void fillQuarterCircleAA(TFT_eSprite& spr, int16_t cx, int16_t cy,
                                 int16_t r, uint16_t color, uint16_t bgColor,
                                 LcarsElbowPos quadrant) {
    // Use TFT_eSPI's drawSmoothArc for anti-aliased rendering
    // Arc angles (TFT_eSPI convention: 0=right, 90=down, 180=left, 270=up, clockwise)
    //
    // We draw a FILLED sector (ir=0) of the background color to cut into the frame,
    // creating the smooth inner curve of the elbow.
    //
    // For each elbow orientation, the curve cuts from the inner corner
    // into the colored area:

    uint32_t startAngle, endAngle;

    switch (quadrant) {
        case LCARS_ELBOW_TL:
            // Inner corner at bottom-right of elbow, curve cuts toward top-left
            startAngle = 180; endAngle = 270;
            break;
        case LCARS_ELBOW_TR:
            // Inner corner at bottom-left of elbow, curve cuts toward top-right
            startAngle = 270; endAngle = 360;
            break;
        case LCARS_ELBOW_BL:
            // Inner corner at top-right of elbow, curve cuts toward bottom-left
            startAngle = 90; endAngle = 180;
            break;
        case LCARS_ELBOW_BR:
            // Inner corner at top-left of elbow, curve cuts toward bottom-right
            startAngle = 0; endAngle = 90;
            break;
    }

    // drawSmoothArc: filled sector (inner radius = 0)
    // fg = black (cutting out), bg = colored frame (for AA blending)
    spr.drawSmoothArc(cx, cy, r, 0, startAngle, endAngle, color, bgColor, false);
}

// ============================================================
// Elbow
// ============================================================

void LcarsFrame::drawElbow(TFT_eSprite& spr, int16_t x, int16_t y,
                            int16_t sideW, int16_t barH, int16_t innerR,
                            uint16_t color, LcarsElbowPos pos) {
    int16_t ew = sideW + innerR;  // Elbow total width
    int16_t eh = barH + innerR;   // Elbow total height

    switch (pos) {
        case LCARS_ELBOW_TL:
            // ████████████  <- bar row (full width)
            // ████████████
            // ██████╲
            // ██████ │     <- sidebar column (sideW only)
            spr.fillRect(x, y, ew, barH, color);            // Top row
            spr.fillRect(x, y + barH, sideW, innerR, color); // Left column
            // Cut the inner curve: black quarter-circle at (x+sideW, y+barH)
            fillQuarterCircleAA(spr, x + sideW, y + barH, innerR,
                                LCARS_BLACK, color, LCARS_ELBOW_TL);
            break;

        case LCARS_ELBOW_TR:
            // ████████████  <- bar row
            // ████████████
            //       ╱██████
            //      │ ██████  <- sidebar on right
            spr.fillRect(x, y, ew, barH, color);
            spr.fillRect(x + innerR, y + barH, sideW, innerR, color);
            fillQuarterCircleAA(spr, x + innerR, y + barH, innerR,
                                LCARS_BLACK, color, LCARS_ELBOW_TR);
            break;

        case LCARS_ELBOW_BL:
            // ██████ │     <- sidebar on left
            // ██████╱
            // ████████████
            // ████████████  <- bar row
            spr.fillRect(x, y, sideW, innerR, color);
            spr.fillRect(x, y + innerR, ew, barH, color);
            fillQuarterCircleAA(spr, x + sideW, y + innerR, innerR,
                                LCARS_BLACK, color, LCARS_ELBOW_BL);
            break;

        case LCARS_ELBOW_BR:
            //      │ ██████  <- sidebar on right
            //       ╲██████
            // ████████████
            // ████████████  <- bar row
            spr.fillRect(x + innerR, y, sideW, innerR, color);
            spr.fillRect(x, y + innerR, ew, barH, color);
            fillQuarterCircleAA(spr, x + innerR, y + innerR, innerR,
                                LCARS_BLACK, color, LCARS_ELBOW_BR);
            break;
    }
}

// ============================================================
// Bar
// ============================================================

void LcarsFrame::drawBar(TFT_eSprite& spr, int16_t x, int16_t y,
                          int16_t w, int16_t h, uint16_t color,
                          LcarsBarCap leftCap, LcarsBarCap rightCap) {
    int16_t capR = h / 2;

    // Draw the main body
    int16_t bodyX = x;
    int16_t bodyW = w;

    if (leftCap == LCARS_CAP_PILL) {
        bodyX += capR;
        bodyW -= capR;
    }
    if (rightCap == LCARS_CAP_PILL) {
        bodyW -= capR;
    }

    if (bodyW > 0) {
        spr.fillRect(bodyX, y, bodyW, h, color);
    }

    // Draw pill caps (semicircles)
    if (leftCap == LCARS_CAP_PILL) {
        spr.fillSmoothCircle(x + capR, y + capR, capR, color, LCARS_BLACK);
        // Fill the right half to merge with body
        spr.fillRect(x + capR, y, 1, h, color);
    }
    if (rightCap == LCARS_CAP_PILL) {
        spr.fillSmoothCircle(x + w - capR - 1, y + capR, capR, color, LCARS_BLACK);
        spr.fillRect(x + w - capR - 1, y, 1, h, color);
    }
}

void LcarsFrame::drawBarPartial(TFT_eSprite& spr, int16_t startX, int16_t endX,
                                 int16_t y, int16_t h, uint16_t color,
                                 LcarsBarCap endCap) {
    if (endX <= startX) return;

    int16_t capR = h / 2;
    int16_t barW = endX - startX;

    if (endCap == LCARS_CAP_PILL && barW > capR * 2) {
        spr.fillRect(startX, y, barW - capR, h, color);
        spr.fillSmoothCircle(endX - capR - 1, y + capR, capR, color, LCARS_BLACK);
        spr.fillRect(endX - capR - 1, y, 1, h, color);
    } else {
        spr.fillRect(startX, y, barW, h, color);
    }
}

// ============================================================
// Sidebar
// ============================================================

void LcarsFrame::drawSidebar(TFT_eSprite& spr, int16_t x, int16_t y,
                              int16_t w, int16_t h,
                              const uint16_t* colors, uint8_t count,
                              int16_t gap) {
    if (count == 0) return;

    int16_t totalGap = gap * (count - 1);
    int16_t segH = (h - totalGap) / count;

    for (uint8_t i = 0; i < count; i++) {
        int16_t sy = y + i * (segH + gap);
        int16_t actualH = (i == count - 1) ? (y + h - sy) : segH;

        // Main segment rectangle (left-aligned, flat left edge)
        spr.fillRect(x, sy, w - 4, actualH, colors[i]);

        // Rounded right edge (pill-like right side)
        spr.fillSmoothRoundRect(x + w - 8, sy, 8, actualH, 4, colors[i], LCARS_BLACK);
    }
}

// ============================================================
// Standard Frame
// ============================================================

LcarsFrame::Rect LcarsFrame::drawStandardFrame(TFT_eSprite& spr,
                                                 const LcarsTheme& theme) {
    const int16_t W = SCR_WIDTH;
    const int16_t H = SCR_HEIGHT;
    const int16_t SW = LCARS_SIDEBAR_W;
    const int16_t R  = LCARS_ELBOW_R;
    const int16_t TH = LCARS_TOPBAR_H;
    const int16_t BH = LCARS_BOTBAR_H;
    const int16_t GAP = LCARS_BAR_GAP;

    // Clear to black
    spr.fillSprite(theme.background);

    // ── Top-left elbow ──
    drawElbow(spr, 0, 0, SW, TH, R, theme.elbowTop, LCARS_ELBOW_TL);

    // ── Top bar (extends from elbow to right edge) ──
    int16_t barX = SW + R + GAP;
    drawBar(spr, barX, 0, W - barX, TH, theme.barTop,
            LCARS_CAP_NONE, LCARS_CAP_PILL);

    // ── Bottom-left elbow ──
    drawElbow(spr, 0, H - BH - R, SW, BH, R, theme.elbowBottom, LCARS_ELBOW_BL);

    // ── Bottom bar ──
    drawBar(spr, barX, H - BH, W - barX, BH, theme.barBottom,
            LCARS_CAP_NONE, LCARS_CAP_PILL);

    // ── Sidebar segments (between elbows) ──
    int16_t sideTop = TH + R + 2;
    int16_t sideBot = H - BH - R - 2;
    int16_t sideH   = sideBot - sideTop;

    if (sideH > 0 && theme.sidebarCount > 0) {
        drawSidebar(spr, 0, sideTop, SW, sideH,
                    theme.sidebar, theme.sidebarCount, 2);
    }

    // Return content area
    return {
        (int16_t)(SW + R + 4),
        (int16_t)(TH + 8),
        (int16_t)(W - SW - R - 8),
        (int16_t)(H - TH - BH - 12)
    };
}
