#pragma once

// ============================================================
// LCARS Engine Configuration
// Screen dimensions MUST be provided via build flags:
//   -DSCR_WIDTH=320 -DSCR_HEIGHT=170 (or 172)
// ============================================================

#ifndef SCR_WIDTH
  #error "SCR_WIDTH must be defined via build flags in platformio.ini"
#endif
#ifndef SCR_HEIGHT
  #error "SCR_HEIGHT must be defined via build flags in platformio.ini"
#endif

// ── Layout Defaults ─────────────────────────────────────────
// These can be overridden per-screen; these are sensible LCARS defaults

#define LCARS_SIDEBAR_W     34     // Sidebar width (px)
#define LCARS_ELBOW_R       16     // Inner curve radius (px)
#define LCARS_TOPBAR_H      18     // Top bar height (px)
#define LCARS_BOTBAR_H      16     // Bottom bar height (px)
#define LCARS_BAR_GAP       3      // Gap between bar segments (px)
#define LCARS_BAR_CAP_R     -1     // Pill cap radius (-1 = auto = height/2)

// ── Content Area (computed from frame) ──────────────────────
#define LCARS_CONTENT_X     (LCARS_SIDEBAR_W + LCARS_ELBOW_R + LCARS_BAR_GAP + 1)
#define LCARS_CONTENT_Y     (LCARS_TOPBAR_H + 4)
#define LCARS_CONTENT_W     (SCR_WIDTH - LCARS_CONTENT_X - 2)
#define LCARS_CONTENT_H     (SCR_HEIGHT - LCARS_TOPBAR_H - LCARS_BOTBAR_H - 6)

// ── Animation Timing ────────────────────────────────────────
#define LCARS_BOOT_DIAG_MS       2000   // Boot diagnostic text duration
#define LCARS_BOOT_FRAME_MS      600    // Frame assembly animation duration
#define LCARS_TRANSITION_MS      2400   // Screen transition total duration (2 × 1200ms halves)
#define LCARS_BLINK_INTERVAL_MS  500    // Indicator blink rate
#define LCARS_CASCADE_SPEED_MS   50     // Data cascade update interval
#define LCARS_FPS_TARGET         30     // Target frames per second

// ── Backlight ───────────────────────────────────────────────
#define LCARS_BACKLIGHT_FULL     255
#define LCARS_BACKLIGHT_DIM      40
#define LCARS_DIM_TIMEOUT_MS     120000  // Dim after 2 minutes idle
