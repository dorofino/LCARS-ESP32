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
// Global screen instances
// ============================================================

LcarsBootScreen   bootScreen;
DashboardScreen   dashScreen;
StatusScreen      statusScreen;
TacticalScreen    tacticalScreen;
EngineeringScreen engineeringScreen;

LcarsScreen* screens[] = { &dashScreen, &statusScreen, &tacticalScreen, &engineeringScreen };
const uint8_t SCREEN_COUNT = 4;
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
