#pragma once

#include <TFT_eSPI.h>
#include "lcars_theme.h"
#include "lcars_frame.h"

// ============================================================
// LCARS Screen Base Class
// Users extend this to create custom screens.
// The engine calls lifecycle methods in order:
//   onSetup() → [onUpdate() → onDraw()] per frame → onTeardown()
// ============================================================

class LcarsScreen {
public:
    virtual ~LcarsScreen() = default;

    // Called once when screen becomes active
    virtual void onSetup() {}

    // Called each frame for data/logic updates
    virtual void onUpdate(uint32_t deltaMs) {}

    // Called each frame to render. The sprite is already cleared.
    // The LCARS frame is drawn before this call; draw content inside the frame.
    virtual void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& content) = 0;

    // Called once when screen is about to be replaced
    virtual void onTeardown() {}

    // Screen title (shown in top bar)
    virtual const char* title() const { return "LCARS"; }

    // Whether this screen wants the standard LCARS frame
    // Set to false for full-screen custom rendering (e.g., boot sequence)
    virtual bool wantsFrame() const { return true; }

    // How often this screen should redraw (0 = every frame)
    virtual uint32_t refreshIntervalMs() const { return 0; }

protected:
    // Accessible to subclasses
    const LcarsTheme* _theme = nullptr;
    friend class LcarsEngine;
};
