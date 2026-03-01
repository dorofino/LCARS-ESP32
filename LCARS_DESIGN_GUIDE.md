# LCARS Design Guide for Embedded Displays

A comprehensive reference for building authentic Star Trek LCARS interfaces on ESP32-S3 microcontrollers with TFT displays. This document serves two purposes:

1. **Implementation reference** — for anyone extending the lcars-esp32 engine
2. **Design guide** — a standalone resource for implementing LCARS-style interfaces on embedded hardware

---

## Table of Contents

- [1. LCARS: Origin and Canon](#1-lcars-origin-and-canon)
- [2. Geometric Vocabulary](#2-geometric-vocabulary)
- [3. Color System](#3-color-system)
- [4. Theme System](#4-theme-system)
- [5. Typography](#5-typography)
- [6. Rendering Architecture](#6-rendering-architecture)
- [7. Layout System](#7-layout-system)
- [8. Animation System](#8-animation-system)
- [9. Audio Design](#9-audio-design)
- [10. Widget Catalog](#10-widget-catalog)
- [11. Screen System](#11-screen-system)
- [Appendix A: Quick Reference](#appendix-a-quick-reference)
- [Appendix B: External References](#appendix-b-external-references)

---

## 1. LCARS: Origin and Canon

### What Is LCARS?

LCARS — **Library Computer Access/Retrieval System** — is the fictional computer interface used aboard Federation starships in the Star Trek universe. It was designed by **Michael Okuda**, the scenic art supervisor for Star Trek: The Next Generation (1987), and has since appeared in every Star Trek series and film set in the TNG era and beyond.

### Why LCARS Looks the Way It Does

LCARS was born from a production constraint: building practical, functional control panels for every set was prohibitively expensive. Okuda's solution was elegant — **backlit colored panels with printed graphics** that could be swapped out cheaply between episodes. This led directly to the flat, geometric aesthetic:

- **No depth or shadows** — everything is flat-printed on translucent material
- **Large colored blocks** — readable on camera from a distance
- **Rounded "pill" shapes** — softer and more futuristic than sharp rectangles
- **Black background** — the backlit panels sit against dark set walls (and space)
- **Uppercase text** — legibility at a glance on camera

These production-driven decisions created one of the most recognizable UI designs in science fiction.

### Core Visual Principles

These are the rules that make something look authentically LCARS:

| Principle | Description |
|-----------|-------------|
| **Black background** | Always. LCARS panels float on black (space). |
| **Warm structural colors** | Golds, ambers, oranges, peaches for frame elements (elbows, bars, sidebars). |
| **Cool data colors** | Blues and ice tones for data readouts, progress indicators, and status values. |
| **Purple accents** | Violets and lavenders as secondary structural and accent colors. |
| **Asymmetric layout** | LCARS panels are never symmetrically balanced. The sidebar is always on one side. |
| **Rounded terminations** | Every bar ends in a semicircle ("pill cap"). No sharp exposed edges. |
| **Uppercase text** | All LCARS text is displayed in uppercase. Always. |
| **L-shaped elbows** | The signature LCARS element — curved junctions between horizontal bars and vertical sidebars. |
| **Segmented sidebars** | Vertical bars are divided into colored blocks separated by thin gaps. |

### Evolution Across Series

| Series | Era | Palette | Characteristics |
|--------|-----|---------|-----------------|
| **TNG** (1987-1994) | Canonical | Warm golds, ambers, violets, peach | The definitive LCARS. Asymmetric frames, segmented sidebars, pill-capped bars. |
| **DS9** (1993-1999) | Federation variant | Similar to TNG, slightly more muted | Cardassian station overlays blended with Federation LCARS. |
| **Voyager** (1995-2001) | Intrepid-class | Brighter blues and greens added | More data-dense layouts, additional cool tones. |
| **Nemesis** (2002) | Sovereign-class refit | Cool blues, midnight, grape | Darker, more military aesthetic. Shift from warm to cool palette. |
| **Lower Decks** (2020-2024) | California-class | Vivid purples, magentas, cyans | Animated series — brighter, more saturated, playful energy. |

---

## 2. Geometric Vocabulary

LCARS is built from a small set of geometric primitives. Understanding these is essential.

### 2.1 Elbows

The **elbow** is the signature LCARS element — an L-shaped junction that connects a horizontal bar to a vertical sidebar with a smooth inner curve. It comes in four orientations:

```
Top-Left (TL)                    Top-Right (TR)

 ┌──────────────────╮             ╭──────────────────┐
 │████████████████████             ████████████████████│
 │████████████████████             ████████████████████│
 │██████╭─────                         ─────╮██████│
 │██████│                                   │██████│
 │██████│                                   │██████│
 sidebar                                     sidebar

Bottom-Left (BL)                 Bottom-Right (BR)

 │██████│                                   │██████│
 │██████│                                   │██████│
 │██████╰─────                         ─────╯██████│
 │████████████████████             ████████████████████│
 │████████████████████             ████████████████████│
 └──────────────────╯             ╰──────────────────┘
```

**Elbow anatomy:**
- **Bar portion** — the horizontal band (width = sidebarW + innerR)
- **Sidebar portion** — the vertical band (height = barH + innerR)
- **Inner curve** — a quarter-circle with radius `innerR` (default 16px)
- **Junction point** — where the bar's inner edge meets the sidebar's inner edge

#### The Additive Drawing Approach

Drawing smooth, gap-free elbows on embedded displays requires a specific technique. There are two approaches:

**Subtractive (WRONG):** Fill the entire elbow area, then cut out a quarter-circle for the inner curve. This fails because TFT_eSPI's anti-aliased functions blend edge pixels against a background color. When you "cut" into an already-drawn solid area, the AA edge pixels blend against the wrong color, creating visible artifacts — dark dots and gaps at the arc center.

**Additive (CORRECT):** Draw a full anti-aliased circle at the junction point, then paint solid rectangles on top to cover three of the four quadrants. The exposed quarter has perfect AA edges because the circle was drawn against the true black background.

```
Step 1: Full AA circle      Step 2: Cover 3 quadrants    Result: Clean elbow
at junction point           with bar + sidebar rects

      ╭──────╮              ████████████████              ████████████████
     ╱        ╲             ████████████████              ████████████████
    │    ()    │            ████████╭─────               ████████╭─────
     ╲        ╱             ████████│                     ████████│
      ╰──────╯              ████████│                     ████████│
```

Implementation (TL case from `lcars_frame.cpp`):

```cpp
int16_t cx = x + sideW, cy = y + barH;  // Junction point
int16_t d = innerR * 2;

// 1. Draw full AA circle centered at junction
spr.fillSmoothRoundRect(cx - innerR, cy - innerR, d, d, innerR, color, LCARS_BLACK);

// 2. Cover top half (bar) and left half (sidebar)
spr.fillRect(x, y, ew, barH, color);           // Bar covers top
spr.fillRect(x, cy, sideW, innerR + 1, color); // Sidebar covers left
// Result: only bottom-right quarter-circle is visible
```

**Key insight:** `fillSmoothRoundRect` with width = height = 2 * radius creates a perfectly anti-aliased circle. The radius parameter of `innerR` means every corner is fully rounded — it becomes a circle. We use this instead of `fillSmoothCircle` because TFT_eSPI's round-rect AA is more reliable for this use case.

### 2.2 Bars

Horizontal bands that extend from elbows across the top and bottom of the frame.

**Cap styles:**

| Cap | Usage | Visual |
|-----|-------|--------|
| `LCARS_CAP_NONE` | Connected end (adjacent to elbow) | Flat edge `│` |
| `LCARS_CAP_PILL` | Free end (exposed edge) | Semicircle `)` with radius = height/2 |
| `LCARS_CAP_ROUND` | Small rounding (defined but rarely used) | Slight curve |

In a standard frame, bars have a flat left edge (connected to the elbow) and a pill cap on the right:

```
│████████████████████████████████████████●●●●●│
 ^                                       ^
 flat left                          pill cap right
```

**Drawing technique:** Draw a full `fillSmoothRoundRect` (pill on both ends), then `fillRect` to square off the unwanted side:

```cpp
spr.fillSmoothRoundRect(x, y, w, h, r, color, LCARS_BLACK);  // Full pill
if (!roundLeft)
    spr.fillRect(x, y, r, h, color);      // Square off left end
if (!roundRight)
    spr.fillRect(x + w - r, y, r, h, color);  // Square off right end
```

**Partial bars** (`drawBarPartial`) extend a bar from a start X to an end X with an optional pill cap. These are used during animations — the bar "grows" from the elbow outward.

### 2.3 Sidebars

A vertical column of colored segments between the top and bottom elbows. This is one of LCARS's most distinctive visual elements — the "stacked data blocks" on the left edge of a panel.

```
│██████████╮    segment 1, rounded right edge
│██████████│
│          │    2px black gap
│██████████╮    segment 2
│██████████│
│          │    2px black gap
│██████████╮    segment 3
│██████████│
│          │    2px black gap
│██████████╮    segment 4
│██████████│
 ^         ^
 flat      rounded
 left      right (4px r)
```

**Properties:**
- Up to 4 segments, each a different color (defined by theme)
- 2px black gap between segments
- **Right edge:** rounded (4px radius) — the "indicator block" look
- **Left edge:** always flat — flush with the elbows above and below
- Last segment absorbs any remaining height (avoids rounding gaps)

**Drawing technique:** Each segment is a `fillSmoothRoundRect` (rounded on all sides), then a `fillRect` to flatten the left edge:

```cpp
spr.fillSmoothRoundRect(x, sy, w, actualH, 4, colors[i], LCARS_BLACK);
spr.fillRect(x, sy, 4, actualH, colors[i]);  // Flatten left edge
```

### 2.4 The Standard Frame

The complete LCARS frame border that surrounds every screen:

```
 0         34 50 54                                       320
 │          │  │  │                                        │
 ├──────────┼──┼──┼────────────────────────────────────────┤
 │          │  :  │                                     ╮  │  y=0
 │ TL ELBOW │gap  │ TOP BAR  (pill cap →)          ●●●●│  │
 │          │  :  │                                     │  │  y=18
 ├──────────┤  :  ├─────────────────────────────────────┘  │  y=22
 │          │     │                                        │
 │ SIDEBAR  │     │                                        │  y=36
 │ seg 1    │     │                                        │
 ├ ╌ ╌ ╌ ╌ ┤     │                                        │
 │ SIDEBAR  │     │       C O N T E N T                    │
 │ seg 2    │     │       A R E A                          │
 ├ ╌ ╌ ╌ ╌ ┤     │                                        │
 │ SIDEBAR  │     │       264 x 130 px                     │
 │ seg 3    │     │       (on 320x170)                     │
 ├ ╌ ╌ ╌ ╌ ┤     │                                        │
 │ SIDEBAR  │     │                                        │
 │ seg 4    │     │                                        │
 ├──────────┤  :  ├─────────────────────────────────────╮  │  y=H-32
 │          │  :  │                                     │  │
 │ BL ELBOW │gap  │ BOTTOM BAR  (pill cap →)       ●●●●│  │
 │          │  :  │                                     ╯  │  y=H
 ├──────────┼──┼──┼────────────────────────────────────────┤
 │          │  │  │                                        │
 0         34 50 54                                       320
```

**Dimensions** (colors shown are TNG Classic theme):

| Element | Position | Size |
|---------|----------|------|
| Sidebar | x: 0 | w: 34px |
| Elbow radius | — | 16px |
| Gap | between elbow and bar | 3px |
| Top bar | y: 0 | h: 18px |
| Bottom bar | y: H-16 | h: 16px |
| **Content area** | **(54, 22)** | **(264 × 130)** on 320×170 |

---

## 3. Color System

### 3.1 RGB565 Format

TFT displays use 16-bit color — 5 bits red, 6 bits green, 5 bits blue. This gives 65,536 possible colors in just 2 bytes per pixel.

```cpp
#define LCARS_RGB565(r, g, b) \
    (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
```

**Precision:** Red and blue each have 32 levels (5-bit), green has 64 levels (6-bit). The human eye is most sensitive to green, which is why it gets the extra bit. For LCARS, this is plenty — the warm amber/gold palette translates well to RGB565.

### 3.2 TNG Classic Palette

Based on Michael Okuda's reference palette. Source: [thelcars.com/colors.php](https://www.thelcars.com/colors.php)

**Warm structural colors** (bars, elbows, sidebars):

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_GOLD` | `#FFAA00` | Primary gold, accent highlights |
| `LCARS_AMBER` | `#FF9900` | Golden orange, top bar |
| `LCARS_ORANGE` | `#FF8800` | Orange accent |
| `LCARS_BUTTERSCOTCH` | `#FF9966` | Warm tone, top elbow |
| `LCARS_SUNFLOWER` | `#FFCC99` | Light panels, primary text |
| `LCARS_PEACH` | `#FF8866` | Sidebar segment |
| `LCARS_TAN` | `#CC9966` | Bottom bar |

**Cool data colors:**

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_ICE` | `#99CCFF` | Sky blue — progress bars, status OK, gauges |
| `LCARS_BLUE` | `#5566FF` | Blue accent |
| `LCARS_PERIWINKLE` | `#9999FF` | Blue panels |

**Purple accents:**

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_VIOLET` | `#CC99FF` | African violet — sidebar segment, accent |
| `LCARS_LILAC` | `#CC55FF` | Lilac highlight |
| `LCARS_LAVENDER` | `#CC99CC` | Bottom elbow, sidebar segment |

**Alert/status:**

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_TOMATO` | `#FF5555` | Alert, error status, sidebar segment |
| `LCARS_MARS` | `#FF2200` | Critical alert |
| `LCARS_GREEN` | `#00FF88` | OK/success indicator |

**Text and UI:**

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_WHITE` | `#F5F6FA` | Primary text (slightly warm white) |
| `LCARS_TEXT_DIM` | `#888888` | Secondary/label text |
| `LCARS_BLACK` | `#000000` | Background (always) |
| `LCARS_BAR_TRACK` | `#1A1A1A` | Unfilled progress bar background |

### 3.3 Nemesis Blue Palette

Cool, military aesthetic from Star Trek: Nemesis (2002). Replaces warm golds with blues.

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_NEM_COOL` | `#6699FF` | Primary blue |
| `LCARS_NEM_EVENING` | `#2266FF` | Evening blue |
| `LCARS_NEM_MIDNIGHT` | `#2233FF` | Deep blue |
| `LCARS_NEM_GHOST` | `#88BBFF` | Ghost blue (text) |
| `LCARS_NEM_GRAPE` | `#9966CC` | Purple accent |
| `LCARS_NEM_HONEY` | `#FFCC99` | Warm accent (warnings) |
| `LCARS_NEM_TANGERINE` | `#FF8833` | Orange accent |
| `LCARS_NEM_CARDINAL` | `#CC2233` | Cardinal red (alerts) |

### 3.4 Lower Decks Palette

Vivid, saturated colors from the animated series (2020-2024).

| Constant | Hex | Role |
|----------|-----|------|
| `LCARS_LD_PURPLE` | `#BB44FF` | Vivid purple |
| `LCARS_LD_MAGENTA` | `#FF44CC` | Hot pink |
| `LCARS_LD_CYAN` | `#44DDFF` | Bright cyan |
| `LCARS_LD_CORAL` | `#FF6688` | Coral pink |
| `LCARS_LD_LIME` | `#66FF66` | Bright green |
| `LCARS_LD_TANGERINE` | `#FF8822` | Vivid orange |
| `LCARS_LD_SKY` | `#66BBFF` | Sky blue |
| `LCARS_LD_LAVENDER` | `#DD88FF` | Bright lavender |

### 3.5 Color Roles

The LCARS color system follows a clear hierarchy:

| Role | Color Family | Examples |
|------|-------------|----------|
| **Structural** (elbows, bars, sidebars) | Warm — golds, ambers, oranges | Butterscotch, amber, peach, tan |
| **Data** (values, progress, gauges) | Cool — blues, ice tones | Ice, blue, periwinkle |
| **Accent** (highlights, emphasis) | Purple — violets, lavenders | Violet, lilac, lavender |
| **Status OK** | Cool | Ice (TNG), cool blue (Nemesis) |
| **Status Warning** | Warm | Gold |
| **Status Error** | Red | Tomato, mars |
| **Text (primary)** | Near-white or warm light | Sunflower (TNG), ghost (Nemesis), white (Lower Decks) |
| **Text (secondary)** | Gray | `#888888` |
| **Text on bars** | High contrast against bar color | Black (on warm bars), white (on red alert) |
| **Background** | Black | Always `#000000` — non-negotiable |

---

## 4. Theme System

### 4.1 LcarsTheme Structure

A theme defines the complete color mapping for all LCARS elements:

```cpp
struct LcarsTheme {
    const char* name;

    // Structural frame colors
    uint16_t elbowTop;        // Top-left elbow
    uint16_t elbowBottom;     // Bottom-left elbow
    uint16_t barTop;          // Top horizontal bar
    uint16_t barBottom;       // Bottom horizontal bar

    // Sidebar (up to 4 colored segments)
    uint16_t sidebar[4];
    uint8_t  sidebarCount;

    // Content colors
    uint16_t text;            // Primary text
    uint16_t textDim;         // Secondary/label text
    uint16_t textOnBar;       // Text rendered on colored bars
    uint16_t accent;          // Highlight / accent

    // Widget colors
    uint16_t progressFg;      // Progress bar foreground
    uint16_t progressBg;      // Progress bar background track
    uint16_t gaugeColor;      // Donut gauge default

    // Status colors
    uint16_t statusOk;        // OK / online
    uint16_t statusWarn;      // Warning
    uint16_t statusErr;       // Error / critical
    uint16_t alert;           // Red alert state

    // Background (always black)
    uint16_t background;
};
```

### 4.2 Built-in Themes

**TNG Classic** — The canonical warm palette:

| Element | Color | Constant |
|---------|-------|----------|
| Top elbow | Butterscotch | `LCARS_BUTTERSCOTCH` |
| Bottom elbow | Lavender | `LCARS_LAVENDER` |
| Top bar | Amber | `LCARS_AMBER` |
| Bottom bar | Tan | `LCARS_TAN` |
| Sidebar | Peach → Lavender → Tomato → Violet | 4 segments |
| Text | Sunflower | `LCARS_SUNFLOWER` |
| Data/gauges | Ice | `LCARS_ICE` |

**Nemesis** — Cool military blues:

| Element | Color | Constant |
|---------|-------|----------|
| Top elbow | Cool blue | `LCARS_NEM_COOL` |
| Bottom elbow | Grape | `LCARS_NEM_GRAPE` |
| Sidebar | Ghost → Evening → Midnight → Grape | 4 segments |
| Text | Ghost blue | `LCARS_NEM_GHOST` |
| Data/gauges | Cool blue | `LCARS_NEM_COOL` |

**Red Alert** — Emergency monochrome:

| Element | Color | Constant |
|---------|-------|----------|
| All elbows/bars | Mars / Tomato | Alternating reds |
| Sidebar | Tomato → Mars → Tomato → Mars | Alternating |
| Text | White | `LCARS_WHITE` |
| Text on bars | White | High contrast for urgency |

**Lower Decks** — Vivid animated series:

| Element | Color | Constant |
|---------|-------|----------|
| Top elbow | Purple | `LCARS_LD_PURPLE` |
| Bottom elbow | Magenta | `LCARS_LD_MAGENTA` |
| Sidebar | Cyan → Magenta → Lavender → Tangerine | 4 segments |
| Text | White | `LCARS_WHITE` |
| Data/gauges | Cyan | `LCARS_LD_CYAN` |

### 4.3 Creating Custom Themes

```cpp
LcarsTheme myTheme = {
    .name = "CUSTOM",
    .elbowTop      = LCARS_RGB565(0x00, 0xFF, 0x88),
    .elbowBottom   = LCARS_RGB565(0x00, 0xCC, 0x66),
    .barTop        = LCARS_RGB565(0x00, 0xDD, 0x77),
    .barBottom     = LCARS_RGB565(0x00, 0xAA, 0x55),
    .sidebar       = { LCARS_RGB565(0x00, 0xFF, 0xAA), LCARS_RGB565(0x00, 0xCC, 0x88),
                       LCARS_RGB565(0x00, 0xAA, 0x66), LCARS_RGB565(0x00, 0x88, 0x44) },
    .sidebarCount  = 4,
    .text          = LCARS_WHITE,
    .textDim       = LCARS_TEXT_DIM,
    .textOnBar     = LCARS_BLACK,
    .accent        = LCARS_RGB565(0x00, 0xFF, 0xCC),
    .progressFg    = LCARS_ICE,
    .progressBg    = LCARS_BAR_TRACK,
    .gaugeColor    = LCARS_ICE,
    .statusOk      = LCARS_ICE,
    .statusWarn    = LCARS_GOLD,
    .statusErr     = LCARS_TOMATO,
    .alert         = LCARS_MARS,
    .background    = LCARS_BLACK,
};

engine.setTheme(myTheme);
```

**Color harmony rules for LCARS themes:**

1. Pick **2-3 warm colors** in the same family for structural elements (elbows, bars)
2. Pick **1-2 cool colors** for data display (progress bars, gauges, values)
3. Choose **1 accent color** (often from the purple/violet family)
4. **Status colors should be semantic:** green/blue = OK, gold = warning, red = error
5. **Background is always black** — this is non-negotiable in LCARS canon
6. **Text on bars** should have high contrast against the bar color (typically black on warm bars, white on dark bars)
7. **Sidebar segments** should use a mix of structural and accent colors — variety is part of the LCARS look

---

## 5. Typography

### 5.1 Font Choice: Antonio

The engine uses **Antonio** from Google Fonts — a condensed, geometric sans-serif typeface. It was chosen because:

- **Geometric proportions** similar to Swiss 911 Ultra Compressed, the font family believed to be used on the actual Star Trek props
- **Condensed width** fits more text on small displays (320px wide)
- **Clean letterforms** render well at small sizes with anti-aliasing
- **Open source** (OFL license) — freely usable

Fonts are compiled from `Antonio.ttf` to VLW format (TFT_eSPI's variable-length width bitmap format) using the `create_smooth_fonts.py` tool. The VLW format embeds anti-aliased glyph bitmaps, enabling smooth text rendering on TFT displays without GPU-accelerated font rasterization.

### 5.2 Size Hierarchy

| Enum | Pixels | Usage |
|------|--------|-------|
| `LCARS_FONT_SM` | 12px | Labels, small status text, section headers, status row entries |
| `LCARS_FONT_14` | 14px | Compact labels, tight layouts |
| `LCARS_FONT_16` | 16px | Body text, general content |
| `LCARS_FONT_MD` | 18px | Values, screen titles, boot sequence title |
| `LCARS_FONT_20` | 20px | Medium emphasis, subtitles |
| `LCARS_FONT_LG` | 28px | Large numbers, value displays |
| `LCARS_FONT_XL` | 36px | Hero numbers, primary display values |

Additionally, TFT_eSPI's **built-in font 1** (8px pixel font) is used for the data cascade widget — its tiny monospace characters create the "streaming data" effect.

### 5.3 Auto-Uppercase Convention

All LCARS text is uppercase. The engine provides two text drawing functions:

- `LcarsFont::drawTextUpper()` — automatically converts to uppercase (use this by default)
- `LcarsFont::drawText()` — renders as-is (for pre-formatted or mixed-case data)

The auto-uppercase conversion uses `toupper()` on each character into a 128-byte static buffer.

### 5.4 Text Alignment (Datum)

TFT_eSPI supports multiple text datum modes for positioning:

| Datum | Meaning | Common Use |
|-------|---------|------------|
| `TL_DATUM` | Top-left (default) | Labels, left-aligned text |
| `TR_DATUM` | Top-right | Right-aligned values in status rows |
| `ML_DATUM` | Middle-left | Screen titles in the top bar |
| `MC_DATUM` | Middle-center | Centered button text |

---

## 6. Rendering Architecture

### 6.1 Full-Screen Sprite in PSRAM

The engine allocates a full-screen `TFT_eSprite` in PSRAM at startup:

- **Buffer size:** 320 × 170 × 2 = **108,800 bytes** (~106 KB)
- **Color depth:** 16-bit (RGB565)
- **Location:** PSRAM (ESP32-S3 has 8MB OPI PSRAM)

All drawing operations target this off-screen sprite. Once a frame is complete, a single `pushSprite(0, 0)` call DMAs the entire buffer to the physical display. This produces **zero-flicker rendering** — no partial draws are ever visible on screen.

### 6.2 Anti-Aliasing on Embedded Displays

At 320×170 resolution, **every pixel is visible**. Aliased curves (stairstepped edges) look jagged and unprofessional. Anti-aliasing is essential for the polished LCARS aesthetic.

TFT_eSPI provides these AA drawing primitives:

| Function | Usage |
|----------|-------|
| `fillSmoothRoundRect()` | Rounded rectangles — bars, pills, elbow circles, sidebar segments |
| `drawSmoothArc()` | Arc rings — donut gauges, indicator outlines |
| `fillSmoothCircle()` | Filled circles — indicator dots, small progress fills |

All AA functions take a `bg_color` parameter. Edge pixels are blended between the foreground color and this background color. Since LCARS elements always sit on black, `LCARS_BLACK` is passed as the background everywhere.

### 6.3 Rendering Pipeline

Every frame follows this sequence:

```
LcarsEngine::update()
 |
 +-- Calculate delta time and FPS
 |
 +-- [If transitioning] --> _doTransition()
 |    +-- First half:  bars contract from old screen
 |    +-- Midpoint:    teardown old, setup new screen
 |    +-- Second half: bars extend to new screen
 |    +-- pushSprite()
 |
 +-- [If steady-state] --> _renderFrame()
      +-- Check refresh interval
      +-- screen->onUpdate(deltaMs)
      +-- Clear sprite to black
      +-- drawStandardFrame() --> LCARS border
      +-- Draw screen title in top bar
      +-- screen->onDraw(spr, contentRect)
      +-- pushSprite()
```

**Frame rate:** The engine targets 30 FPS (`LCARS_FPS_TARGET`). Actual timing is adaptive — it renders as fast as possible and tracks delta time for animations. The FPS counter updates every 1000ms.

### 6.4 Display Initialization

```cpp
tft.init();
tft.invertDisplay(true);   // ST7789 panels need inversion for correct colors
tft.setRotation(3);        // Landscape orientation
tft.fillScreen(TFT_BLACK);
```

**ESP32-S3 SPI port bug:** When using TFT_eSPI in SPI mode (Waveshare board), you must add `-DUSE_FSPI_PORT` to build flags. Without it, `SPI_PORT` defaults to `FSPI=1`, but ESP-IDF's `REG_SPI_BASE(1)` returns 0 (only handles i>=2), causing a crash in `begin_tft_write()`. The fix forces `SPI_PORT=2` (SPI2_HOST) which maps to the correct register.

---

## 7. Layout System

### 7.1 Frame Dimensions

All layout constants are defined in `lcars_config.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LCARS_SIDEBAR_W` | 34px | Sidebar width |
| `LCARS_ELBOW_R` | 16px | Inner curve radius |
| `LCARS_TOPBAR_H` | 18px | Top bar height |
| `LCARS_BOTBAR_H` | 16px | Bottom bar height |
| `LCARS_BAR_GAP` | 3px | Gap between elbow and bar extension |

### 7.2 Content Area

`drawStandardFrame()` returns a `Rect` with the exact usable content area:

```
CONTENT_X = SIDEBAR_W + ELBOW_R + BAR_GAP + 1 = 34 + 16 + 3 + 1 = 54
CONTENT_Y = TOPBAR_H + 4 = 18 + 4 = 22
CONTENT_W = SCR_WIDTH - CONTENT_X - 2 = 320 - 54 - 2 = 264
CONTENT_H = SCR_HEIGHT - TOPBAR_H - BOTBAR_H - 6
```

| Display | Resolution | Content Area |
|---------|------------|--------------|
| LILYGO T-Display-S3 | 320 × 170 | 264 × 130 |
| Waveshare LCD-1.47 | 320 × 172 | 264 × 132 |

### 7.3 Spacing Rules

These pixel-precise vertical spacing values were refined by visual testing on hardware. Apply them consistently across all screens:

| Context | Spacing | Example |
|---------|---------|---------|
| Section header to first content | **+14px** | `drawLabel()` → first `drawStatusRow()` |
| Label/status row to progress bar below | **+18px** | `drawStatusRow()` → `drawProgressBar()` |
| Progress bar to next label below | **+7px** | `drawProgressBar()` → next row |
| Before separator line | **+4px** | Content → `drawSeparator()` |
| After separator line | **+5px** | `drawSeparator()` → next content |
| After separator before data cascade | **+5px** | `drawSeparator()` → `drawDataCascade()` |

**Donut gauge placement:** Labels should go **to the left** of the gauge, not below it. On small displays, labels below the gauge overlap with the bottom elbow.

**Layout pattern — Y accumulator:**

```cpp
void MyScreen::onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) {
    int16_t y = c.y;

    LcarsWidgets::drawLabel(spr, c.x, y, "SECTION TITLE", _theme->accent);
    y += 14;

    LcarsWidgets::drawStatusRow(spr, c.x, y, c.w, "CPU", "47%",
                                 _theme->text, _theme->textDim);
    y += 18;

    LcarsWidgets::drawProgressBar(spr, c.x, y, c.w, 8, 0.47f,
                                   _theme->progressFg, _theme->progressBg);
    y += 7;
    // ... continue stacking
}
```

---

## 8. Animation System

### 8.1 Boot Sequence

The boot screen (`LcarsBootScreen`) runs two animated phases before transitioning to the first user screen:

**Phase 1: Diagnostic Text** (`LCARS_BOOT_DIAG_MS = 2000ms`)

Lines of Trek-style diagnostic text reveal progressively from top to bottom:

```
LCARS INTERFACE SYSTEM              ← gold, 18px (LCARS_FONT_MD)
BUILD 47391.2                       ← amber, 12px

INITIALIZING DISPLAY CORE..........OK    ← amber
LOADING FONT DATABASE..............OK    ← amber
THEME ENGINE ONLINE................OK    ← amber
WIDGET SUBSYSTEM READY.............OK    ← amber
SENSOR ARRAY CALIBRATING...........OK    ← amber
COMMUNICATIONS ARRAY...............STANDBY  ← amber

ALL SYSTEMS NOMINAL                 ← green
TRANSFERRING CONTROL TO PRIMARY DISPLAY  ← amber
█                                   ← blinking cursor (300ms interval)
```

**Phase 2: Frame Assembly** (`LCARS_BOOT_FRAME_MS = 600ms`)

Frame elements assemble with overlapping timing:

| Time (normalized) | Element | Animation |
|-------------------|---------|-----------|
| t = 0.0 – 0.4 | Elbows | Appear instantly at t > 0 |
| t = 0.2 – 0.7 | Bars | Extend from elbows with `easeOutQuad` |
| t = 0.4 – 1.0 | Sidebar segments | Reveal one-by-one from top |

The overall progress uses `easeOutCubic` — fast start, smooth deceleration — giving the animation visual "punch."

### 8.2 Screen Transitions

Total duration: `LCARS_TRANSITION_MS = 300ms`

The transition is split into two halves:

1. **First half (0-150ms):** Current screen's bars contract inward (bar length × (1 - t))
2. **Midpoint (150ms):** Old screen teardown → new screen setup
3. **Second half (150-300ms):** New screen's bars extend outward (bar length × t)

Elbows and sidebar remain visible throughout — only the bars animate. The screen title appears on the extending bar once it's long enough (>20px past the start).

### 8.3 Easing Functions

Available in the `LcarsEasing` namespace:

| Function | Formula | Curve | Usage |
|----------|---------|-------|-------|
| `linear(t)` | `t` | Straight line | Uniform motion |
| `easeInQuad(t)` | `t²` | Accelerating | — |
| `easeOutQuad(t)` | `t(2-t)` | Decelerating | Bar extension in transitions and boot |
| `easeInOutQuad(t)` | Piecewise parabolic | S-curve | — |
| `easeOutCubic(t)` | `(t-1)³ + 1` | Strong deceleration | Boot overall progress |

### 8.4 Continuous Animations

These run every frame without explicit state management:

- **Data cascade:** Characters update every `LCARS_CASCADE_SPEED_MS = 50ms` via a time-based hash (`millis() / 50 + seed`). Uses Knuth's multiplicative hash for pseudo-random appearance.
- **Blinking indicator:** Toggles between filled circle and outline ring every `LCARS_BLINK_INTERVAL_MS = 500ms` using `(millis() / 500) % 2`.

---

## 9. Audio Design

### 9.1 Sound Design Principles

LCARS interface sounds in Star Trek are characteristically:

- **Short** — 50-150ms per tone. Quick chirps, not lingering notes.
- **Clean** — simple square waves. No reverb, no complex waveforms.
- **Mid-high frequency** — 800-2400Hz range. Audible but not harsh.
- **Purposeful** — each sound maps to a specific UI action.

The engine replicates this using the ESP32's `tone()` function, which generates square waves via a hardware timer. No DAC, no audio codec — just a piezo buzzer or small speaker on a GPIO pin.

### 9.2 Beep Catalog

| Function | Frequency | Duration | Gap | UI Action |
|----------|-----------|----------|-----|-----------|
| `beepHigh()` | 1760 Hz | 60ms | — | Button press / tap |
| `beepLow()` | 880 Hz | 80ms | — | Screen transition / navigation |
| `beepConfirm()` | 1200 → 1800 Hz | 60ms + 80ms | 30ms | Confirmation / success |
| `beepAlert()` | 1400 Hz × 3 | 50ms × 3 | 30ms × 2 | Alert / warning |
| `beepBoot()` | 880 → 1320 → 1760 → 2200 Hz | 60 + 60 + 60 + 100ms | 30ms × 3 | Boot sequence complete |

**Design notes:**
- `beepHigh` at 1760 Hz (A6) matches the TNG button chirp frequency range
- `beepBoot` ascending sweep creates a "powering up" impression — each tone is a perfect fifth above the previous
- `beepAlert` uses three rapid identical tones — urgency without melody
- `beepConfirm` rises by a perfect fifth (1200→1800 Hz) — the universal "success" interval

### 9.3 Non-Blocking Architecture

All audio is non-blocking:

```cpp
LcarsAudio::begin(BUZZER_PIN);  // Initialize (pass -1 to disable)
LcarsAudio::beepHigh();         // Starts immediately, returns instantly
LcarsAudio::update();           // Call in loop() to handle tone-off and sequencing
```

Multi-tone sequences (confirm, alert, boot) are queued internally. The `update()` function advances through the sequence with 30ms gaps between tones. Up to 4 tones per sequence.

---

## 10. Widget Catalog

All widgets are static functions in the `LcarsWidgets` namespace.

### Label

Auto-uppercase text label. The most basic text widget.

```cpp
void drawLabel(TFT_eSprite& spr, int16_t x, int16_t y,
               const char* text, uint16_t color,
               LcarsFontSize size = LCARS_FONT_SM, uint8_t datum = TL_DATUM);
```

### Value + Label

Large value number with a small descriptor label underneath. The value is drawn as-is (not uppercased — it's typically a number), and the label is auto-uppercased. 6px vertical gap between value and label.

```cpp
void drawValueLabel(TFT_eSprite& spr, int16_t x, int16_t y,
                    const char* value, const char* label,
                    uint16_t valueColor, uint16_t labelColor,
                    LcarsFontSize valueSize = LCARS_FONT_LG,
                    LcarsFontSize labelSize = LCARS_FONT_SM);
```

### Progress Bar

Horizontal bar with pill-capped ends. Background track is always visible; foreground fill extends proportionally.

```cpp
void drawProgressBar(TFT_eSprite& spr, int16_t x, int16_t y,
                     int16_t w, int16_t h, float percent,
                     uint16_t fgColor, uint16_t bgColor);
```

- `percent`: 0.0 to 1.0
- When fill width < bar height: draws a single filled circle instead of a bar
- Cap radius = height / 2 (automatic)
- Background: typically `LCARS_BAR_TRACK` (#1A1A1A)

### Status Row

Left-aligned label, right-aligned value — the standard LCARS data display pattern.

```cpp
void drawStatusRow(TFT_eSprite& spr, int16_t x, int16_t y,
                   int16_t w, const char* label, const char* value,
                   uint16_t valueColor, uint16_t labelColor,
                   LcarsFontSize size = LCARS_FONT_SM);
```

Both label and value are auto-uppercased. Value uses `TR_DATUM` for right alignment.

### Separator

Single-pixel horizontal line for visual grouping.

```cpp
void drawSeparator(TFT_eSprite& spr, int16_t x, int16_t y,
                   int16_t w, uint16_t color);
```

Use spacing: +4px before, +5px after.

### Donut Gauge

Circular ring indicator inspired by [Project RITOS](https://www.mewho.com/ritos/) ring gauges. The arc starts at 12 o'clock (270 degrees) and sweeps clockwise.

```cpp
void drawDonutGauge(TFT_eSprite& spr, int16_t cx, int16_t cy,
                    int16_t outerR, int16_t thickness,
                    float percent, uint16_t fgColor, uint16_t bgColor);
```

- `outerR`: outer edge radius
- `thickness`: ring width (innerR = outerR - thickness)
- Handles wrap-around: when arc exceeds 360 degrees, draws in two segments (270→360, then 0→remainder)
- Uses `drawSmoothArc()` for anti-aliased edges

### Pill Button

LCARS pill-shaped button with centered uppercase text.

```cpp
void drawPillButton(TFT_eSprite& spr, int16_t x, int16_t y,
                    int16_t w, int16_t h, const char* text,
                    uint16_t color, uint16_t textColor = 0x0000,
                    LcarsFontSize size = LCARS_FONT_SM);
```

Radius = height / 2 for full pill shape. Text centered using `MC_DATUM`.

### Data Cascade

Streaming pseudo-random hex digits — pure visual flair suggesting active data processing. Uses the tiny built-in 8px pixel font.

```cpp
void drawDataCascade(TFT_eSprite& spr, int16_t x, int16_t y,
                     int16_t w, int16_t h, uint16_t color,
                     uint32_t seed = 0);
```

- Characters from "0123456789ABCDEF"
- Grid: 7px wide × 10px tall per character
- Updates every 50ms using Knuth multiplicative hash: `(t * 2654435761u) ^ (row * 31 + col * 17 + seed)`
- Different `seed` values produce different patterns

### Blinking Indicator

Small circle that blinks at 500ms intervals. Alternates between filled circle (on) and outline ring (off). When `active=false`, stays solid.

```cpp
void drawIndicator(TFT_eSprite& spr, int16_t x, int16_t y,
                   int16_t radius, uint16_t color, bool active = true);
```

### Number Formatting

**formatCount** — SI suffix formatting for large numbers:

| Input | Output |
|-------|--------|
| 1,234,567 | `"1.23M"` |
| 45,600 | `"45.6K"` |
| 999 | `"999"` |

**formatCost** — Currency formatting:

| Input | Output |
|-------|--------|
| 1234.56 | `"$1235"` |
| 12.50 | `"$12.50"` |
| 0.0042 | `"$0.0042"` |

---

## 11. Screen System

### 11.1 Screen Lifecycle

Screens extend `LcarsScreen` and implement lifecycle hooks:

```
 onSetup()                      Called once when screen becomes active
    |
    +----> onUpdate(deltaMs)    Called each frame for data/logic
    |          |
    |          +----> onDraw()  Called each frame to render content
    |                 |
    +-----------------+         (repeats at refresh interval)
    |
 onTeardown()                   Called once when screen is replaced
```

### 11.2 Creating a Screen

```cpp
class StatusScreen : public LcarsScreen {
    const char* title() const override { return "SYSTEM STATUS"; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t y = c.y;

        LcarsWidgets::drawLabel(spr, c.x, y, "DIAGNOSTICS",
                                _theme->accent, LCARS_FONT_SM);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, c.x, y, c.w,
                                     "POWER", "98%",
                                     _theme->statusOk, _theme->textDim);
        y += 18;

        LcarsWidgets::drawProgressBar(spr, c.x, y, c.w, 8, 0.98f,
                                       _theme->progressFg, _theme->progressBg);
    }
};
```

### 11.3 Configuration Methods

| Method | Default | Purpose |
|--------|---------|---------|
| `title()` | `"LCARS"` | Text shown in the top bar (dark on bar color) |
| `wantsFrame()` | `true` | Whether to draw the standard LCARS frame. Set `false` for custom full-screen rendering (e.g., boot sequence). |
| `refreshIntervalMs()` | `0` | Throttle redraws. 0 = every frame (~30fps). Set higher for screens that don't need constant updates. |

---

## Appendix A: Quick Reference

### Layout Constants

| Constant | Value |
|----------|-------|
| `LCARS_SIDEBAR_W` | 34px |
| `LCARS_ELBOW_R` | 16px |
| `LCARS_TOPBAR_H` | 18px |
| `LCARS_BOTBAR_H` | 16px |
| `LCARS_BAR_GAP` | 3px |
| Content origin (320×170) | (54, 22) |
| Content size (320×170) | 264 × 130 |

### Timing Constants

| Constant | Value |
|----------|-------|
| `LCARS_BOOT_DIAG_MS` | 2000ms |
| `LCARS_BOOT_FRAME_MS` | 600ms |
| `LCARS_TRANSITION_MS` | 300ms |
| `LCARS_BLINK_INTERVAL_MS` | 500ms |
| `LCARS_CASCADE_SPEED_MS` | 50ms |
| `LCARS_FPS_TARGET` | 30 |
| `LCARS_DIM_TIMEOUT_MS` | 120000ms (2 min) |
| `LCARS_BACKLIGHT_FULL` | 255 |
| `LCARS_BACKLIGHT_DIM` | 40 |

### Spacing Rules

| Context | Value |
|---------|-------|
| Section header → content | +14px |
| Label → progress bar | +18px |
| Progress bar → next label | +7px |
| Before separator | +4px |
| After separator | +5px |

### Font Sizes

| Enum | Size | Usage |
|------|------|-------|
| `LCARS_FONT_SM` | 12px | Labels, status |
| `LCARS_FONT_14` | 14px | Compact |
| `LCARS_FONT_16` | 16px | Body |
| `LCARS_FONT_MD` | 18px | Titles |
| `LCARS_FONT_20` | 20px | Medium |
| `LCARS_FONT_LG` | 28px | Large |
| `LCARS_FONT_XL` | 36px | Hero |
| Built-in 1 | 8px | Data cascade |

---

## Appendix B: External References

### LCARS Design Sources

- **[thelcars.com Color Guide](https://www.thelcars.com/colors.php)** — Canonical LCARS color palette reference. The TNG palette in this engine is derived from this source.
- **[LCARS-SDK Elbow Geometry](https://github.com/Aricwithana/LCARS-SDK/issues/4)** — Detailed discussion of elbow geometry, curve calculations, and element proportions.
- **[cb-lcars Component Taxonomy](https://github.com/snootched/cb-lcars)** — Comprehensive LCARS component classification and naming conventions.
- **[Project RITOS](https://www.mewho.com/ritos/)** — The gold standard for interactive LCARS interfaces. Inspiration for donut gauges and interactive element design.
- **[LCARS CSS Framework](https://joernweissenborn.github.io/lcars/)** — Web-based LCARS implementation. Useful for understanding element proportions in a different rendering context.

### Technical References

- **[TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)** — The display driver library used by this engine. Provides sprite management, smooth font rendering, and anti-aliased drawing primitives.
- **[Antonio Font](https://fonts.google.com/specimen/Antonio)** — The typeface used throughout the engine, available under the Open Font License.
- **Michael Okuda** — Scenic art supervisor for Star Trek: TNG, DS9, Voyager, and the TNG-era films. Creator of the LCARS visual language.
