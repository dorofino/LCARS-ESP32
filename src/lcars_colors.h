#pragma once

#include <stdint.h>

// ============================================================
// LCARS Color Palette — RGB565 for TFT_eSPI
// Based on Michael Okuda's TNG reference palette
// Source: thelcars.com/colors.php
//
// RGB565 macro: 5 bits red, 6 bits green, 5 bits blue
// ============================================================

#define LCARS_RGB565(r, g, b) \
    (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// ── Background ──────────────────────────────────────────────
#define LCARS_BLACK         0x0000     // Always black background

// ── TNG Classic Palette ─────────────────────────────────────
// Warm structural colors (bars, elbows, sidebars)
#define LCARS_GOLD          LCARS_RGB565(0xFF, 0xAA, 0x00)  // #FFAA00 — primary gold
#define LCARS_AMBER         LCARS_RGB565(0xFF, 0x99, 0x00)  // #FF9900 — golden orange
#define LCARS_ORANGE        LCARS_RGB565(0xFF, 0x88, 0x00)  // #FF8800 — orange accent
#define LCARS_BUTTERSCOTCH  LCARS_RGB565(0xFF, 0x99, 0x66)  // #FF9966 — warm bars
#define LCARS_SUNFLOWER     LCARS_RGB565(0xFF, 0xCC, 0x99)  // #FFCC99 — light panels
#define LCARS_PEACH         LCARS_RGB565(0xFF, 0x88, 0x66)  // #FF8866 — peach
#define LCARS_TAN           LCARS_RGB565(0xCC, 0x99, 0x66)  // #CC9966 — tan

// Cool data colors
#define LCARS_ICE           LCARS_RGB565(0x99, 0xCC, 0xFF)  // #99CCFF — sky blue
#define LCARS_BLUE          LCARS_RGB565(0x55, 0x66, 0xFF)  // #5566FF — blue accent
#define LCARS_PERIWINKLE    LCARS_RGB565(0x99, 0x99, 0xFF)  // #9999FF — blue panels

// Purple accents
#define LCARS_VIOLET        LCARS_RGB565(0xCC, 0x99, 0xFF)  // #CC99FF — african violet
#define LCARS_LILAC         LCARS_RGB565(0xCC, 0x55, 0xFF)  // #CC55FF — lilac
#define LCARS_LAVENDER      LCARS_RGB565(0xCC, 0x99, 0xCC)  // #CC99CC — lavender

// Alert/status colors
#define LCARS_TOMATO        LCARS_RGB565(0xFF, 0x55, 0x55)  // #FF5555 — alert
#define LCARS_MARS          LCARS_RGB565(0xFF, 0x22, 0x00)  // #FF2200 — critical
#define LCARS_GREEN         LCARS_RGB565(0x00, 0xFF, 0x88)  // #00FF88 — ok/success

// Text
#define LCARS_WHITE         LCARS_RGB565(0xF5, 0xF6, 0xFA)  // #F5F6FA — space white
#define LCARS_TEXT_DIM      LCARS_RGB565(0x88, 0x88, 0x88)  // #888888 — secondary text

// ── Nemesis Blue Palette ────────────────────────────────────
#define LCARS_NEM_COOL      LCARS_RGB565(0x66, 0x99, 0xFF)  // #6699FF
#define LCARS_NEM_EVENING   LCARS_RGB565(0x22, 0x66, 0xFF)  // #2266FF
#define LCARS_NEM_MIDNIGHT  LCARS_RGB565(0x22, 0x33, 0xFF)  // #2233FF
#define LCARS_NEM_GHOST     LCARS_RGB565(0x88, 0xBB, 0xFF)  // #88BBFF
#define LCARS_NEM_GRAPE     LCARS_RGB565(0x99, 0x66, 0xCC)  // #9966CC
#define LCARS_NEM_HONEY     LCARS_RGB565(0xFF, 0xCC, 0x99)  // #FFCC99
#define LCARS_NEM_TANGERINE LCARS_RGB565(0xFF, 0x88, 0x33)  // #FF8833
#define LCARS_NEM_CARDINAL  LCARS_RGB565(0xCC, 0x22, 0x33)  // #CC2233

// ── Lower Decks Palette ───────────────────────────────────
// Brighter, more saturated — vivid purple/magenta/cyan
#define LCARS_LD_PURPLE     LCARS_RGB565(0xBB, 0x44, 0xFF)  // #BB44FF — vivid purple
#define LCARS_LD_MAGENTA    LCARS_RGB565(0xFF, 0x44, 0xCC)  // #FF44CC — hot pink
#define LCARS_LD_CYAN       LCARS_RGB565(0x44, 0xDD, 0xFF)  // #44DDFF — bright cyan
#define LCARS_LD_CORAL      LCARS_RGB565(0xFF, 0x66, 0x88)  // #FF6688 — coral pink
#define LCARS_LD_LIME       LCARS_RGB565(0x66, 0xFF, 0x66)  // #66FF66 — bright green
#define LCARS_LD_TANGERINE  LCARS_RGB565(0xFF, 0x88, 0x22)  // #FF8822 — vivid orange
#define LCARS_LD_SKY        LCARS_RGB565(0x66, 0xBB, 0xFF)  // #66BBFF — sky blue
#define LCARS_LD_LAVENDER   LCARS_RGB565(0xDD, 0x88, 0xFF)  // #DD88FF — bright lavender

// ── UI Constants ────────────────────────────────────────────
#define LCARS_BAR_TRACK     LCARS_RGB565(0x1A, 0x1A, 0x1A)  // #1A1A1A — unfilled bar
