#include "lcars_screen.h"
#include "lcars_frame.h"
#include "lcars_font.h"
#include "lcars_config.h"
#include "lcars_animation.h"

// ============================================================
// Default transition: standard left-side LCARS frame stagger
// Screens with custom frame layouts should override this.
// ============================================================

void LcarsScreen::onDrawTransition(TFT_eSprite& spr, float t) {
    if (!_theme) return;

    const int16_t W = SCR_WIDTH;
    const int16_t H = SCR_HEIGHT;
    const int16_t SW = LCARS_SIDEBAR_W;
    const int16_t R  = LCARS_ELBOW_R;
    const int16_t TH = LCARS_TOPBAR_H;
    const int16_t BH = LCARS_BOTBAR_H;
    const int16_t GAP = LCARS_BAR_GAP;
    const int16_t barX = SW + R + GAP;

    // Phase 1 (t = 0.0 – 0.2): Elbows appear with power-on glow
    if (t > 0.0f) {
        // Glow: elbows flash bright then settle to normal color
        float glowT = (t < 0.2f) ? (1.0f - t / 0.2f) : 0.0f;
        uint8_t glow = (uint8_t)(glowT * 90);  // blend toward white
        uint16_t topC = spr.alphaBlend(255 - glow, _theme->elbowTop, TFT_WHITE);
        uint16_t botC = spr.alphaBlend(255 - glow, _theme->elbowBottom, TFT_WHITE);

        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R, topC, LCARS_ELBOW_TL);
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              botC, LCARS_ELBOW_BL);
    }

    // Phase 2 (t = 0.2 – 0.6): Bars extend with easeOutCubic (starts AFTER elbows)
    if (t > 0.2f) {
        float barT = (t - 0.2f) / 0.4f;
        if (barT > 1.0f) barT = 1.0f;
        barT = LcarsEasing::easeOutCubic(barT);

        int16_t barEnd = barX + (int16_t)((W - barX) * barT);
        if (barEnd > barX) {
            LcarsFrame::drawBarPartial(spr, barX, barEnd, 0, TH,
                                       _theme->barTop, LCARS_CAP_PILL);
            LcarsFrame::drawBarPartial(spr, barX, barEnd, H - BH, BH,
                                       _theme->barBottom, LCARS_CAP_PILL);
        }
    }

    // Phase 3 (t = 0.6 – 1.0): Sidebar segments reveal (starts AFTER bars complete)
    if (t > 0.6f) {
        float sideT = (t - 0.6f) / 0.4f;
        if (sideT > 1.0f) sideT = 1.0f;

        int16_t sideTop = TH + R + 2;
        int16_t sideH = H - TH - BH - 2 * R - 4;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            int showSegs = (int)(sideT * _theme->sidebarCount) + 1;
            if (showSegs > _theme->sidebarCount) showSegs = _theme->sidebarCount;
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, showSegs, 2);
        }
    }

    // Title appears once bars have extended enough
    if (t > 0.4f) {
        const char* t_title = title();
        int16_t titleX = barX + 4;
        if (t_title && t_title[0]) {
            LcarsFont::drawTextUpper(spr, t_title,
                                     titleX, TH / 2,
                                     LCARS_FONT_SM, _theme->textOnBar,
                                     _theme->barTop, ML_DATUM);
        }
    }
}
