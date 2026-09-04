#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include "App_Globals.h"

/**
 * @brief Draws the static background and button frames for the main menu.
 */
void drawMenu() {
    // 2026 Deep Midnight Background
    gfx.fillScreen(0x0821); // Deep Navy Black
    
    // Modern Hex-Grid Pattern (Very Subtle)
    for(int i=0; i<=gfx.width(); i+=40) gfx.drawFastVLine(i, 0, gfx.height(), 0x0104);
    for(int i=0; i<=gfx.height(); i+=40) gfx.drawFastHLine(0, i, gfx.width(), 0x0104);

    gfx.setFont(&fonts::FreeSansBold9pt7b); // Scaled down to prevent clipping
    gfx.setTextColor(0x07FF); // Surgical Cyan
    gfx.setTextDatum(TC_DATUM);
    gfx.drawString("EL-BASEET CONTROL INTERFACE", gfx.width()/2, 15);

    // Menu Options
    const char* options[] = {"LIVE VIEW", "SURGICAL CONFIG", "CAMERA CONFIG", "SYSTEM INFO", "TOUCH CALIBRATE"};
    int btnW = 240; // Widened to prevent clipping
    int btnH = 30; // Shrink buttons to fit 5 items
    int startX = (gfx.width() - btnW) / 2;

    gfx.setFont(&fonts::DejaVu12); // Smaller font for long labels like SURGICAL CONFIG
    for(int i=0; i<5; i++) {
        int y = 60 + (i * 36);
        // High-tech hollow frame
        gfx.drawRect(startX, y, btnW, btnH, 0x07FF);
        gfx.drawRect(startX+1, y+1, btnW-2, btnH-2, 0x03EF); // Inner Cyan accent
        
        gfx.setTextColor(TFT_WHITE);
        gfx.setTextDatum(ML_DATUM);
        gfx.drawString(options[i], startX + 45, y + (btnH/2) + 2);
    }
}

/**
 * @brief Handles real-time icon animations. Called in the main loop.
 */
void updateMenuAnimations() {
    static uint32_t lastAnim = 0;
    if (millis() - lastAnim < 40) return; // ~25 FPS for smooth animation
    lastAnim = millis();

    int startX = (gfx.width() - 240) / 2;
    int iconX = startX + 22; // Center of the icon gutter
    uint16_t cyan = 0x07FF;

    // Professional Static/Subtle Icons
    // 1. LIVE VIEW: Static HUD Bracket
    gfx.drawRect(iconX - 6, 60 + 10, 12, 10, cyan);
    gfx.fillCircle(iconX, 60 + 15, 2, gfx.color888(255, 0, 0)); // Static REC dot

    // 2. SURGICAL CONFIG: Static Crosshair
    gfx.drawFastHLine(iconX - 7, 96 + 15, 14, cyan);
    gfx.drawFastVLine(iconX, 96 + 8, 14, cyan);

    // 3. CAMERA CONFIG: Static Lens Ring
    gfx.drawCircle(iconX, 132 + 15, 7, cyan);
    gfx.drawCircle(iconX, 132 + 15, 3, cyan);

    // 4. SYSTEM INFO: Static Info 'i'
    gfx.drawCircle(iconX, 168 + 15, 7, cyan);
    gfx.drawFastVLine(iconX, 168 + 13, 5, cyan);
    gfx.drawPixel(iconX, 168 + 11, cyan);

    // 5. TOUCH CALIBRATE: Static HUD Corners
    gfx.drawRect(iconX - 6, 204 + 10, 5, 5, cyan);
    gfx.drawRect(iconX + 1, 204 + 16, 5, 5, cyan);
}

#endif