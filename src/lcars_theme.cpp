#include "lcars_theme.h"

// ============================================================
// LCARS Theme Definitions
// ============================================================

static const LcarsTheme _themeTNG = {
    .name = "TNG CLASSIC",
    .elbowTop      = LCARS_BUTTERSCOTCH,
    .elbowBottom   = LCARS_LAVENDER,
    .barTop        = LCARS_AMBER,
    .barBottom     = LCARS_TAN,
    .sidebar       = { LCARS_PEACH, LCARS_LAVENDER, LCARS_TOMATO, LCARS_VIOLET },
    .sidebarCount  = 4,
    .elbowTopRight     = LCARS_SUNFLOWER,
    .elbowBottomRight  = LCARS_VIOLET,
    .sidebarRight      = { LCARS_SUNFLOWER, LCARS_AMBER, LCARS_PEACH, LCARS_LAVENDER },
    .sidebarRightCount = 4,
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
    .elbowTopRight     = LCARS_NEM_GHOST,
    .elbowBottomRight  = LCARS_NEM_EVENING,
    .sidebarRight      = { LCARS_NEM_GHOST, LCARS_NEM_COOL, LCARS_NEM_EVENING, LCARS_NEM_MIDNIGHT },
    .sidebarRightCount = 4,
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
    .elbowTopRight     = LCARS_TOMATO,
    .elbowBottomRight  = LCARS_MARS,
    .sidebarRight      = { LCARS_MARS, LCARS_TOMATO, LCARS_MARS, LCARS_TOMATO },
    .sidebarRightCount = 4,
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

static const LcarsTheme _themeLowerDecks = {
    .name = "LOWER DECKS",
    .elbowTop      = LCARS_LD_PURPLE,
    .elbowBottom   = LCARS_LD_MAGENTA,
    .barTop        = LCARS_LD_PURPLE,
    .barBottom     = LCARS_LD_CORAL,
    .sidebar       = { LCARS_LD_CYAN, LCARS_LD_MAGENTA, LCARS_LD_LAVENDER, LCARS_LD_TANGERINE },
    .sidebarCount  = 4,
    .elbowTopRight     = LCARS_LD_CYAN,
    .elbowBottomRight  = LCARS_LD_CORAL,
    .sidebarRight      = { LCARS_LD_SKY, LCARS_LD_PURPLE, LCARS_LD_MAGENTA, LCARS_LD_LIME },
    .sidebarRightCount = 4,
    .text          = LCARS_WHITE,
    .textDim       = LCARS_LD_LAVENDER,
    .textOnBar     = LCARS_BLACK,
    .accent        = LCARS_LD_CYAN,
    .progressFg    = LCARS_LD_CYAN,
    .progressBg    = LCARS_BAR_TRACK,
    .gaugeColor    = LCARS_LD_CYAN,
    .statusOk      = LCARS_LD_LIME,
    .statusWarn    = LCARS_LD_TANGERINE,
    .statusErr     = LCARS_LD_CORAL,
    .alert         = LCARS_LD_MAGENTA,
    .background    = LCARS_BLACK,
};

static const LcarsTheme* _themes[LCARS_THEME_COUNT] = {
    &_themeTNG,
    &_themeNemesis,
    &_themeRedAlert,
    &_themeLowerDecks,
};

const LcarsTheme& LcarsThemes::get(LcarsThemeId id) {
    if (id >= LCARS_THEME_COUNT) return _themeTNG;
    return *_themes[id];
}

const LcarsTheme& LcarsThemes::tng()        { return _themeTNG; }
const LcarsTheme& LcarsThemes::nemesis()     { return _themeNemesis; }
const LcarsTheme& LcarsThemes::redAlert()    { return _themeRedAlert; }
const LcarsTheme& LcarsThemes::lowerDecks()  { return _themeLowerDecks; }
