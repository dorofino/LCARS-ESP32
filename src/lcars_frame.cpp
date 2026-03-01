#include "lcars_frame.h"
#include "lcars_font.h"

// ============================================================
// Elbow
// ============================================================

void LcarsFrame::drawElbow(TFT_eSprite& spr, int16_t x, int16_t y,
                            int16_t sideW, int16_t barH, int16_t innerR,
                            uint16_t color, LcarsElbowPos pos) {
    int16_t ew = sideW + innerR;  // Elbow total width
    int16_t eh = barH + innerR;   // Elbow total height

    // Additive approach: draw bar rect + sidebar rect + colored arc
    // This avoids holes from drawSmoothArc(ir=0) not filling to center

    switch (pos) {
        case LCARS_ELBOW_TL: {
            // ████████████  <- bar row (full width ew)
            // ██████╲
            // ██████ │     <- sidebar column (sideW)
            int16_t cx = x + sideW, cy = y + barH;
            spr.fillRect(x, y, ew, barH, color);                    // Bar
            spr.fillRect(x, cy, sideW, innerR, color);              // Sidebar
            spr.fillCircle(cx, cy, innerR - 1, color);              // Solid fill under arc
            spr.drawSmoothArc(cx, cy, innerR, 0,
                              270, 360, color, LCARS_BLACK, false);
            break;
        }
        case LCARS_ELBOW_TR: {
            // ████████████  <- bar row
            //       ╱██████
            //      │ ██████  <- sidebar on right
            int16_t cx = x + innerR, cy = y + barH;
            spr.fillRect(x, y, ew, barH, color);                    // Bar
            spr.fillRect(cx, cy, sideW, innerR, color);             // Sidebar
            spr.fillCircle(cx, cy, innerR - 1, color);              // Solid fill under arc
            spr.drawSmoothArc(cx, cy, innerR, 0,
                              0, 90, color, LCARS_BLACK, false);
            break;
        }
        case LCARS_ELBOW_BL: {
            // ██████ │     <- sidebar on left
            // ██████╱
            // ████████████  <- bar row
            int16_t cx = x + sideW, cy = y + innerR;
            spr.fillRect(x, y, sideW, innerR, color);               // Sidebar
            spr.fillRect(x, cy, ew, barH, color);                   // Bar
            spr.fillCircle(cx, cy, innerR - 1, color);              // Solid fill under arc
            spr.drawSmoothArc(cx, cy, innerR, 0,
                              180, 270, color, LCARS_BLACK, false);
            break;
        }
        case LCARS_ELBOW_BR: {
            //      │ ██████  <- sidebar on right
            //       ╲██████
            // ████████████  <- bar row
            int16_t cx = x + innerR, cy = y + innerR;
            spr.fillRect(cx, y, sideW, innerR, color);              // Sidebar
            spr.fillRect(x, cy, ew, barH, color);                   // Bar
            spr.fillCircle(cx, cy, innerR - 1, color);              // Solid fill under arc
            spr.drawSmoothArc(cx, cy, innerR, 0,
                              90, 180, color, LCARS_BLACK, false);
            break;
        }
    }
}

// ============================================================
// Bar
// ============================================================

void LcarsFrame::drawBar(TFT_eSprite& spr, int16_t x, int16_t y,
                          int16_t w, int16_t h, uint16_t color,
                          LcarsBarCap leftCap, LcarsBarCap rightCap) {
    int16_t r = h / 2;
    bool roundL = (leftCap == LCARS_CAP_PILL);
    bool roundR = (rightCap == LCARS_CAP_PILL);

    if (roundL || roundR) {
        // Draw full rounded rect, then square off unwanted sides
        spr.fillSmoothRoundRect(x, y, w, h, r, color, LCARS_BLACK);
        if (!roundL) {
            spr.fillRect(x, y, r, h, color);       // Square off left
        }
        if (!roundR) {
            spr.fillRect(x + w - r, y, r, h, color); // Square off right
        }
    } else {
        spr.fillRect(x, y, w, h, color);
    }
}

void LcarsFrame::drawBarPartial(TFT_eSprite& spr, int16_t startX, int16_t endX,
                                 int16_t y, int16_t h, uint16_t color,
                                 LcarsBarCap endCap) {
    if (endX <= startX) return;

    int16_t r = h / 2;
    int16_t barW = endX - startX;

    if (endCap == LCARS_CAP_PILL && barW > r * 2) {
        // Rounded right end, flat left end
        spr.fillSmoothRoundRect(startX, y, barW, h, r, color, LCARS_BLACK);
        spr.fillRect(startX, y, r, h, color);  // Square off left
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

        // Full-width rounded rect, then square off left side
        spr.fillSmoothRoundRect(x, sy, w, actualH, 4, colors[i], LCARS_BLACK);
        spr.fillRect(x, sy, 4, actualH, colors[i]);  // Flat left edge
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

    // Return content area (tight fit for small displays)
    return {
        (int16_t)(SW + R + GAP + 1),
        (int16_t)(TH + 4),
        (int16_t)(W - SW - R - GAP - 3),
        (int16_t)(H - TH - BH - 6)
    };
}
