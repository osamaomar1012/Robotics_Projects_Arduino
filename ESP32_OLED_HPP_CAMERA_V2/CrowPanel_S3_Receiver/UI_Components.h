#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "App_Globals.h"
#include <TJpg_Decoder.h>

/**
 * @brief Helper to draw a JPEG image at a specific location
 * @param data Byte array of the JPEG file
 * @param len Size of the array
 * @param x X coordinate
 * @param y Y coordinate
 */
void drawImage(const uint8_t* data, size_t len, int x, int y) {
    // Using TJpgDec for high-speed hardware-accelerated JPEG rendering
    if (data == nullptr) return;
    TJpgDec.drawJpg(x, y, data, len);
}

/**
 * @brief Draws a futuristic "Laser" progress bar for system initialization.
 * @param percent 0-100 completion.
 * @param msg Status text to display.
 */
void updateWelcomeProgress(int percent, const char* msg) {
    // Centered bar with high-tech slate/gold aesthetic
    int w = 180; 
    int h = 2;
    int x = (gfx.width() - w) / 2; 
    int y = 195;

    gfx.setFont(&fonts::DejaVu12);
    gfx.setTextDatum(MC_DATUM);
    
    // Track: Deep Slate / Fill: Surgical Cyan
    gfx.fillRect(x, y, w, h, 0x2104); 
    gfx.fillRect(x, y, map(percent, 0, 100, 0, w), h, 0x07FF); 

    // Clear and draw status message
    gfx.fillRect(0, y + 10, 320, 25, TFT_BLACK);
    gfx.setTextColor(0xAD75, TFT_BLACK); // Muted Silver
    gfx.drawString(msg, gfx.width() / 2, y + 20);
}

/**
 * @brief Executes the EL-BASEET HPP_V3 welcome sequence with fade-in and audio.
 */
void showWelcomeScreen() {
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextDatum(MC_DATUM); // Middle Center Alignment for perfect centering

    // 1. Subtle High-Tech Grid (Medical Slate)
    for(int i=0; i<=gfx.width(); i+=20) gfx.drawFastVLine(i, 0, gfx.height(), 0x0841);
    for(int i=0; i<=gfx.height(); i+=20) gfx.drawFastHLine(0, i, gfx.width(), 0x0841);

    // 2. Animated Welcome Logo Sequence
    // We simulate a GIF using a procedural "Aperture Scan" and Fade
    for (int i = 0; i <= 100; i += 2) {
        int alpha = map(i, 0, 100, 0, 255);
        uint16_t cyan = gfx.color888(0, alpha, alpha);
        uint16_t slate = gfx.color888(0, alpha/4, alpha/2);

        // Draw "Digital Glitch" scanning lines
        if (i % 10 == 0) {
            gfx.drawFastHLine(0, random(gfx.height()), gfx.width(), slate);
        }
        
        // Center the Branding Text
        gfx.setFont(&fonts::FreeSansBold18pt7b);
        gfx.setTextColor(cyan, TFT_BLACK);
        gfx.drawString("EL-BASEET", gfx.width() / 2, gfx.height() / 2 - 20);
        
        // Subtle Sub-text
        gfx.setFont(&fonts::DejaVu12);
        gfx.setTextColor(slate, TFT_BLACK);
        gfx.drawString("SURGICAL INTERFACE V3", gfx.width() / 2, gfx.height() / 2 + 20);

        // Optional: If you had a JPEG logo array, you would call it here:
        // drawImage(logo_frames[i % total_frames], logo_size, x, y);

        delay(15);
    }

    // 3. Clear glitch lines and finalize static text
    gfx.fillScreen(TFT_BLACK);
    for(int i=0; i<=gfx.width(); i+=40) gfx.drawFastVLine(i, 0, gfx.height(), 0x10A2);
    for(int i=0; i<=gfx.height(); i+=40) gfx.drawFastHLine(0, i, gfx.width(), 0x10A2);
    
    gfx.setFont(&fonts::DejaVu12);
    gfx.setTextColor(0x07FF); // Surgical Cyan
    gfx.drawCenterString("EL-BASEET // SYSTEM BOOT", gfx.width() / 2, 110);
    
    // Audio Startup Chime (C5 -> E5 -> G5 -> C6)
    int notes[] = {523, 659, 784, 1046};
    for (int i = 0; i < 4; i++) {
        tone(BUZZER_PIN, notes[i], 150);
        delay(200);
    }
    noTone(BUZZER_PIN);
    
    // Initial Progress
    updateWelcomeProgress(5, "INITIALIZING NEURAL LINK...");
}

#endif