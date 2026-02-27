#include "lcars_theme.h"

// ============================================================
// LCARS Theme Definitions
// ============================================================

static const LcarsTheme _themeTNG = {
    .name = "TNG CLASSIC",
    .elbowTop      = LCARS_AMBER,
    .elbowBottom   = LCARS_TAN,
    .barTop        = LCARS_AMBER,
    .barBottom     = LCARS_TAN,
    .sidebar       = { LCARS_BUTTERSCOTCH, LCARS_LAVENDER, LCARS_PERIWINKLE, LCARS_VIOLET },
    .sidebarCount  = 4,
    .text          = LCARS_SUNFLOWER,
    .textDim       = LCARS_TEXT_DIM,
    .textOnBar     = LCARS_BLACK,
    .accent        = LCARS_GOLD,
    .progressFg    = LCARS_ICE,
    .progressBg    = LCARS_BAR_TRACK,
    .gaugeColor    = LCARS_ICE,
    .statusOk      = LCARS_ICE,
    .statusWarn    = LCARS_GOLD,
    .statusErr     = LCARS_TOMATO,
    .alert         = LCARS_MARS,
    .background    = LCARS_BLACK,
};

static const LcarsTheme _themeNemesis = {
    .name = "NEMESIS",
    .elbowTop      = LCARS_NEM_COOL,
    .elbowBottom   = LCARS_NEM_GRAPE,
    .barTop        = LCARS_NEM_COOL,
    .barBottom     = LCARS_NEM_GRAPE,
    .sidebar       = { LCARS_NEM_GHOST, LCARS_NEM_EVENING, LCARS_NEM_MIDNIGHT, LCARS_NEM_GRAPE },
    .sidebarCount  = 4,
    .text          = LCARS_NEM_GHOST,
    .textDim       = LCARS_TEXT_DIM,
    .textOnBar     = LCARS_BLACK,
    .accent        = LCARS_NEM_COOL,
    .progressFg    = LCARS_NEM_COOL,
    .progressBg    = LCARS_BAR_TRACK,
    .gaugeColor    = LCARS_NEM_COOL,
    .statusOk      = LCARS_NEM_COOL,
    .statusWarn    = LCARS_NEM_HONEY,
    .statusErr     = LCARS_NEM_CARDINAL,
    .alert         = LCARS_NEM_CARDINAL,
    .background    = LCARS_BLACK,
};

static const LcarsTheme _themeRedAlert = {
    .name = "RED ALERT",
    .elbowTop      = LCARS_MARS,
    .elbowBottom   = LCARS_TOMATO,
    .barTop        = LCARS_MARS,
    .barBottom     = LCARS_TOMATO,
    .sidebar       = { LCARS_TOMATO, LCARS_MARS, LCARS_TOMATO, LCARS_MARS },
    .sidebarCount  = 4,
    .text          = LCARS_WHITE,
    .textDim       = LCARS_TOMATO,
    .textOnBar     = LCARS_WHITE,
    .accent        = LCARS_WHITE,
    .progressFg    = LCARS_TOMATO,
    .progressBg    = LCARS_BAR_TRACK,
    .gaugeColor    = LCARS_TOMATO,
    .statusOk      = LCARS_GREEN,
    .statusWarn    = LCARS_GOLD,
    .statusErr     = LCARS_WHITE,
    .alert         = LCARS_MARS,
    .background    = LCARS_BLACK,
};

static const LcarsTheme* _themes[LCARS_THEME_COUNT] = {
    &_themeTNG,
    &_themeNemesis,
    &_themeRedAlert,
};

const LcarsTheme& LcarsThemes::get(LcarsThemeId id) {
    if (id >= LCARS_THEME_COUNT) return _themeTNG;
    return *_themes[id];
}

const LcarsTheme& LcarsThemes::tng()      { return _themeTNG; }
const LcarsTheme& LcarsThemes::nemesis()   { return _themeNemesis; }
const LcarsTheme& LcarsThemes::redAlert()  { return _themeRedAlert; }
