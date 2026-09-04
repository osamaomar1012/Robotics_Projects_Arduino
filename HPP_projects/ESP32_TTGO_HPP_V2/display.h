/**
 * MODULE: Integrated TTGO T-Display (ST7789)
 * DESCRIPTION: This module handles the initialization and UI drawing routines
 *              for the 1.14" ST7789 color TFT display on the TTGO T-Display board.
 *              It uses the TFT_eSPI library.
 * DEPENDENCIES: TFT_eSPI.h, SPI.h
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Create the Sprite object

// --- CONFIRMATION MESSAGE STATE ---
static String s_confirmationMessage = "";
static unsigned long s_confirmationTimer = 0;
static bool s_oledInitialized = false;
#define CONFIRMATION_DURATION 2000 // Display message for 2 seconds

// --- GRAFT FLASH MESSAGE STATE ---
static String s_flashMessage = "";
static unsigned long s_flashTimer = 0;
#define CONFIRMATION_DURATION 2000 // Display message for 2 seconds

/**
 * @brief Initializes the ST7789 TFT display on the TTGO T-Display.
 * 
 * This function initializes the TFT_eSPI library, sets the screen rotation,
 * and clears the display to a default background color.
 * NOTE: Ensure your TFT_eSPI library is configured for the TTGO T-Display
 * in the User_Setup.h file.
 */
void setupOLED() {
    tft.init();
    tft.setRotation(1); // Use 1 or 3 for landscape orientation
    tft.fillScreen(TFT_BLACK);

    // Create a 16-bit color sprite the size of the screen
    // All drawing will be done on this sprite, then pushed to the screen
    spr.setColorDepth(16);
    spr.createSprite(tft.width(), tft.height());
    s_oledInitialized = true;
}

/**
 * @brief Displays a professional splash screen for the system.
 */
void showWelcomeLogo() {
    if (!s_oledInitialized) return;

    spr.fillSprite(TFT_BLACK); // Use fillSprite to clear the buffer
    spr.setTextColor(TFT_CYAN, TFT_BLACK);

    // Branding - Main Title
    spr.setTextSize(3);
    spr.setCursor(40, 25);
    spr.println(F("EL-BASEET"));

    // Decorative line
    spr.drawFastHLine(20, 60, spr.width() - 40, TFT_DARKGREY);

    // Subtitle
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextSize(2);
    spr.setCursor(25, 75);
    spr.println(F("HAIR PEN V2.0"));

    // Push the completed sprite to the screen at coordinate 0,0
    spr.pushSprite(0, 0);
}

/**
 * @brief Draws a dynamic battery icon in the top-right corner of the sprite.
 * 
 * This function handles drawing a battery icon that reflects the current charge
 * state. It shows a static, color-coded level when on battery power and a
 * cycling animation when charging.
 * 
 * @param spr A reference to the TFT_eSprite object to draw on.
 * @param isConnected True if a battery is detected.
 * @param isCharging True if the device is plugged in and charging.
 * @param batPct The current battery percentage (0-100).
 */
void drawBatteryIcon(TFT_eSprite &spr, bool isConnected, bool isCharging, uint8_t batPct) {
    int16_t x = spr.width() - 28; // Top-right corner X position
    int16_t y = 4;                // Top-right corner Y position

    if (isCharging) {
        // Draw charging bolt icon
        spr.setTextColor(TFT_GREEN);
        spr.drawString("CHG", x - 30, y, 2);
        spr.fillTriangle(x + 13, y + 1, x + 7, y + 7, x + 10, y + 7, TFT_YELLOW);
        spr.fillTriangle(x + 10, y + 7, x + 16, y + 1, x + 13, y + 1, TFT_YELLOW);
        spr.fillTriangle(x + 7, y + 6, x + 13, y + 12, x + 10, y + 12, TFT_YELLOW);
        spr.fillTriangle(x + 10, y + 12, x + 4, y + 6, x + 7, y + 6, TFT_YELLOW);
    } else if (isConnected) {
        // Draw static battery level icon
        spr.setTextColor(TFT_WHITE);
        spr.drawString(String(batPct) + "%", x - 8, y, 2);
        spr.drawRect(x, y, 22, 12, TFT_WHITE); // Body
        spr.fillRect(x + 22, y + 3, 3, 6, TFT_WHITE); // Terminal

        uint16_t barColor = (batPct < 20) ? TFT_RED : (batPct < 50) ? TFT_ORANGE : TFT_GREEN;
        int fillWidth = map(batPct, 0, 100, 0, 18);
        if (fillWidth > 0) {
            spr.fillRect(x + 2, y + 2, fillWidth, 8, barColor);
        }
    }
}

/**
 * @brief Refreshes the OLED display with current motor and battery telemetry.
 * 
 * This function clears the display, draws the UI elements, and updates the
 * displayed values for motor speed, battery voltage, and battery current.
 * 
 * @param speed Current PWM duty cycle (0-255) for the motor.
 * @param volt Measured battery voltage (e.g., 3.85V).
 * @param amp Measured current draw in Amperes (e.g., 0.25A).
 * @param currentTrip Safety flag indicating an overcurrent event.
 * @param voltageTrip Safety flag indicating low battery voltage.
 * @param rpm The calculated estimated RPM of the motor.
 * @param pwr The calculated power consumption in Watts.
 * @param isBatConnected Flag for battery presence.
 * @param batPct Battery charge percentage.
 * @param isCharging Flag for charging status.
 * @param screen The active screen index (0: Drive, 1: Energy, 2: Net Status).
 * @param ip The Access Point IP address.
 * @param clients Number of connected WiFi stations.
 * @param debug Flag for engineering debug mode.
 * @param wifiConnecting Flag indicating if STA connection is in progress.
 * @param oscMode Flag indicating if oscillation mode is active.
 * @param oscCW The duration for clockwise rotation in ms.
 * @param oscCCW The duration for counter-clockwise rotation in ms.
 */
void refreshOLED(int16_t speed, float volt, float amp, bool currentTrip, bool voltageTrip, uint32_t rpm, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint8_t screen, String ip, uint8_t clients, bool debug, uint32_t count, bool wifiConnecting, bool oscMode, uint32_t oscCW, uint32_t oscCCW, String pName, String pAge, String pNat) {
    spr.fillSprite(TFT_BLACK); // Clear the sprite buffer, not the screen
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    
    if (millis() - s_confirmationTimer < CONFIRMATION_DURATION) {
        spr.setTextDatum(MC_DATUM); // Middle-Center datum
        spr.setTextSize(2);
        spr.drawString(s_confirmationMessage, spr.width() / 2, spr.height() / 2);
        spr.pushSprite(0, 0); // Push the confirmation message to the screen
        return; // Skip normal screen drawing
    }

    if (millis() - s_flashTimer < 200) { // Display flash for 200ms
        spr.fillSprite(TFT_WHITE); // White background
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(TFT_BLACK); // Black text
        spr.drawString(s_flashMessage, spr.width() / 2, spr.height() / 2, 4);
        spr.pushSprite(0, 0);
        return; // Skip normal screen drawing
    }

    // --- SHARED HEADER ZONE ---
    spr.setTextSize(1);
    spr.setTextDatum(TL_DATUM); // Top-Left datum
    bool anyTrip = currentTrip || voltageTrip;
    
    // Header Text
    spr.setCursor(5, 5);
    spr.setTextColor(TFT_CYAN);
    if (anyTrip) {
        // Flash the Alarm text if a trip is active
        if ((millis() / 500) % 2 == 0) spr.drawString("!!! ALARM !!!", 5, 5, 2);
    }
    else if (screen == 0) spr.drawString("SURGICAL DASH", 5, 5, 2);
    else if (screen == 1) spr.drawString("ENERGY STATUS", 5, 5, 2);
    else if (screen == 2) spr.drawString("NET STATUS", 5, 5, 2);
    else if (screen == 3) spr.drawString("OSCILLATION", 5, 5, 2);
    else spr.drawString("PATIENT INFO", 5, 5, 2);

    // Debug Indicator (Inverted Tag) - Flashing every 500ms for high visibility
    if (debug && (millis() / 500) % 2 == 0) {
        spr.fillRoundRect(spr.width() - 55, 3, 50, 16, 3, TFT_YELLOW);
        spr.setTextColor(TFT_BLACK);
        spr.drawString("DEBUG", spr.width() - 50, 5, 2);
    }
    
    // Power Status
    spr.setTextDatum(TR_DATUM); // Top-Right datum
    drawBatteryIcon(spr, isBatConnected, isCharging, batPct);

    spr.drawFastHLine(0, 22, spr.width(), TFT_DARKGREY);
    spr.setTextDatum(TL_DATUM); // Reset datum

    if (anyTrip && !debug) {
        // --- ALARM OVERRIDE ---
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(TFT_RED);
        spr.drawString(currentTrip ? "OVERCURRENT" : "LOW BATTERY", spr.width()/2, 50, 4);
        spr.drawString("SYSTEM HALTED", spr.width()/2, 85, 4);
        spr.pushSprite(0, 0); // Push the alarm message to the screen
        return; // Skip drawing normal telemetry
    }

    if (screen == 0) {
        // --- SCREEN 0: DRIVE DASHBOARD ---
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("SPD", 20, 40, 2);
        spr.drawString("RPM", 20, 80, 2);
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(abs(speed)), 80, 40, 4);
        spr.drawString(String(rpm), 80, 80, 4);

        // FOOTER ZONE
        spr.drawFastHLine(0, 115, spr.width(), TFT_DARKGREY); 
        if (pName != "N/A") {
            spr.setTextColor(TFT_WHITE, TFT_BLACK);
            spr.drawString(pName, 5, 120, 2);
        } else {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("GRAFT: " + String(count), 5, 120, 2);
        }

        spr.setTextColor(speed >= 0 ? TFT_CYAN : TFT_ORANGE, TFT_BLACK); 
        spr.setTextDatum(TR_DATUM);
        spr.drawString(speed >= 0 ? "FWD" : "REV", spr.width() - 5, 120, 2);

    } else if (screen == 1) {
        // --- SCREEN 1: ENERGY STATUS ---
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("Voltage:", 20, 35, 2);
        spr.drawString("Current:", 20, 65, 2);
        spr.drawString("Power:",   20, 95, 2);
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(volt, 2) + " V", 120, 35, 2);
        spr.drawString(String(amp, 2) + " A", 120, 65, 2);
        spr.drawString(String(pwr, 1) + " W", 120, 95, 2);

        // Battery Bar
        uint8_t barWidth = spr.width() - 10;
        spr.drawRect(5, 120, barWidth, 12, TFT_WHITE);
        uint8_t fillWidth = map(batPct, 0, 100, 0, barWidth - 2);
        uint16_t barColor = (batPct < 20) ? TFT_RED : (batPct < 50) ? TFT_ORANGE : TFT_GREEN;
        spr.fillRect(6, 121, fillWidth, 10, barColor);

    } else if (screen == 2) {
        // --- SCREEN 2: NETWORK STATUS ---
        if (wifiConnecting) {
            spr.setTextDatum(MC_DATUM);
            spr.drawString("WiFi Connecting...", spr.width()/2, spr.height()/2, 2);
            spr.pushSprite(0, 0); // Push the connecting message
            return;
        }
        
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("Mode:", 20, 35, 2);
        spr.drawString("IP:", 20, 65, 2);
        spr.drawString("Users:", 20, 95, 2);

        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        String mode = (WiFi.status() == WL_CONNECTED) ? "STA" : "AP";
        spr.drawString(mode, 120, 35, 2);
        spr.drawString(ip, 120, 65, 2);
        spr.drawString(String(clients), 120, 95, 2);
    } else if (screen == 3) {
        // --- SCREEN 3: OSCILLATION SETTINGS ---
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("Mode:", 20, 35, 2);
        spr.drawString("CW Time:", 20, 65, 2);
        spr.drawString("CCW Time:", 20, 95, 2);

        // Status
        if (oscMode) {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("ACTIVE", 140, 35, 2);
        } else {
            spr.setTextColor(TFT_ORANGE, TFT_BLACK);
            spr.drawString("INACTIVE", 140, 35, 2);
        }
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(oscCW) + " ms", 140, 65, 2);
        spr.drawString(String(oscCCW) + " ms", 140, 95, 2);
    } else {
        // --- SCREEN 4: PATIENT INFO ---
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString("Name:", 20, 35, 2);
        spr.drawString("Age:", 20, 65, 2);
        spr.drawString("Nat.:", 20, 95, 2);
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(pName, 120, 35, 2);
        spr.drawString(pAge, 120, 65, 2);
        spr.drawString(pNat, 120, 95, 2);
    }

    // Finally, push the completed sprite to the screen
    spr.pushSprite(0, 0);
}

/**
 * @brief Sets a confirmation message to be displayed on the OLED.
 * @param msg The message string to display.
 */
void displayWebConfirmation(String msg) {
    s_confirmationMessage = msg;
    s_confirmationTimer = millis();
}

/**
 * @brief Sets a confirmation message specifically for graft detection.
 * @param count The new graft count to display.
 */
void displayGraftFlash(uint32_t count) {
    s_flashMessage = String(count);
    s_flashTimer = millis();
}

#endif // DISPLAY_H