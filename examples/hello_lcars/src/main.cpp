// ============================================================
// Hello LCARS — Demo for lcars-esp32 engine
//
// Shows:
//   1. Animated boot sequence (diagnostic text + frame assembly)
//   2. Dashboard screen with widgets
//   3. Status screen (auto-cycles between them)
// ============================================================

#include <lcars.h>

TFT_eSPI tft;
LcarsEngine engine;

// ============================================================
// Dashboard Screen — widgets demo
// ============================================================

class DashboardScreen : public LcarsScreen {
public:
    const char* title() const override { return "MAIN SYSTEMS"; }
    uint32_t refreshIntervalMs() const override { return 100; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Section: Power Systems
        LcarsWidgets::drawLabel(spr, x, y, "POWER ALLOCATION", _theme->accent);
        y += 16;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "WARP CORE", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 14;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 8, 0.78f,
                                      LCARS_ICE, LCARS_BAR_TRACK);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "SHIELDS", "87%",
                                    _theme->statusOk, _theme->textDim);
        y += 14;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 8, 0.87f,
                                      LCARS_VIOLET, LCARS_BAR_TRACK);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "WEAPONS", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 14;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 8, 0.45f,
                                      LCARS_GOLD, LCARS_BAR_TRACK);
        y += 18;

        // Separator
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 8;

        // Donut gauges side by side
        int16_t gaugeR = 20;
        int16_t gaugeSpacing = w / 3;

        LcarsWidgets::drawDonutGauge(spr, x + gaugeSpacing / 2, y + gaugeR + 2,
                                     gaugeR, 5, 0.78f, LCARS_ICE, LCARS_BAR_TRACK);
        LcarsWidgets::drawLabel(spr, x + gaugeSpacing / 2, y + gaugeR * 2 + 8,
                                "EPS", _theme->textDim, LCARS_FONT_SM, TC_DATUM);

        LcarsWidgets::drawDonutGauge(spr, x + gaugeSpacing + gaugeSpacing / 2, y + gaugeR + 2,
                                     gaugeR, 5, 0.92f, LCARS_GREEN, LCARS_BAR_TRACK);
        LcarsWidgets::drawLabel(spr, x + gaugeSpacing + gaugeSpacing / 2, y + gaugeR * 2 + 8,
                                "SIF", _theme->textDim, LCARS_FONT_SM, TC_DATUM);

        LcarsWidgets::drawDonutGauge(spr, x + 2 * gaugeSpacing + gaugeSpacing / 2, y + gaugeR + 2,
                                     gaugeR, 5, 0.34f, LCARS_TOMATO, LCARS_BAR_TRACK);
        LcarsWidgets::drawLabel(spr, x + 2 * gaugeSpacing + gaugeSpacing / 2, y + gaugeR * 2 + 8,
                                "DEF", _theme->textDim, LCARS_FONT_SM, TC_DATUM);
    }
};

// ============================================================
// Status Screen — data cascade demo
// ============================================================

class StatusScreen : public LcarsScreen {
public:
    const char* title() const override { return "SYSTEM STATUS"; }
    uint32_t refreshIntervalMs() const override { return 50; }

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
        y += 16;

        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 6;

        // Status rows
        LcarsWidgets::drawStatusRow(spr, x, y, w, "HULL INTEGRITY", "98.7%",
                                    _theme->statusOk, _theme->textDim);
        y += 14;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "LIFE SUPPORT", "NOMINAL",
                                    _theme->statusOk, _theme->textDim);
        y += 14;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "INERTIAL DAMPERS", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 14;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "DEFLECTOR ARRAY", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 14;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "COMM ARRAY", "ACTIVE",
                                    _theme->statusOk, _theme->textDim);
        y += 18;

        // Data cascade at bottom
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 4;
        int16_t cascadeH = c.y + c.h - y;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, x, y, w, cascadeH, _theme->accent);
        }
    }
};

// ============================================================
// Global screen instances
// ============================================================

LcarsBootScreen bootScreen;
DashboardScreen dashScreen;
StatusScreen    statusScreen;

uint32_t screenSwitchMs = 0;
bool showingDash = true;

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

    // Auto-cycle between dashboard and status every 8 seconds
    if (engine.currentScreen() != &bootScreen &&
        !engine.isTransitioning() &&
        millis() - screenSwitchMs > 8000) {

        if (showingDash) {
            engine.setScreen(&statusScreen);
        } else {
            engine.setScreen(&dashScreen);
        }
        showingDash = !showingDash;
        screenSwitchMs = millis();
    }
}
