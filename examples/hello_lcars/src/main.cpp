// ============================================================
// Hello LCARS — Demo for lcars-esp32 engine
//
// Shows:
//   1. Animated boot sequence (diagnostic text + frame assembly)
//   2. Dashboard screen with bars + donut gauges
//   3. Status screen with data cascade
//   4. Tactical screen with buttons + indicators + value labels
//   5. Engineering screen with large gauge + cascade
// ============================================================

#include <lcars.h>
#include <math.h>

TFT_eSPI tft;
LcarsEngine engine;

// ============================================================
// Dashboard Screen — bars + gauges demo
// ============================================================

class DashboardScreen : public LcarsScreen {
public:
    const char* title() const override { return "MAIN SYSTEMS"; }
    uint32_t refreshIntervalMs() const override { return 100; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Section header
        LcarsWidgets::drawLabel(spr, x, y, "POWER SYSTEMS", _theme->accent);
        y += 14;

        // Warp core
        LcarsWidgets::drawStatusRow(spr, x, y, w, "WARP CORE", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, 0.78f,
                                      LCARS_ICE, LCARS_BAR_TRACK);
        y += 7;

        // Shields
        LcarsWidgets::drawStatusRow(spr, x, y, w, "SHIELDS", "87%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, 0.87f,
                                      LCARS_VIOLET, LCARS_BAR_TRACK);
        y += 7;

        // Weapons
        LcarsWidgets::drawStatusRow(spr, x, y, w, "WEAPONS", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, 0.45f,
                                      LCARS_GOLD, LCARS_BAR_TRACK);
        y += 7;

        // Separator
        y += 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        // Donut gauges — compact, 3 across, labels on left, centered
        int16_t gaugeR = 11;
        int16_t thickness = 3;
        int16_t labelW = 28;  // space for 3-char label
        int16_t cellW = (w - labelW * 3) / 3 + labelW;
        int16_t gaugeRowW = cellW * 2 + labelW + gaugeR * 2;
        int16_t gx = x + (w - gaugeRowW) / 2;  // centered offset
        int16_t gaugeCY = y + gaugeR + 5;

        // EPS
        LcarsWidgets::drawLabel(spr, gx, gaugeCY,
                                "EPS", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        LcarsWidgets::drawDonutGauge(spr, gx + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, 0.78f,
                                     LCARS_ICE, LCARS_BAR_TRACK);

        // SIF
        LcarsWidgets::drawLabel(spr, gx + cellW, gaugeCY,
                                "SIF", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        LcarsWidgets::drawDonutGauge(spr, gx + cellW + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, 0.92f,
                                     LCARS_GREEN, LCARS_BAR_TRACK);

        // DEF
        LcarsWidgets::drawLabel(spr, gx + cellW * 2, gaugeCY,
                                "DEF", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        LcarsWidgets::drawDonutGauge(spr, gx + cellW * 2 + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, 0.34f,
                                     LCARS_TOMATO, LCARS_BAR_TRACK);
    }
};

// ============================================================
// Status Screen — status rows + data cascade
// ============================================================

class StatusScreen : public LcarsScreen {
public:
    const char* title() const override { return "SYSTEM STATUS"; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Stardate
        char stardate[32];
        uint32_t sd = 47000 + (millis() / 100) % 999;
        snprintf(stardate, sizeof(stardate), "%d.%d", sd / 10, sd % 10);
        LcarsWidgets::drawLabel(spr, x, y, "STARDATE", _theme->textDim);
        LcarsFont::drawText(spr, stardate, x + 80, y, LCARS_FONT_SM, _theme->accent);
        y += 15;

        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        // Status rows — generous spacing
        LcarsWidgets::drawStatusRow(spr, x, y, w, "HULL INTEGRITY", "98.7%",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "LIFE SUPPORT", "NOMINAL",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "INERTIAL DAMPERS", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "DEFLECTOR ARRAY", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "COMM ARRAY", "ACTIVE",
                                    _theme->statusOk, _theme->textDim);
        y += 17;

        // Data cascade fills remaining space
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 3;
        int16_t cascadeH = c.y + c.h - y;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, x, y, w, cascadeH, _theme->accent);
        }
    }
};

// ============================================================
// Tactical Screen — pill buttons + indicators + value labels
// ============================================================

class TacticalScreen : public LcarsScreen {
public:
    const char* title() const override { return "TACTICAL"; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Row of pill buttons
        int16_t btnW = (w - 6) / 3;
        int16_t btnH = 16;

        LcarsWidgets::drawPillButton(spr, x, y, btnW, btnH,
                                     "PHASERS", LCARS_TOMATO, LCARS_WHITE);
        LcarsWidgets::drawPillButton(spr, x + btnW + 3, y, btnW, btnH,
                                     "TORPEDOES", LCARS_AMBER, LCARS_BLACK);
        LcarsWidgets::drawPillButton(spr, x + (btnW + 3) * 2, y, btnW, btnH,
                                     "SHIELDS", LCARS_ICE, LCARS_BLACK);
        y += btnH + 4;

        int16_t sep1Y = y;
        LcarsWidgets::drawSeparator(spr, x, sep1Y, w, _theme->textDim);

        // Value labels section — vertically centered between separators
        int16_t sectionH = 52;   // total height of section between separators
        int16_t contentH = 46;   // LG(28) + gap(6) + SM(12)
        int16_t valY = sep1Y + 1 + (sectionH - contentH) / 2;
        int16_t colW = w / 3;

        // Phaser power (animated)
        char phaserBuf[8];
        int phaserPct = 85 + (millis() / 200) % 15;
        snprintf(phaserBuf, sizeof(phaserBuf), "%d%%", phaserPct);
        LcarsWidgets::drawValueLabel(spr, x + 4, valY, phaserBuf, "PHASER PWR",
                                     LCARS_TOMATO, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        // Torpedo count
        LcarsWidgets::drawValueLabel(spr, x + colW + 4, valY, "247", "TORPEDOES",
                                     LCARS_AMBER, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        // Shield frequency (animated)
        char freqBuf[12];
        float freq = 257.4f + (float)((millis() / 300) % 20) * 0.1f;
        snprintf(freqBuf, sizeof(freqBuf), "%.1f", freq);
        LcarsWidgets::drawValueLabel(spr, x + colW * 2 + 4, valY, freqBuf, "SHIELD MHZ",
                                     LCARS_ICE, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        y = sep1Y + sectionH + 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        // Blinking indicators — row 1 (dot and text share same vertical center)
        int16_t indR = 4;
        int16_t indCY = y + 6;  // center Y for indicator dot and text

        LcarsWidgets::drawIndicator(spr, x + 8, indCY, indR, LCARS_TOMATO, true);
        LcarsWidgets::drawLabel(spr, x + 16, indCY, "ALERT",
                                LCARS_TOMATO, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 68, indCY, indR, LCARS_GREEN, false);
        LcarsWidgets::drawLabel(spr, x + 76, indCY, "SHIELDS",
                                LCARS_GREEN, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 142, indCY, indR, LCARS_AMBER, true);
        LcarsWidgets::drawLabel(spr, x + 150, indCY, "TARGET LOCK",
                                LCARS_AMBER, LCARS_FONT_SM, ML_DATUM);
        y += 16;

        // Blinking indicators — row 2
        indCY = y + 6;

        LcarsWidgets::drawIndicator(spr, x + 8, indCY, indR, LCARS_VIOLET, false);
        LcarsWidgets::drawLabel(spr, x + 16, indCY, "CLOAK DET",
                                LCARS_VIOLET, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 96, indCY, indR, LCARS_ICE, false);
        LcarsWidgets::drawLabel(spr, x + 104, indCY, "SENSORS",
                                LCARS_ICE, LCARS_FONT_SM, ML_DATUM);
    }
};

// ============================================================
// Engineering Screen — large gauge + chambers + cascade
// ============================================================

class EngineeringScreen : public LcarsScreen {
public:
    const char* title() const override { return "ENGINEERING"; }
    uint32_t refreshIntervalMs() const override { return 60; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Left side: large warp core gauge
        int16_t gaugeR = 28;
        int16_t gaugeCX = x + gaugeR + 4;
        int16_t gaugeCY = y + gaugeR + 2;
        int16_t thickness = 6;

        // Animated warp core output
        float warpPct = 0.6f + 0.3f * sinf((float)millis() / 800.0f);
        LcarsWidgets::drawDonutGauge(spr, gaugeCX, gaugeCY,
                                     gaugeR, thickness, warpPct,
                                     LCARS_GOLD, LCARS_BAR_TRACK);

        // Percentage in center of gauge
        char pctBuf[8];
        snprintf(pctBuf, sizeof(pctBuf), "%d", (int)(warpPct * 100));
        LcarsFont::drawText(spr, pctBuf, gaugeCX, gaugeCY - 4,
                            LCARS_FONT_MD, LCARS_GOLD, LCARS_BLACK, TC_DATUM);

        // Label under gauge
        LcarsWidgets::drawLabel(spr, gaugeCX, gaugeCY + gaugeR + 4,
                                "WARP CORE", _theme->textDim, LCARS_FONT_SM, TC_DATUM);

        // Right side: dilithium chamber status
        int16_t rx = x + gaugeR * 2 + 20;
        int16_t rw = w - (gaugeR * 2 + 20);
        int16_t ry = y;

        LcarsWidgets::drawLabel(spr, rx, ry, "DILITHIUM MATRIX",
                                _theme->accent, LCARS_FONT_SM);
        ry += 14;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 1", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, 0.95f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ry += 7;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 2", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, 0.88f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ry += 7;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 3", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        // Chamber 3 fluctuates — color changes with level
        float ch3 = 0.3f + 0.15f * sinf((float)millis() / 500.0f);
        uint16_t ch3Color = (ch3 < 0.4f) ? LCARS_TOMATO : LCARS_AMBER;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, ch3,
                                      ch3Color, LCARS_BAR_TRACK);
        ry += 7;

        // Blinking warning for chamber 3
        LcarsWidgets::drawIndicator(spr, rx + 4, ry + 5, 3, LCARS_TOMATO, true);
        LcarsWidgets::drawLabel(spr, rx + 12, ry + 1, "REALIGN",
                                LCARS_TOMATO, LCARS_FONT_SM);

        // Data cascade at bottom — full width
        int16_t cascadeY = y + gaugeR * 2 + 18;
        cascadeY += 4;
        LcarsWidgets::drawSeparator(spr, x, cascadeY, w, _theme->textDim);
        cascadeY += 5;

        int16_t cascadeH = c.y + c.h - cascadeY;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, x, cascadeY, w, cascadeH,
                                          LCARS_GOLD, 42);
        }
    }
};

// ============================================================
// Quarters Screen — dual sidebar, elbows on all 4 corners
// ============================================================

class QuartersScreen : public LcarsScreen {
public:
    const char* title() const override { return "QUARTERS"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 100; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;

        spr.fillSprite(_theme->background);

        // ── 4 corner elbows ──
        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R,
                              _theme->barTop, LCARS_ELBOW_TR);
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R,
                              _theme->barBottom, LCARS_ELBOW_BR);

        // ── Top bar (between elbows) ──
        int16_t barLX = SW + R + GAP;
        int16_t barRX = W - SW - R - GAP;
        int16_t barW = barRX - barLX;
        if (barW > 0) {
            LcarsFrame::drawBar(spr, barLX, 0, barW, TH,
                                _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_NONE);
        }

        // ── Bottom bar (between elbows) ──
        if (barW > 0) {
            LcarsFrame::drawBar(spr, barLX, H - BH, barW, BH,
                                _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_NONE);
        }

        // ── Left sidebar ──
        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        // ── Right sidebar (uses accent/different theme colors) ──
        uint16_t rSidebar[] = {
            _theme->accent, _theme->progressFg,
            _theme->gaugeColor, _theme->statusOk
        };
        if (sideH > 0) {
            LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                    rSidebar, 4, 2);
        }

        // ── Title on top bar ──
        LcarsFont::drawTextUpper(spr, "QUARTERS",
                                 barLX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        // ── Content area ──
        int16_t cx = SW + R + GAP + 1;
        int16_t cy = TH + 4;
        int16_t cw = W - 2 * (SW + R + GAP) - 2;
        int16_t y = cy;

        LcarsWidgets::drawLabel(spr, cx, y, "ENVIRONMENTAL", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "TEMPERATURE", "22.4 C",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.62f,
                                      LCARS_ICE, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "HUMIDITY", "45%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.45f,
                                      LCARS_VIOLET, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "LIGHTING", "70%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.70f,
                                      LCARS_GOLD, LCARS_BAR_TRACK);
        y += 7;

        y += 4;
        LcarsWidgets::drawSeparator(spr, cx, y, cw, _theme->textDim);
        y += 5;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "REPLICATOR", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "COMM PANEL", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
    }

    void onDrawTransition(TFT_eSprite& spr, float t) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;

        // Phase 1 (t = 0.0 – 0.2): All 4 elbows appear with power-on glow
        if (t > 0.0f) {
            float glowT = (t < 0.2f) ? (1.0f - t / 0.2f) : 0.0f;
            uint8_t glow = (uint8_t)(glowT * 90);

            uint16_t tlC = spr.alphaBlend(255 - glow, _theme->elbowTop, TFT_WHITE);
            uint16_t trC = spr.alphaBlend(255 - glow, _theme->barTop, TFT_WHITE);
            uint16_t blC = spr.alphaBlend(255 - glow, _theme->elbowBottom, TFT_WHITE);
            uint16_t brC = spr.alphaBlend(255 - glow, _theme->barBottom, TFT_WHITE);

            LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R, tlC, LCARS_ELBOW_TL);
            LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R, trC, LCARS_ELBOW_TR);
            LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R, blC, LCARS_ELBOW_BL);
            LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R, brC, LCARS_ELBOW_BR);
        }

        // Phase 2 (t = 0.2 – 0.6): Bars grow inward from both sides toward center
        if (t > 0.2f) {
            float barT = (t - 0.2f) / 0.4f;
            if (barT > 1.0f) barT = 1.0f;
            barT = LcarsEasing::easeOutCubic(barT);

            int16_t barLX = SW + R + GAP;
            int16_t barRX = W - SW - R - GAP;
            int16_t fullBarW = barRX - barLX;
            int16_t halfBar = fullBarW / 2;

            // Left half grows right from TL elbow
            int16_t leftEnd = barLX + (int16_t)(halfBar * barT);
            if (leftEnd > barLX) {
                LcarsFrame::drawBarPartial(spr, barLX, leftEnd, 0, TH,
                                           _theme->barTop, LCARS_CAP_NONE);
                LcarsFrame::drawBarPartial(spr, barLX, leftEnd, H - BH, BH,
                                           _theme->barBottom, LCARS_CAP_NONE);
            }
            // Right half grows left from TR elbow
            int16_t rightStart = barRX - (int16_t)(halfBar * barT);
            if (rightStart < barRX) {
                LcarsFrame::drawBarPartial(spr, rightStart, barRX, 0, TH,
                                           _theme->barTop, LCARS_CAP_NONE);
                LcarsFrame::drawBarPartial(spr, rightStart, barRX, H - BH, BH,
                                           _theme->barBottom, LCARS_CAP_NONE);
            }
        }

        // Phase 3 (t = 0.6 – 1.0): Both sidebars fill in segment by segment
        if (t > 0.6f) {
            float sideT = (t - 0.6f) / 0.4f;
            if (sideT > 1.0f) sideT = 1.0f;

            int16_t sideTop = TH + R + 2;
            int16_t sideH = H - TH - BH - 2 * R - 4;

            if (sideH > 0) {
                // Left sidebar
                if (_theme->sidebarCount > 0) {
                    int showSegs = (int)(sideT * _theme->sidebarCount) + 1;
                    if (showSegs > _theme->sidebarCount) showSegs = _theme->sidebarCount;
                    LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                            _theme->sidebar, showSegs, 2);
                }
                // Right sidebar
                uint16_t rSidebar[] = {
                    _theme->accent, _theme->progressFg,
                    _theme->gaugeColor, _theme->statusOk
                };
                int showR = (int)(sideT * 4) + 1;
                if (showR > 4) showR = 4;
                LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                        rSidebar, showR, 2);
            }
        }
    }
};

// ============================================================
// Split Panel Screen — mid-bar divider, upper/lower zones
// ============================================================

class SplitPanelScreen : public LcarsScreen {
public:
    const char* title() const override { return "SHIP MAP"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t MID_H = 12;  // Mid-bar height (thinner than top/bottom)

        spr.fillSprite(_theme->background);

        // ── Top-left elbow + top bar ──
        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        int16_t barX = SW + R + GAP;
        LcarsFrame::drawBar(spr, barX, 0, W - barX, TH,
                            _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_PILL);

        // ── Bottom-left elbow + bottom bar ──
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        LcarsFrame::drawBar(spr, barX, H - BH, W - barX, BH,
                            _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_PILL);

        // ── Mid-height divider bar ──
        int16_t midY = H / 2 - MID_H / 2;
        LcarsFrame::drawBar(spr, barX, midY, W - barX, MID_H,
                            _theme->accent, LCARS_CAP_NONE, LCARS_CAP_PILL);

        // ── Left sidebar (full height, continuous) ──
        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        // ── Title on top bar ──
        LcarsFont::drawTextUpper(spr, "SHIP MAP",
                                 barX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        // ── Upper content zone ──
        int16_t cx = barX + 1;
        int16_t cw = W - barX - 3;
        int16_t uy = TH + 4;

        LcarsWidgets::drawLabel(spr, cx, uy, "PRIMARY SYSTEMS", _theme->accent);
        uy += 14;

        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "WARP DRIVE", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        uy += 15;
        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "IMPULSE", "ACTIVE",
                                    _theme->statusOk, _theme->textDim);
        uy += 15;
        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "NAVIGATION", "NOMINAL",
                                    _theme->statusOk, _theme->textDim);

        // ── Lower content zone ──
        int16_t ly = midY + MID_H + 4;

        LcarsWidgets::drawLabel(spr, cx, ly, "SENSOR LOG", _theme->accent);
        ly += 14;

        int16_t cascadeH = (H - BH) - ly - 2;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, cx, ly, cw, cascadeH,
                                          _theme->accent, 77);
        }
    }

    void onDrawTransition(TFT_eSprite& spr, float t) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t MID_H = 12;
        const int16_t barX = SW + R + GAP;

        // Phase 1 (t = 0.0 – 0.2): Elbows appear with power-on glow
        if (t > 0.0f) {
            float glowT = (t < 0.2f) ? (1.0f - t / 0.2f) : 0.0f;
            uint8_t glow = (uint8_t)(glowT * 90);
            uint16_t topC = spr.alphaBlend(255 - glow, _theme->elbowTop, TFT_WHITE);
            uint16_t botC = spr.alphaBlend(255 - glow, _theme->elbowBottom, TFT_WHITE);

            LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R, topC, LCARS_ELBOW_TL);
            LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R, botC, LCARS_ELBOW_BL);
        }

        // Phase 2 (t = 0.2 – 0.5): Top + bottom bars extend
        if (t > 0.2f) {
            float barT = (t - 0.2f) / 0.3f;
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

        // Phase 2b (t = 0.5 – 0.7): Mid-bar extends (starts AFTER main bars)
        if (t > 0.5f) {
            float midT = (t - 0.5f) / 0.2f;
            if (midT > 1.0f) midT = 1.0f;
            midT = LcarsEasing::easeOutCubic(midT);

            int16_t midY = H / 2 - MID_H / 2;
            int16_t midEnd = barX + (int16_t)((W - barX) * midT);
            if (midEnd > barX) {
                LcarsFrame::drawBarPartial(spr, barX, midEnd, midY, MID_H,
                                           _theme->accent, LCARS_CAP_PILL);
            }
        }

        // Phase 3 (t = 0.7 – 1.0): Sidebar segments fill in
        if (t > 0.7f) {
            float sideT = (t - 0.7f) / 0.3f;
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
    }
};

// ============================================================
// Medical Screen — split dual frames side by side
// ============================================================

class MedicalScreen : public LcarsScreen {
public:
    const char* title() const override { return "SICKBAY"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t SPLIT_GAP = 6;  // Gap between left and right frames

        spr.fillSprite(_theme->background);

        // Calculate split point
        int16_t splitX = W / 2;
        int16_t leftW = splitX - SPLIT_GAP / 2;
        int16_t rightX = splitX + SPLIT_GAP / 2;
        int16_t rightW = W - rightX;

        // ═══════════ LEFT FRAME ═══════════

        // Left TL elbow
        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        // Left top bar (pill cap on right end)
        int16_t lBarX = SW + R + GAP;
        int16_t lBarW = leftW - lBarX;
        if (lBarW > 0) {
            LcarsFrame::drawBar(spr, lBarX, 0, lBarW, TH,
                                _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_PILL);
        }

        // Left BL elbow
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        // Left bottom bar
        if (lBarW > 0) {
            LcarsFrame::drawBar(spr, lBarX, H - BH, lBarW, BH,
                                _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_PILL);
        }

        // Left sidebar
        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        // Left title
        LcarsFont::drawTextUpper(spr, "VITALS",
                                 lBarX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        // ═══════════ RIGHT FRAME ═══════════

        // Right TR elbow
        LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R,
                              _theme->accent, LCARS_ELBOW_TR);
        // Right top bar (pill cap on left end)
        int16_t rBarX = rightX;
        int16_t rBarW = W - SW - R - GAP - rBarX;
        if (rBarW > 0) {
            LcarsFrame::drawBar(spr, rBarX, 0, rBarW, TH,
                                _theme->accent, LCARS_CAP_PILL, LCARS_CAP_NONE);
        }

        // Right BR elbow
        LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R,
                              _theme->gaugeColor, LCARS_ELBOW_BR);
        // Right bottom bar
        if (rBarW > 0) {
            LcarsFrame::drawBar(spr, rBarX, H - BH, rBarW, BH,
                                _theme->gaugeColor, LCARS_CAP_PILL, LCARS_CAP_NONE);
        }

        // Right sidebar
        uint16_t rSidebar[] = {
            _theme->accent, _theme->gaugeColor,
            _theme->progressFg, _theme->statusOk
        };
        if (sideH > 0) {
            LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                    rSidebar, 4, 2);
        }

        // Right title
        if (rBarW > 0) {
            LcarsFont::drawTextUpper(spr, "MEDICAL",
                                     rBarX + TH / 2 + 4, TH / 2,
                                     LCARS_FONT_SM, _theme->textOnBar,
                                     _theme->accent, ML_DATUM);
        }

        // ═══════════ LEFT CONTENT (Vitals) ═══════════
        int16_t lcx = lBarX + 1;
        int16_t lcw = lBarW - 3;
        int16_t ly = TH + 4;

        // Heart rate — animated
        char hrBuf[8];
        int hr = 72 + (millis() / 400) % 8;
        snprintf(hrBuf, sizeof(hrBuf), "%d", hr);
        LcarsWidgets::drawValueLabel(spr, lcx, ly, hrBuf, "HEART BPM",
                                     LCARS_GREEN, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);
        ly += 46;

        LcarsWidgets::drawStatusRow(spr, lcx, ly, lcw, "O2 SAT", "98%",
                                    _theme->statusOk, _theme->textDim);
        ly += 18;
        LcarsWidgets::drawProgressBar(spr, lcx, ly, lcw, 5, 0.98f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ly += 7;

        LcarsWidgets::drawStatusRow(spr, lcx, ly, lcw, "BP", "120/80",
                                    _theme->statusOk, _theme->textDim);
        ly += 18;
        LcarsWidgets::drawProgressBar(spr, lcx, ly, lcw, 5, 0.75f,
                                      LCARS_ICE, LCARS_BAR_TRACK);

        // ═══════════ RIGHT CONTENT (Medical) ═══════════
        int16_t rcx = rBarX + TH / 2 + 1;
        int16_t rcw = rBarW - TH / 2 - 3;
        int16_t ry = TH + 4;

        LcarsWidgets::drawLabel(spr, rcx, ry, "BIOBED 1", _theme->accent);
        ry += 14;

        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "NEURAL", "NORMAL",
                                    _theme->statusOk, _theme->textDim);
        ry += 15;
        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "CORTISOL", "LOW",
                                    _theme->statusWarn, _theme->textDim);
        ry += 15;
        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "INAPROVALINE", "2CC",
                                    _theme->statusOk, _theme->textDim);
        ry += 17;

        LcarsWidgets::drawSeparator(spr, rcx, ry, rcw, _theme->textDim);
        ry += 5;

        int16_t cascadeH = (H - BH) - ry - 2;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, rcx, ry, rcw, cascadeH,
                                          _theme->accent, 33);
        }
    }

    void onDrawTransition(TFT_eSprite& spr, float t) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t SPLIT_GAP = 6;

        int16_t splitX = W / 2;
        int16_t leftW = splitX - SPLIT_GAP / 2;
        int16_t rightX = splitX + SPLIT_GAP / 2;
        int16_t lBarX = SW + R + GAP;
        int16_t lBarW = leftW - lBarX;
        int16_t rBarW = W - SW - R - GAP - rightX;
        int16_t sideTop = TH + R + 2;
        int16_t sideH = H - TH - BH - 2 * R - 4;

        // ═══ LEFT FRAME (t = 0.0 – 0.5) ═══

        // Phase 1 (t = 0.0 – 0.1): Left elbows appear with power-on glow
        if (t > 0.0f) {
            float glowT = (t < 0.1f) ? (1.0f - t / 0.1f) : 0.0f;
            uint8_t glow = (uint8_t)(glowT * 90);
            uint16_t topC = spr.alphaBlend(255 - glow, _theme->elbowTop, TFT_WHITE);
            uint16_t botC = spr.alphaBlend(255 - glow, _theme->elbowBottom, TFT_WHITE);

            LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R, topC, LCARS_ELBOW_TL);
            LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R, botC, LCARS_ELBOW_BL);
        }

        // Phase 2 (t = 0.1 – 0.3): Left bars extend
        if (t > 0.1f) {
            float lBarT = (t - 0.1f) / 0.2f;
            if (lBarT > 1.0f) lBarT = 1.0f;
            lBarT = LcarsEasing::easeOutCubic(lBarT);

            if (lBarW > 0) {
                int16_t lEnd = lBarX + (int16_t)(lBarW * lBarT);
                LcarsFrame::drawBarPartial(spr, lBarX, lEnd, 0, TH,
                                           _theme->barTop, LCARS_CAP_PILL);
                LcarsFrame::drawBarPartial(spr, lBarX, lEnd, H - BH, BH,
                                           _theme->barBottom, LCARS_CAP_PILL);
            }
        }

        // Phase 3 (t = 0.3 – 0.5): Left sidebar fills
        if (t > 0.3f) {
            float lSideT = (t - 0.3f) / 0.2f;
            if (lSideT > 1.0f) lSideT = 1.0f;

            if (sideH > 0 && _theme->sidebarCount > 0) {
                int showSegs = (int)(lSideT * _theme->sidebarCount) + 1;
                if (showSegs > _theme->sidebarCount) showSegs = _theme->sidebarCount;
                LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                        _theme->sidebar, showSegs, 2);
            }
        }

        // ═══ RIGHT FRAME (t = 0.5 – 1.0) ═══

        // Phase 4 (t = 0.5 – 0.6): Right elbows appear with power-on glow
        if (t > 0.5f) {
            float rGlowT = (t < 0.6f) ? (1.0f - (t - 0.5f) / 0.1f) : 0.0f;
            uint8_t rGlow = (uint8_t)(rGlowT * 90);
            uint16_t trC = spr.alphaBlend(255 - rGlow, _theme->accent, TFT_WHITE);
            uint16_t brC = spr.alphaBlend(255 - rGlow, _theme->gaugeColor, TFT_WHITE);

            LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R, trC, LCARS_ELBOW_TR);
            LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R, brC, LCARS_ELBOW_BR);
        }

        // Phase 5 (t = 0.6 – 0.8): Right bars extend (grows leftward)
        if (t > 0.6f) {
            float rBarT = (t - 0.6f) / 0.2f;
            if (rBarT > 1.0f) rBarT = 1.0f;
            rBarT = LcarsEasing::easeOutCubic(rBarT);

            if (rBarW > 0) {
                int16_t rStart = (rightX + rBarW) - (int16_t)(rBarW * rBarT);
                int16_t rEnd = rightX + rBarW;
                LcarsFrame::drawBarPartial(spr, rStart, rEnd, 0, TH,
                                           _theme->accent, LCARS_CAP_PILL);
                LcarsFrame::drawBarPartial(spr, rStart, rEnd, H - BH, BH,
                                           _theme->gaugeColor, LCARS_CAP_PILL);
            }
        }

        // Phase 6 (t = 0.8 – 1.0): Right sidebar fills
        if (t > 0.8f) {
            float rSideT = (t - 0.8f) / 0.2f;
            if (rSideT > 1.0f) rSideT = 1.0f;

            if (sideH > 0) {
                uint16_t rSidebar[] = {
                    _theme->accent, _theme->gaugeColor,
                    _theme->progressFg, _theme->statusOk
                };
                int showR = (int)(rSideT * 4) + 1;
                if (showR > 4) showR = 4;
                LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                        rSidebar, showR, 2);
            }
        }
    }
};

// ============================================================
// Global screen instances
// ============================================================

LcarsBootScreen   bootScreen;
DashboardScreen   dashScreen;
StatusScreen      statusScreen;
TacticalScreen    tacticalScreen;
EngineeringScreen engineeringScreen;
QuartersScreen    quartersScreen;
SplitPanelScreen  splitPanelScreen;
MedicalScreen     medicalScreen;

LcarsScreen* screens[] = {
    &dashScreen, &statusScreen, &tacticalScreen, &engineeringScreen,
    &quartersScreen, &splitPanelScreen, &medicalScreen
};
const uint8_t SCREEN_COUNT = 7;
uint32_t screenSwitchMs = 0;
uint8_t  currentScreenIdx = 0;

// Theme cycling — switch theme after showing all screens
LcarsThemeId themes[] = { LCARS_THEME_TNG, LCARS_THEME_NEMESIS, LCARS_THEME_LOWER_DECKS, LCARS_THEME_RED_ALERT };
const uint8_t THEME_COUNT = 4;
uint8_t  currentThemeIdx = 0;

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[LCARS-ESP32] Hello LCARS Demo");

    // Initialize engine
    engine.begin(tft);
    engine.setTheme(LCARS_THEME_TNG);

    // Set backlight pin
    #if defined(TFT_BL)
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
        engine.setBLPin(TFT_BL);
    #endif

    // Start with boot sequence
    bootScreen.setNextScreen(&dashScreen);
    engine.setScreen(&bootScreen);

    Serial.println("[LCARS-ESP32] Boot sequence started");
}

// ============================================================
// Loop
// ============================================================

void loop() {
    engine.update();

    // Check if boot sequence is done
    if (bootScreen.isComplete() && engine.currentScreen() == &bootScreen) {
        engine.setScreen(bootScreen.nextScreen());
        screenSwitchMs = millis();
        Serial.println("[LCARS-ESP32] Boot complete, switching to dashboard");
    }

    // Auto-cycle through all screens every 8 seconds
    if (engine.currentScreen() != &bootScreen &&
        !engine.isTransitioning() &&
        millis() - screenSwitchMs > 8000) {

        currentScreenIdx = (currentScreenIdx + 1) % SCREEN_COUNT;

        // Switch theme after cycling through all screens
        if (currentScreenIdx == 0) {
            currentThemeIdx = (currentThemeIdx + 1) % THEME_COUNT;
            engine.setTheme(themes[currentThemeIdx]);
        }

        engine.setScreen(screens[currentScreenIdx]);
        screenSwitchMs = millis();
    }
}
