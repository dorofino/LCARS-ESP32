# LCARS-ESP32

A procedural LCARS (Star Trek TNG) UI rendering engine for ESP32-S3 + TFT_eSPI. Anti-aliased elbows, animated transitions, boot sequences, color themes, and a full widget library — all rendered in real-time on a tiny display.

**No pre-rendered images. No PNGs. Every pixel is drawn procedurally.**

## Features

- **Authentic LCARS primitives** — elbows (4 orientations), pill-capped bars, segmented sidebars, all with anti-aliased curves
- **Widget library** — progress bars, donut gauges, status rows, pill buttons, data cascades, blinking indicators
- **Screen system** — base class with lifecycle (setup/update/draw/teardown), automatic frame rendering
- **Animated transitions** — bar extend/contract between screens with easing
- **Boot sequence** — diagnostic text scroll + frame assembly animation
- **Color themes** — TNG Classic, Nemesis Blue, Red Alert (or define your own)
- **Font system** — Antonio font at 7 sizes (12px–36px), auto-uppercase
- **Multi-board** — LILYGO T-Display-S3 + Waveshare ESP32-S3-LCD-1.47

## Quick Start

### Clone and flash

```bash
git clone https://github.com/dorofino/lcars-esp32.git
cd lcars-esp32
pio run -e waveshare147 -t upload    # Waveshare board
# or
pio run -e tdisplays3 -t upload      # LILYGO T-Display-S3
```

### Use as a library in your project

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/dorofino/lcars-esp32.git
```

Then in your code:

```cpp
#include <lcars.h>

TFT_eSPI tft;
LcarsEngine engine;

class MyScreen : public LcarsScreen {
    const char* title() const override { return "MY SCREEN"; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        LcarsWidgets::drawLabel(spr, c.x, c.y, "HELLO LCARS",
                                _theme->accent, LCARS_FONT_LG);

        LcarsWidgets::drawProgressBar(spr, c.x, c.y + 40, c.w, 10,
                                      0.75f, LCARS_ICE, LCARS_BAR_TRACK);

        LcarsWidgets::drawDonutGauge(spr, c.x + 40, c.y + 90, 25, 6,
                                     0.82f, LCARS_VIOLET, LCARS_BAR_TRACK);
    }
};

MyScreen myScreen;

void setup() {
    engine.begin(tft);
    engine.setTheme(LCARS_THEME_TNG);
    engine.setScreen(&myScreen);
}

void loop() {
    engine.update();
}
```

## Architecture

```
┌─────────────────────────────────────────────────┐
│  LcarsEngine                                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐  │
│  │ Screen   │  │ Theme    │  │ Sprite       │  │
│  │ Manager  │  │ System   │  │ Buffer       │  │
│  └────┬─────┘  └──────────┘  └──────────────┘  │
│       │                                          │
│  ┌────▼──────────────────────────────────────┐  │
│  │ LcarsScreen (user subclass)               │  │
│  │  onSetup() → onUpdate() → onDraw() → ... │  │
│  └───────────────────────────────────────────┘  │
│       │                                          │
│  ┌────▼─────┐  ┌──────────┐  ┌──────────────┐  │
│  │ Frame    │  │ Widgets  │  │ Animation    │  │
│  │ Renderer │  │          │  │ System       │  │
│  └──────────┘  └──────────┘  └──────────────┘  │
└─────────────────────────────────────────────────┘
```

### Source Files

| File | Purpose |
|------|---------|
| `lcars.h` | Single-include entry point |
| `lcars_engine` | Core engine: display init, screen lifecycle, sprite management |
| `lcars_frame` | Frame primitives: elbow, bar, sidebar, standard frame |
| `lcars_screen` | Screen base class with lifecycle hooks |
| `lcars_widgets` | UI components: gauge, progress, button, label, cascade |
| `lcars_theme` | Color theme system with TNG/Nemesis/RedAlert presets |
| `lcars_font` | Antonio font management at 7 sizes |
| `lcars_animation` | Boot sequence, easing functions |
| `lcars_colors` | RGB565 color palette definitions |
| `lcars_config` | Layout constants, timing, dimensions |

## Themes

### TNG Classic
The canonical orange/amber/violet palette from The Next Generation.

### Nemesis Blue
Cool blue palette from Star Trek: Nemesis.

### Red Alert
All red with white text. Pair with blinking indicators.

### Custom Theme

```cpp
LcarsTheme myTheme = {
    .name = "CUSTOM",
    .elbowTop    = LCARS_RGB565(0x00, 0xFF, 0x88),
    .elbowBottom = LCARS_RGB565(0x00, 0xCC, 0x66),
    // ... (see lcars_theme.h for all fields)
};
engine.setTheme(myTheme);
```

## Supported Boards

| Board | Display | Interface | Resolution |
|-------|---------|-----------|------------|
| Waveshare ESP32-S3-LCD-1.47 | 1.47" IPS | SPI | 320x172 |
| LILYGO T-Display-S3 | 1.9" IPS | 8-bit parallel | 320x170 |

## Fonts

Antonio font (Google Fonts) pre-compiled to VLW format at 7 sizes:

| Enum | Size | Usage |
|------|------|-------|
| `LCARS_FONT_SM` | 12px | Labels, small status |
| `LCARS_FONT_14` | 14px | Compact labels |
| `LCARS_FONT_16` | 16px | Body text |
| `LCARS_FONT_MD` | 18px | Values, titles |
| `LCARS_FONT_20` | 20px | Medium emphasis |
| `LCARS_FONT_LG` | 28px | Large numbers |
| `LCARS_FONT_XL` | 36px | Primary display |

## Widgets

- **`drawLabel()`** — Uppercase text label
- **`drawValueLabel()`** — Large value + small label underneath
- **`drawProgressBar()`** — Horizontal bar with pill caps
- **`drawStatusRow()`** — "LABEL ... VALUE" row
- **`drawDonutGauge()`** — Circular percentage gauge
- **`drawPillButton()`** — LCARS pill-shaped button
- **`drawDataCascade()`** — Streaming hex numbers (visual flair)
- **`drawIndicator()`** — Blinking status circle
- **`drawSeparator()`** — Thin horizontal line

## LCARS Design References

This project follows authentic LCARS design principles:

- [LCARS-SDK Elbow Geometry](https://github.com/Aricwithana/LCARS-SDK/issues/4)
- [thelcars.com Color Guide](https://www.thelcars.com/colors.php)
- [cb-lcars Component Taxonomy](https://github.com/snootched/cb-lcars)
- [Project RITOS](https://www.mewho.com/ritos/) — gold standard for interactivity
- [LCARS CSS Framework](https://joernweissenborn.github.io/lcars/)

## License

MIT — do whatever you want with it. Contributions welcome.

## Contributing

PRs welcome! Some ideas:

- More widgets (chart, table, scrolling list)
- Touch input support
- More themes (Lower Decks, Discovery, Picard)
- Sound effects (buzzer beeps for button presses)
- Multi-display support
- WASM/web version for the WYSIWYG designer
