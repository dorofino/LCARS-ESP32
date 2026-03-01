#include "lcars_frame.h"
#include "lcars_font.h"
#include <math.h>

// ============================================================
// Elbow gradient — RITOS-style 3D beveled shading
// ============================================================

static void _applyElbowGradient(TFT_eSprite& spr, int16_t x, int16_t y,
                                 int16_t w, int16_t h,
                                 LcarsElbowPos pos) {
    // Diagonal linear gradient — lighter at outer corner, darker toward inner curve
    // Mimics RITOS-style directional lighting
    for (int16_t py = 0; py < h; py++) {
        for (int16_t px = 0; px < w; px++) {
            int16_t sx = x + px, sy = y + py;
            uint16_t pixel = spr.readPixel(sx, sy);
            if (pixel == 0) continue;  // Skip black (background)

            // Normalized position within bounding box (0..1)
            float nx = (float)px / (float)(w - 1);
            float ny = (float)py / (float)(h - 1);

            // t = diagonal position: 0 = outer corner (light), 1 = inner curve (dark)
            float t;
            switch (pos) {
                case LCARS_ELBOW_TL: t = (nx + ny) * 0.5f; break;
                case LCARS_ELBOW_TR: t = ((1.0f - nx) + ny) * 0.5f; break;
                case LCARS_ELBOW_BL: t = (nx + (1.0f - ny)) * 0.5f; break;
                default:             t = ((1.0f - nx) + (1.0f - ny)) * 0.5f; break;
            }

            // Map: t=0 → lighten 15%, t=0.5 → neutral, t=1 → darken 20%
            if (t < 0.5f) {
                float strength = (0.5f - t) / 0.5f;  // 1..0
                uint8_t a = 255 - (uint8_t)(strength * 38);  // 217..255 (max 15% lighten)
                pixel = spr.alphaBlend(a, pixel, 0xFFFF);
            } else {
                float strength = (t - 0.5f) / 0.5f;  // 0..1
                uint8_t a = 255 - (uint8_t)(strength * 51);  // 255..204 (max 20% darken)
                pixel = spr.alphaBlend(a, pixel, 0x0000);
            }
            spr.drawPixel(sx, sy, pixel);
        }
    }
}

// ============================================================
// Elbow
// ============================================================

void LcarsFrame::drawElbow(TFT_eSprite& spr, int16_t x, int16_t y,
                            int16_t sideW, int16_t barH, int16_t innerR,
                            uint16_t color, LcarsElbowPos pos) {
    int16_t ew = sideW + innerR;  // Elbow total width
    int16_t eh = barH + innerR;   // Elbow total height

    // 1. Fill solid rect
    spr.fillRect(x, y, ew, eh, color);

    // 2. Apply gradient BEFORE cutting inner curve (avoids AA artifacts)
    _applyElbowGradient(spr, x, y, ew, eh, pos);

    // 3. Cut inner curve with black AA circle (clean edge, no gradient on AA pixels)
    switch (pos) {
        case LCARS_ELBOW_TL:
            spr.fillSmoothCircle(x + ew, y + eh, innerR, LCARS_BLACK);
            break;
        case LCARS_ELBOW_TR:
            spr.fillSmoothCircle(x - 1, y + eh, innerR, LCARS_BLACK);
            break;
        case LCARS_ELBOW_BL:
            spr.fillSmoothCircle(x + ew, y - 1, innerR, LCARS_BLACK);
            break;
        case LCARS_ELBOW_BR:
            spr.fillSmoothCircle(x - 1, y - 1, innerR, LCARS_BLACK);
            break;
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
                              int16_t gap, bool rightSide) {
    if (count == 0) return;

    int16_t totalGap = gap * (count - 1);
    int16_t segH = (h - totalGap) / count;

    for (uint8_t i = 0; i < count; i++) {
        int16_t sy = y + i * (segH + gap);
        int16_t actualH = (i == count - 1) ? (y + h - sy) : segH;

        spr.fillRect(x, sy, w, actualH, colors[i]);
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
    const bool hasRight = (theme.sidebarRightCount > 0);

    // Clear to black
    spr.fillSprite(theme.background);

    // ── Left side ──
    drawElbow(spr, 0, 0, SW, TH, R, theme.elbowTop, LCARS_ELBOW_TL);
    drawElbow(spr, 0, H - BH - R, SW, BH, R, theme.elbowBottom, LCARS_ELBOW_BL);

    int16_t barLX = SW + R + GAP;

    if (hasRight) {
        // ── Right side ──
        int16_t rElbowX = W - SW - R;
        drawElbow(spr, rElbowX, 0, SW, TH, R, theme.elbowTopRight, LCARS_ELBOW_TR);
        drawElbow(spr, rElbowX, H - BH - R, SW, BH, R, theme.elbowBottomRight, LCARS_ELBOW_BR);

        // ── Bars between elbows ──
        int16_t barRX = rElbowX - GAP;
        drawBar(spr, barLX, 0, barRX - barLX, TH, theme.barTop,
                LCARS_CAP_NONE, LCARS_CAP_NONE);
        drawBar(spr, barLX, H - BH, barRX - barLX, BH, theme.barBottom,
                LCARS_CAP_NONE, LCARS_CAP_NONE);

        // ── Right sidebar ──
        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH   = sideBot - sideTop;
        if (sideH > 0) {
            drawSidebar(spr, W - SW, sideTop, SW, sideH,
                        theme.sidebarRight, theme.sidebarRightCount, 2, true);
        }
    } else {
        // ── Bars with pill cap on right (legacy mode) ──
        drawBar(spr, barLX, 0, W - barLX, TH, theme.barTop,
                LCARS_CAP_NONE, LCARS_CAP_PILL);
        drawBar(spr, barLX, H - BH, W - barLX, BH, theme.barBottom,
                LCARS_CAP_NONE, LCARS_CAP_PILL);
    }

    // ── Left sidebar ──
    int16_t sideTop = TH + R + 2;
    int16_t sideBot = H - BH - R - 2;
    int16_t sideH   = sideBot - sideTop;

    if (sideH > 0 && theme.sidebarCount > 0) {
        drawSidebar(spr, 0, sideTop, SW, sideH,
                    theme.sidebar, theme.sidebarCount, 2);
    }

    // Return content area
    int16_t contentX = SW + R + GAP + 1;
    int16_t contentY = TH + 4;
    int16_t contentR = hasRight ? (W - SW - R - GAP - 1) : (W - 2);
    return {
        contentX,
        contentY,
        (int16_t)(contentR - contentX),
        (int16_t)(H - TH - BH - 6)
    };
}
