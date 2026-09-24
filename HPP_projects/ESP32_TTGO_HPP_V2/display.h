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
#include <TJpg_Decoder.h>

// Forward declaration for TJpg_Decoder rendering callback
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

// Animation Frame Counts
#define WELCOME_FRAMES 70
#define START_OP_FRAMES 68
#define STOP_OP_FRAMES 60
#define REPORT_FRAMES 25

// Animation Function Prototypes
void showWelcomeAnimation();
void showStartOperationAnimation();
void showStopOperationAnimation();
void showReportAnimation();

// Extern global references for local menu system
extern bool inMenuMode;
extern uint8_t currentMenuItem;
extern uint8_t webBaseSpeed;
extern bool oscillatingMode;
extern uint8_t motorVersion;
extern float internalVoltage;
extern float baselineCurrent;
extern float penetrationOffset;
extern float penetrationHysteresis;
extern bool calibrationNeeded;
extern bool systemEnabled;
extern unsigned long lastInteractionTime;
extern uint8_t ledBrightness;
extern void setLEDBrightness(uint8_t brightness);

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); // Create the Sprite object
TFT_eSprite clockSpr = TFT_eSprite(&tft); // Dedicated sprite for flicker-free clock panel

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

    // Create dedicated sprite for the right-hand clock panel (105x135)
    clockSpr.setColorDepth(16);
    clockSpr.createSprite(105, 135);

    s_oledInitialized = true;
}

/**
 * @brief Displays a professional splash screen for the system.
 */
void showWelcomeLogo() {
    if (!s_oledInitialized) return;

    spr.fillSprite(TFT_BLACK); // Use fillSprite to clear the buffer
    
    // Setup text alignment for easy centering
    spr.setTextDatum(MC_DATUM); // Middle-Center datum

    // 1. Branding - Main Title ("EL-BASEET")
    // Color matching #38bdf8 (Light Blue / Cyan)
    spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
    spr.setTextSize(3);
    spr.drawString("EL-BASEET", spr.width() / 2, 35, 1);

    // Decorative center line
    spr.drawFastHLine(20, 60, spr.width() - 40, TFT_DARKGREY);

    // 2. Subtitle ("HAIR PEN-V2")
    // Color matching white/light cyan
    spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
    spr.setTextSize(2);
    spr.drawString("HAIR PEN-V2", spr.width() / 2, 85, 1);

    // 3. Sub-subtitle ("SURGICAL ASSISTANT")
    // Color matching #94a3b8 (Light Grey)
    spr.setTextColor(tft.color565(148, 163, 184), TFT_BLACK);
    spr.setTextSize(1);
    spr.drawString("SURGICAL ASSISTANT", spr.width() / 2, 110, 2);

    // Reset datum for other functions
    spr.setTextDatum(TL_DATUM);

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
        // Draw battery casing and terminal (consistent with connected battery icon size)
        spr.drawRect(x, y, 22, 12, TFT_WHITE); // Body
        spr.fillRect(x + 22, y + 3, 3, 6, TFT_WHITE); // Terminal

        // Dynamic battery filling animation: Cycles from 20% to 100% every 1.5 seconds (300ms per step)
        uint8_t animPct = 20 + ((millis() / 300) % 5) * 20;
        int fillWidth = map(animPct, 0, 100, 0, 18);
        if (fillWidth > 0) {
            spr.fillRect(x + 2, y + 2, fillWidth, 8, TFT_GREEN);
        }

        // Draw CHG text in front of battery
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString("CHG", x - 32, y, 2);
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
        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK); // Cyan matching brand
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

    // --- LOCAL MENU MODE OVERRIDE ---
    if (inMenuMode) {
        spr.fillSprite(TFT_BLACK);
        spr.setTextSize(1); // Reset text size from screensaver to prevent giant menu text
        spr.setTextDatum(TL_DATUM); // Reset text datum to Top-Left for proper menu alignment
        
        // Draw Header
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString("SYSTEM MENU", 5, 5, 2);
        spr.drawFastHLine(0, 22, spr.width(), TFT_DARKGREY);

        const char* labels[] = {
            "1. Speed: ",
            "2. LED Light: ",
            "3. Mode: ",
            "4. Hardw: ",
            "5. Reset Grafts",
            "6. Exit Menu"
        };

        for (uint8_t i = 0; i < 6; i++) {
            int yPos = 26 + (i * 17);
            
            // Draw background highlight bar for the highlighted item
            if (i == currentMenuItem) {
                spr.fillRoundRect(2, yPos - 1, spr.width() - 4, 16, 3, TFT_BLUE);
                spr.setTextColor(TFT_WHITE, TFT_BLUE);
            } else {
                spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            }

            String text = labels[i];
            if (i == 0) {
                int percentSpeed = round((float)webBaseSpeed * 100.0f / 255.0f);
                text += String(percentSpeed) + "%";
            }
            else if (i == 1) {
                if (ledBrightness == 0) text += "OFF";
                else {
                    int percentLed = round((float)ledBrightness * 100.0f / 255.0f);
                    text += String(percentLed) + "%";
                }
            }
            else if (i == 2) text += (oscillatingMode ? "OSC" : "NORMAL");
            else if (i == 3) text += (motorVersion == 1 ? "V1-Enc" : "V2-Std");

            spr.drawString(text, 10, yPos, 2);
        }
        spr.pushSprite(0, 0);
        return; // Skip drawing normal telemetry screens
    }

    // Check if we should activate the elegant Animated screensaver.
    // Activated if the system is IDLE (no motor speed, system disabled, not in menu, and no interaction for > 15 seconds)
    bool inScreensaver = (!inMenuMode && !systemEnabled && abs(speed) <= 10 && (millis() - lastInteractionTime > 15000));
    static bool s_wasScreensaver = false;
    
    if (inScreensaver) {
        // Clear screen ONCE upon transition into screensaver mode (prevents flicker)
        if (!s_wasScreensaver) {
            tft.fillScreen(TFT_BLACK);
            s_wasScreensaver = true;
        }
        
        // 1. Play the animation frame from the hair frame set (0, 0 to 134, 134)
        static uint8_t s_savFrame = 0;
        s_savFrame = (s_savFrame + 1) % STOP_OP_FRAMES;
        
        TJpgDec.setJpgScale(1);
        TJpgDec.setSwapBytes(true);
        TJpgDec.setCallback(tft_output);
        TJpgDec.drawFsJpg(0, 0, "/hair_" + String(s_savFrame) + ".jpg");
        
        // 2. Render real-time clock and status into dedicated double-buffered sprite (Width: 105, Height: 135)
        clockSpr.fillSprite(TFT_BLACK);
        
        clockSpr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK); // Brand Cyan
        clockSpr.setTextDatum(TC_DATUM);
        clockSpr.drawString("EL-BASEET", 52, 12, 2);
        
        clockSpr.drawFastHLine(5, 30, 95, TFT_DARKGREY);
        
        struct tm timeinfo;
        bool hasTime = getLocalTime(&timeinfo, 5); // Short non-blocking timeout
        
        if (hasTime && timeinfo.tm_year >= 120) {
            // Live Digital Clock
            char timeStr[6]; // "HH:MM"
            strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
            clockSpr.setTextColor(TFT_WHITE, TFT_BLACK);
            clockSpr.drawString(timeStr, 52, 38, 4);
            
            // Blinking Seconds
            char secStr[4]; // ":SS"
            strftime(secStr, sizeof(secStr), ":%S", &timeinfo);
            clockSpr.setTextColor(tft.color565(14, 165, 233), TFT_BLACK);
            clockSpr.drawString(secStr, 52, 64, 2);
            
            // Abbreviated Date
            char dateStr[20]; // "Sep 19, 2026"
            strftime(dateStr, sizeof(dateStr), "%b %d, %Y", &timeinfo);
            clockSpr.setTextColor(TFT_YELLOW, TFT_BLACK);
            clockSpr.drawString(dateStr, 52, 84, 1);
        } else {
            // Standby Status
            clockSpr.setTextColor(TFT_GREEN, TFT_BLACK);
            if ((millis() / 500) % 2 == 0) {
                clockSpr.drawString("● READY", 52, 50, 2);
            } else {
                clockSpr.drawString("  READY", 52, 50, 2);
            }
            
            clockSpr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            clockSpr.drawString("SYS ACTIVE", 52, 72, 1);
        }
        
        clockSpr.drawFastHLine(5, 104, 95, TFT_DARKGREY);
        
        // Onscreen live battery telemetry
        clockSpr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        clockSpr.drawString("BAT: " + String(batPct) + "%", 52, 112, 1);
        
        // Push the entire right panel in ONE atomic DMA transfer (zero flicker)
        clockSpr.pushSprite(135, 0);
        return;
    } else {
        s_wasScreensaver = false;
    }

    // --- SHARED HEADER ZONE ---
    spr.setTextSize(1);
    spr.setTextDatum(TL_DATUM); // Top-Left datum
    bool anyTrip = currentTrip || voltageTrip;
    
    // Header Text
    spr.setCursor(5, 5);
    spr.setTextColor(tft.color565(56, 189, 248)); // Match Web Dashboard Cyan
    if (anyTrip) {
        // Flash the Alarm text if a trip is active
        if ((millis() / 500) % 2 == 0) spr.drawString("!!! ALARM !!!", 5, 5, 2);
    }
    else if (screen == 0) spr.drawString("SURGICAL DASH", 5, 5, 2);
    else if (screen == 1) spr.drawString("ENERGY STATUS", 5, 5, 2);
    else if (screen == 2) spr.drawString("NET STATUS", 5, 5, 2);
    else if (screen == 3) spr.drawString("OSCILLATION", 5, 5, 2);
    else if (screen == 4) spr.drawString("DIAGNOSTICS", 5, 5, 2);
    else spr.drawString("PATIENT INFO", 5, 5, 2);

    // Surgical LED Illumination Status Indicator in Header
    if (ledBrightness > 0) {
        uint8_t ledPct = round((float)ledBrightness * 100.0f / 255.0f);
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString("LED:" + String(ledPct) + "%", 118, 5, 2);
    }

    // Debug Indicator (Inverted Tag) - Flashing every 500ms for high visibility
    if (debug && (millis() / 500) % 2 == 0) {
        spr.fillRoundRect(spr.width() - 75, 3, 40, 16, 3, TFT_YELLOW);
        spr.setTextColor(TFT_BLACK, TFT_YELLOW);
        spr.drawString("DBG", spr.width() - 70, 5, 2);
    }
    
    // Power Status
    spr.setTextDatum(TR_DATUM); // Top-Right datum
    drawBatteryIcon(spr, isBatConnected, isCharging, batPct);

    spr.drawFastHLine(0, 22, spr.width(), TFT_DARKGREY);
    spr.setTextDatum(TL_DATUM); // Reset datum

    if (anyTrip && !debug) {
        // --- ALARM OVERRIDE ---
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(TFT_RED, TFT_BLACK);
        spr.drawString(currentTrip ? "OVERCURRENT" : "LOW BATTERY", spr.width()/2, 50, 4);
        spr.drawString("SYSTEM HALTED", spr.width()/2, 85, 4);
        spr.pushSprite(0, 0); // Push the alarm message to the screen
        return; // Skip drawing normal telemetry
    }

    if (screen == 0) {
        // --- SCREEN 0: DRIVE DASHBOARD (REDESIGNED WITH ROTATING MOTOR ANIMATION) ---
        // Left Column: Telemetry readings
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("SET SPEED:", 10, 32, 2);
        spr.drawString("ACT RPM:", 10, 68, 2);
        
        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK); // Cyberpunk Cyan
        int percentSpeed = round((float)abs(speed) * 100.0f / 255.0f);
        spr.drawString(String(percentSpeed) + "%", 90, 30, 4);
        
        spr.setTextColor(TFT_GREEN, TFT_BLACK); // Neon Green
        spr.drawString(String(rpm), 90, 66, 4);
        
        // Motor Status Text
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("STATUS:", 10, 95, 2);
        if (speed != 0) {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString(oscillatingMode ? "OSCILLATING" : "RUNNING", 70, 95, 2);
        } else {
            spr.setTextColor(TFT_YELLOW, TFT_BLACK);
            spr.drawString("STANDBY", 70, 95, 2);
        }

        // Beautiful Spinning Motor Animation (Right Column)
        int cx = 190;
        int cy = 68;
        int r = 24;
        
        // Casing and outer ring
        spr.drawCircle(cx, cy, r + 4, TFT_DARKGREY);
        spr.drawCircle(cx, cy, r, tft.color565(71, 85, 105)); // Slate Gray Casing
        
        // Spinning Angle Calculation proportional to motor speed
        static float currentAngle = 0;
        if (speed != 0) {
            currentAngle += (speed / 255.0f) * 12.0f; // Multiplier defines spinning speed
            if (currentAngle >= 360.0f) currentAngle -= 360.0f;
            if (currentAngle < 0.0f) currentAngle += 360.0f;
        }
        
        // Draw 4 spokes
        for (int i = 0; i < 4; i++) {
            float rad = (currentAngle + (i * 90)) * DEG_TO_RAD;
            int x_end = cx + (int)(r * cos(rad));
            int y_end = cy + (int)(r * sin(rad));
            uint16_t spokeColor = (speed != 0) ? tft.color565(34, 197, 94) : TFT_DARKGREY; // Green if turning, dark grey if idle
            spr.drawLine(cx, cy, x_end, y_end, spokeColor);
        }
        
        // Center spindle
        spr.fillCircle(cx, cy, 5, (speed != 0) ? TFT_YELLOW : TFT_LIGHTGREY);
        
        // Direction Labels above the animation
        if (speed > 0) {
            spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
            spr.drawString("FWD CW ↻", cx - 25, 28, 1);
        } else if (speed < 0) {
            spr.setTextColor(TFT_ORANGE, TFT_BLACK);
            spr.drawString("REV CCW ↺", cx - 25, 28, 1);
        } else {
            spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
            spr.drawString("STOPPED", cx - 22, 28, 1);
        }

        // FOOTER ZONE
        spr.drawFastHLine(0, 115, spr.width(), TFT_DARKGREY); 
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString("GRAFTS COUNT: " + String(count), 5, 118, 2);

        if (speed != 0) {
            spr.setTextColor(speed > 0 ? tft.color565(56, 189, 248) : TFT_ORANGE, TFT_BLACK); 
            spr.setTextDatum(TR_DATUM);
            // Dynamic moving chevron frames
            static uint8_t arrowFrame = 0;
            arrowFrame = (arrowFrame + 1) % 4;
            String arrowStr = "";
            if (speed > 0) {
                for (int a = 0; a < 3; a++) arrowStr += (a == arrowFrame) ? ">" : "»";
                spr.drawString("FWD " + arrowStr, spr.width() - 5, 118, 2);
            } else {
                for (int a = 0; a < 3; a++) arrowStr += (a == (3 - arrowFrame)) ? "<" : "«";
                spr.drawString(arrowStr + " REV", spr.width() - 5, 118, 2);
            }
        } else {
            spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            spr.setTextDatum(TR_DATUM);
            spr.drawString("IDLE", spr.width() - 5, 118, 2);
        }

    } else if (screen == 1) {
        // --- SCREEN 1: ENERGY STATUS (REDESIGNED GRID) ---
        spr.drawFastVLine(120, 25, 65, TFT_DARKGREY);
        
        // Left Column: BATTERY
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("BATTERY:", 10, 30, 2);
        spr.drawString("Voltage:", 10, 50, 2);
        spr.drawString("Percent:", 10, 70, 2);
        
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(volt, 2) + " V", 70, 50, 2);
        spr.drawString(String(batPct) + " %", 70, 70, 2);

        // Right Column: LIVE SYSTEM LOAD
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("SYS LOAD:", 130, 30, 2);
        spr.drawString("Current:", 130, 50, 2);
        spr.drawString("Power:", 130, 70, 2);

        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
        spr.drawString(String(amp, 3) + " A", 195, 50, 2);
        spr.drawString(String(pwr, 1) + " W", 195, 70, 2);
        
        // Lower Zone: Charging/Discharging status bar
        spr.drawFastHLine(0, 92, spr.width(), TFT_DARKGREY);
        spr.setTextDatum(MC_DATUM);
        if (isCharging) {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("⚡ CHARGING SYSTEM (USB)", spr.width() / 2, 105, 2);
        } else {
            spr.setTextColor(TFT_ORANGE, TFT_BLACK);
            spr.drawString("🔋 DISCHARGING (BATTERY)", spr.width() / 2, 105, 2);
        }

    } else if (screen == 2) {
        // --- SCREEN 2: NETWORK STATUS (REDESIGNED NON-BLOCKING WITH URL) ---
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("SSID/Mode:", 20, 30, 2);
        spr.drawString("IP Address:", 20, 52, 2);
        spr.drawString("Web URL:", 20, 74, 2);
        spr.drawString("Active Clients:", 20, 96, 2);

        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
        
        String ssidName = "AP Mode (El-Baseet)";
        if (WiFi.status() == WL_CONNECTED) {
            ssidName = WiFi.SSID();
        } else if (wifiConnecting) {
            ssidName = "Connecting...";
        }
        
        if (ssidName.length() > 16) {
            ssidName = ssidName.substring(0, 14) + "..";
        }
        
        spr.drawString(ssidName, 120, 30, 2);
        spr.drawString(ip, 120, 52, 2);
        spr.drawString("elbaseet.local", 120, 74, 2);
        spr.drawString(String(clients) + " Connected", 120, 96, 2);
        
        // Signal Quality representation at the bottom
        if (WiFi.status() == WL_CONNECTED) {
            int32_t rssi = WiFi.RSSI();
            String sigStr = String(rssi) + " dBm ";
            if (rssi >= -50) { spr.setTextColor(TFT_GREEN, TFT_BLACK); sigStr += "Excellent"; }
            else if (rssi >= -70) { spr.setTextColor(TFT_YELLOW, TFT_BLACK); sigStr += "Good"; }
            else { spr.setTextColor(TFT_ORANGE, TFT_BLACK); sigStr += "Weak"; }
            spr.drawString("Signal: " + sigStr, 20, 116, 1);
        } else {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("Signal: 100% (AP)", 20, 116, 1);
        }
        
    } else if (screen == 3) {
        // --- SCREEN 3: OSCILLATION SETTINGS (REDESIGNED) ---
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("Drive Mode:", 15, 32, 2);
        spr.drawString("CW Duration:", 15, 57, 2);
        spr.drawString("CCW Duration:", 15, 82, 2);
        spr.drawString("Total Period:", 15, 107, 2);

        if (oscMode) {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("OSCILLATING Mode", 125, 32, 2);
        } else {
            spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            spr.drawString("CONTINUOUS Mode", 125, 32, 2);
        }

        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
        spr.drawString(String(oscCW) + " ms", 125, 57, 2);
        spr.drawString(String(oscCCW) + " ms", 125, 82, 2);
        
        uint32_t totalPeriod = oscCW + oscCCW;
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(totalPeriod) + " ms", 125, 107, 2);
        
    } else if (screen == 4) {
        // --- SCREEN 4: DIAGNOSTICS (REDESIGNED GRID) ---
        spr.drawFastVLine(120, 25, 65, TFT_DARKGREY);
        
        // Left Column: CALIBRATION
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("CALIBRATION:", 10, 30, 2);
        spr.drawString("Base I:", 10, 50, 2);
        spr.drawString("Status:", 10, 70, 2);
        
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString(String(baselineCurrent, 3) + " A", 70, 50, 2);
        if (calibrationNeeded) {
            spr.setTextColor(TFT_RED, TFT_BLACK);
            spr.drawString("CALIB", 70, 70, 2);
        } else {
            spr.setTextColor(TFT_GREEN, TFT_BLACK);
            spr.drawString("STABLE", 70, 70, 2);
        }

        // Right Column: THRESHOLDS
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("THRESHOLDS:", 130, 30, 2);
        spr.drawString("Spike I:", 130, 50, 2);
        spr.drawString("Reset I:", 130, 70, 2);

        spr.setTextColor(TFT_ORANGE, TFT_BLACK);
        spr.drawString(String(baselineCurrent + penetrationOffset, 3) + " A", 190, 50, 2);
        spr.drawString(String((baselineCurrent + penetrationOffset) - penetrationHysteresis, 3) + " A", 190, 70, 2);
        
        // Bottom Sensitivity bar
        spr.drawFastHLine(0, 92, spr.width(), TFT_DARKGREY);
        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
        spr.drawString("Sens Offset: " + String(penetrationOffset, 3) + "A | Hyst: " + String(penetrationHysteresis, 3) + "A", 10, 98, 2);
        
        // LED Illumination Telemetry
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        uint8_t ledDiagPct = round((float)ledBrightness * 100.0f / 255.0f);
        spr.drawString("LED Pin 17: " + String(ledDiagPct) + "% | PWM: " + String(ledBrightness) + " / 255", 10, 118, 1);
        
    } else {
        // --- SCREEN 5: PATIENT INFO (REDESIGNED) ---
        spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        spr.drawString("Active Patient Name:", 15, 32, 2);
        spr.drawString("Patient Age / Gen:", 15, 62, 2);
        spr.drawString("Patient Nationality:", 15, 92, 2);

        spr.setTextColor(tft.color565(56, 189, 248), TFT_BLACK);
        String shortName = pName;
        if (shortName.length() > 18) {
            shortName = shortName.substring(0, 16) + "..";
        }
        spr.drawString(shortName, 150, 32, 2);
        spr.drawString(pAge, 150, 62, 2);
        spr.drawString(pNat, 150, 92, 2);
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