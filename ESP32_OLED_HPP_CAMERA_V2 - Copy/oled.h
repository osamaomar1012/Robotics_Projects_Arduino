/**
 * MODULE: Integrated OLED Display (SSD1306)
 * DESCRIPTION: This module handles the initialization and user interface (UI)
 *              drawing routines for the 0.96" SSD1306 OLED display integrated
 *              on the TTGO ESP32 board. It uses the Adafruit GFX and SSD1306
 *              libraries for display control.
 * DEPENDENCIES: Wire.h, Adafruit_GFX.h, Adafruit_SSD1306.h
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef OLED_H
#define OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BoardConfig.h"

/* 
 * -------------------------------------------------------------------
 * WIRING DIAGRAM: SSD1306 OLED (I2C)
 * -------------------------------------------------------------------
 */

#define HAS_DISPLAY   true
#define SCREEN_WIDTH  128 
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C
#define OLED_RESET    PIN_OLED_RST

// --- ENHANCED UI ASSETS ---
// Large Icons (24x24) for Primary Data
// NOTE: A 24x24 1-bit bitmap requires (24 * 24) / 8 = 72 bytes.
// The original data might have been malformed or truncated.
// Replacing with a simple solid square for testing stability.
// If this works, generate your desired 24x24 icon using an online tool (e.g., image2cpp.com).
static const unsigned char PROGMEM icon_speed_large[] __attribute__((aligned(4))) = {
  0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x01, 0xff, 0x80, 0x07, 0xc3, 0xe0, 0x0f, 0x00, 0xf0, 0x1e, 
  0x00, 0x78, 0x3c, 0x00, 0x3c, 0x38, 0x00, 0x1c, 0x70, 0x00, 0x0e, 0x70, 0x18, 0x0e, 0xe0, 0x3c, 
  0x07, 0xe0, 0x7e, 0x07, 0xe0, 0xff, 0x07, 0xe1, 0xff, 0x87, 0xe3, 0xff, 0xc7, 0xe7, 0x81, 0xe7, 
  0x70, 0x00, 0x0e, 0x70, 0x00, 0x0e, 0x38, 0x00, 0x1c, 0x1c, 0x00, 0x38, 0x0f, 0xff, 0xf0, 0x07, 
  0xff, 0xe0, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Total 72 bytes padded
};

// This icon is defined but currently not used in refreshOLED.
// If you intend to use it, ensure it is also a correctly formatted 24x24 bitmap (72 bytes).
static const unsigned char PROGMEM icon_bolt_large[] __attribute__((aligned(4))) = {
  0x00, 0x18, 0x00, 0x00, 0x38, 0x00, 0x00, 0x70, 0x00, 0x00, 0xe0, 0x00, 0x01, 0xc0, 0x00, 0x03,
  0x80, 0x00, 0x07, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0x3f, 0xff, 0xe0, 0x00, 0x38,
  0x00, 0x00, 0x70, 0x00, 0x00, 0xe0, 0x00, 0x01, 0xc0, 0x00, 0x03, 0x80, 0x00, 0x07, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Total 72 bytes
};

// Small Helper Icons (8x8)
static const unsigned char PROGMEM icon_bolt_small[] = { 0x0C, 0x18, 0x30, 0x7E, 0x0C, 0x18, 0x30, 0x00 };
static const unsigned char PROGMEM icon_wave_small[] = { 0x70, 0x88, 0x88, 0x08, 0x08, 0x88, 0x88, 0x07 };
static const unsigned char PROGMEM icon_bat_small[]  = { 0x08, 0x1C, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00 };
static const unsigned char PROGMEM icon_bat_frames[5][8] = {
  { 0x08, 0x1C, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00 }, // Empty
  { 0x08, 0x1C, 0x22, 0x22, 0x22, 0x3E, 0x1C, 0x00 }, // 25%
  { 0x08, 0x1C, 0x22, 0x22, 0x3E, 0x3E, 0x1C, 0x00 }, // 50%
  { 0x08, 0x1C, 0x22, 0x3E, 0x3E, 0x3E, 0x1C, 0x00 }, // 75%
  { 0x08, 0x1C, 0x3E, 0x3E, 0x3E, 0x3E, 0x1C, 0x00 }  // Full
};
static const unsigned char PROGMEM icon_plug_small[] = { 0x44, 0x44, 0x7C, 0x7C, 0x10, 0x10, 0x10, 0x00 };
static const unsigned char PROGMEM icon_arrow_fwd[]  = { 0x10, 0x38, 0x7E, 0x38, 0x10, 0x00, 0x00, 0x00 };
static const unsigned char PROGMEM icon_arrow_rev[]  = { 0x08, 0x1C, 0x7F, 0x1C, 0x08, 0x00, 0x00, 0x00 };

static const unsigned char PROGMEM icon_gear[] = { 0x3C, 0x42, 0x99, 0xBD, 0xBD, 0x99, 0x42, 0x3C };
static const unsigned char PROGMEM icon_gear_r[] = { 0x18, 0x3C, 0x66, 0xDB, 0xDB, 0x66, 0x3C, 0x18 };
static const unsigned char PROGMEM icon_gear_3[] = { 0x3C, 0x5A, 0xA5, 0xC3, 0xC3, 0xA5, 0x5A, 0x3C };
static const unsigned char PROGMEM icon_pwr[]  = { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x18 };

#if HAS_DISPLAY
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// --- CONFIRMATION MESSAGE STATE ---
static String s_confirmationMessage = "";
extern volatile bool isStreaming; // Linked to web_server.h
static unsigned long s_confirmationTimer = 0;
static unsigned long s_graftFeedbackTimer = 0;
static bool s_oledInitialized = false;
#define CONFIRMATION_DURATION 2000 // Display message for 2 seconds
#define GRAFT_FEEDBACK_DURATION 1000 // Display graft hit for 1 second

/**
 * @brief Initializes the SSD1306 OLED display.
 * 
 * This function performs the necessary steps to power on and configure the OLED:
 * - Toggles the OLED_RESET pin (GPIO 16) as required by TTGO boards.
 * - Initializes the I2C communication using specified SDA/SCL pins.
 * - Calls the display.begin() method to allocate memory and start the display.
 * - Sets the display rotation and clears the buffer.
 */
void setupOLED() {
#if HAS_DISPLAY
    // Toggle Reset pin if defined
    if (PIN_OLED_RST != -1) {
        pinMode(PIN_OLED_RST, OUTPUT);
        digitalWrite(PIN_OLED_RST, LOW);
        delay(50);
        digitalWrite(PIN_OLED_RST, HIGH);
    }

    // Centralized I2C pins
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    
    // Give the hardware a moment to stabilize after reset
    delay(100);
    Wire.setClock(400000); // Increase to 400kHz for faster loop execution

    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        // If display initialization fails, print an error and halt execution.
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }

    s_oledInitialized = true;
    display.setRotation(0); // Set display orientation (0 = default, 2 = 180 degrees flipped)
    display.clearDisplay(); 
    display.setTextColor(SSD1306_WHITE);
    display.display();
#endif
}

/**
 * @brief Displays a professional splash screen for the system.
 */
void showWelcomeLogo() {
#if HAS_DISPLAY
    if (!s_oledInitialized) return;

    // Simple "Wipe" Animation
    for (int x = 0; x < 130; x += 10) {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);

        // Decorative stylized box
        display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
        display.drawLine(0, 50, 128, 50, SSD1306_WHITE);
        
        // Branding - Stylized Logo
        display.setTextSize(2);
        display.setCursor(10, 12);
        display.print(F("EL-BASEET"));
        
        // Animate a scanning line
        display.drawLine(x, 10, x, 30, SSD1306_WHITE);

        // Subtitles
        display.setTextSize(1);
        display.setCursor(12, 35);
        display.print(F("HAIR PLANTING PEN"));
        
        display.setCursor(55, 54);
        display.print(F("V2.0"));

        display.display();
        delay(50);
    }
#endif
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
 * @param pwr The calculated power consumption in Watts.
 * @param isBatConnected Flag for battery presence.
 * @param batPct Battery charge percentage.
 * @param isCharging Flag for charging status.
 * @param screen The active screen index (0: Drive, 1: Energy, 2: Net Status).
 * @param ip The Access Point IP address.
 * @param clients Number of connected WiFi stations.
 * @param debug Flag for engineering debug mode.
 * @param wifiConnecting Flag indicating if STA connection is in progress.
 * @param camAvailable Flag indicating if the camera was successfully initialized.
 */
void refreshOLED(int16_t speed, float volt, float amp, bool currentTrip, bool voltageTrip, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint8_t screen, String ip, uint8_t clients, bool debug, uint32_t count, bool wifiConnecting, bool camAvailable) {
#if HAS_DISPLAY
    display.clearDisplay(); // Clear the display buffer before drawing new content
    display.setTextColor(SSD1306_WHITE); // Set text color to white
    
    if (millis() - s_confirmationTimer < CONFIRMATION_DURATION) {
        display.setTextSize(2);
        display.setCursor(0, 25);
        display.print(s_confirmationMessage);
        display.display();
        return; // Skip normal screen drawing
    }

    // --- SHARED HEADER ZONE (Y: 0-11) ---
    display.setTextSize(1);
    bool anyTrip = currentTrip || voltageTrip;
    
    // Header Text
    display.setCursor(0, 0);
    if (anyTrip) {
        // Flash the Alarm text if a trip is active
        if ((millis() / 500) % 2 == 0) display.print(F("!!! ALARM !!!"));
    }
    else if (screen == 0) display.print(F("SURGICAL DASH"));
    else if (screen == 1) display.print(F("ENERGY STATUS"));
    else display.print(F("NET STATUS"));

    // Camera Hardware Warning (Flashing)
    if (!camAvailable && (millis() / 1000) % 2 == 0) {
        display.setCursor(78, 0);
        display.print(F("!CAM"));
    }

    // Debug Indicator (Inverted Tag) - Flashing every 500ms for high visibility
    if (debug && (millis() / 500) % 2 == 0) {
        display.fillRect(82, 0, 35, 9, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(84, 1); display.print(F("DEBUG"));
        display.setTextColor(SSD1306_WHITE);
    }
    
    // Power/Battery Status (Top Right Corner)
    if (isCharging) {
        display.setCursor(110, 0);
        display.print(F("CHG"));
    } else if (isBatConnected) {
        // Consolidated right-aligned battery percentage to prevent overlapping
        int xOffset = (batPct == 100) ? 104 : (batPct >= 10) ? 110 : 116;
        display.setCursor(xOffset, 0);
        display.print(batPct); display.print(F("%"));
    } else {
        display.setCursor(110, 0);
        display.print(F("USB"));
    }

    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    if (anyTrip && !debug) {
        // --- ALARM OVERRIDE (Y: 12-63) ---
        display.setTextSize(1);
        display.setCursor(0, 20);
        if (currentTrip) display.println(F("ALARM: OVERCURRENT"));
        else display.println(F("ALARM: LOW BATT"));
        display.println(F("SYSTEM HALTED!"));
        display.display();
        return; // Skip drawing normal telemetry
    }

    // --- GRAFT DETECTION POP-UP (Y: 12-49) ---
    // Briefly show a large counter when a graft is detected
    if (millis() - s_graftFeedbackTimer < GRAFT_FEEDBACK_DURATION) {
        display.setTextSize(1);
        display.setCursor(35, 18);
        display.print(F("GRAFT DETECTED"));
        display.setTextSize(3);
        display.setCursor(50, 28);
        display.print(count);
        display.display();
        return;
    }

    if (screen == 0) {
        // --- SCREEN 0: SURGICAL DASHBOARD (Graft-Centric) ---
        // Speed Column (Left)
        display.drawBitmap(2, 18, icon_speed_large, 24, 24, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(30, 18); display.print(F("SPD"));
        display.setTextSize(2);
        display.setCursor(30, 28); display.print(abs(speed));

        // Graft Column (Right - Emphasis)
        display.setTextSize(1);
        display.setCursor(80, 18); display.print(F("GRAFTS"));
        display.setTextSize(3);
        // Simple right-alignment adjustment
        uint8_t countX = (count < 10) ? 95 : (count < 100) ? 82 : 70;
        display.setCursor(countX, 28); 
        display.print(count);

        // Footer Section
        display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(2, 56); 
        display.print(F("MODE: "));
        display.print(speed >= 0 ? F("FORWARD") : F("REVERSE"));

    } else if (screen == 1) {
        // --- SCREEN 1: ENERGY STATUS (Y: 12-49) ---
        display.setTextSize(1);
        display.setCursor(0, 16); display.print(F("Voltage: ")); display.print(volt, 2); display.print(F("V"));
        display.setCursor(0, 28); display.print(F("Current: ")); display.print(amp, 2); display.print(F("A"));
        display.setCursor(0, 40); display.print(F("Power:   ")); display.print(pwr, 1); display.print(F("W"));

        // FOOTER ZONE (Y: 50-64)
        display.drawLine(0, 50, 127, 50, SSD1306_WHITE);
        display.setCursor(5, 54); display.print(F("Bat: ")); display.print(batPct); display.print(F("%"));
        uint8_t barWidth = 75;
        display.drawRect(50, 54, barWidth, 8, SSD1306_WHITE);
        uint8_t fillWidth = (batPct * (barWidth - 4)) / 100;
        display.fillRect(52, 56, fillWidth, 4, SSD1306_WHITE);

    } else {
        // --- SCREEN 2: NETWORK STATUS (Y: 12-49) ---
        if (wifiConnecting) {
            display.setTextSize(1);
            display.setCursor(0, 25); display.print(F("WiFi: Connecting..."));
            display.setCursor(0, 35); display.print(F("Please wait..."));
            display.display();
            return; // Skip drawing normal network info for now
        }
        // Text Info (Left Side)
        display.setTextSize(1);
        display.setCursor(0, 15); display.print(F("AP: Active"));
        display.setCursor(0, 25); display.print(F("IP: ")); display.print(ip);
        display.setCursor(0, 35); display.print(F("Users: ")); display.print(clients);

        // Footer Zone
        display.drawLine(0, 50, 127, 50, SSD1306_WHITE);
        display.setCursor(5, 54); display.print(F("Dashboard Connected"));
    }

    display.display(); // Push the buffer content to the actual OLED screen
#endif
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
 * @brief Triggers a visual pop-up on the OLED when a graft is detected.
 */
void triggerGraftFeedback() {
    s_graftFeedbackTimer = millis();
}

#endif