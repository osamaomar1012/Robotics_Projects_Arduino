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

// OLED Screen Configuration
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    16 
#define OLED_ADDR     0x3C

// --- ENHANCED UI ASSETS ---
// Large Icons (24x24) for Primary Data
// NOTE: A 24x24 1-bit bitmap requires (24 * 24) / 8 = 72 bytes.
// The original data might have been malformed or truncated.
// Replacing with a simple solid square for testing stability.
// If this works, generate your desired 24x24 icon using an online tool (e.g., image2cpp.com).
static const unsigned char PROGMEM icon_speed_large[] __attribute__((aligned(4))) = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // Total 72 bytes (24 columns * 3 bytes/column)
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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- CONFIRMATION MESSAGE STATE ---
static String s_confirmationMessage = "";
static unsigned long s_confirmationTimer = 0;
static bool s_oledInitialized = false;
#define CONFIRMATION_DURATION 2000 // Display message for 2 seconds

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
    // On TTGO OLED boards, GPIO 16 must be toggled to wake up the display
    // This sequence ensures the display controller is properly reset.
    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW);
    delay(50);
    digitalWrite(OLED_RESET, HIGH);

    Wire.begin(5, 4); // SDA on GPIO 5, SCL on GPIO 4
    
    // Give the hardware a moment to stabilize after reset
    delay(100);
    Wire.setClock(100000); 

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
}

/**
 * @brief Displays a professional splash screen for the system.
 */
void showWelcomeLogo() {
    if (!s_oledInitialized) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Decorative outer frame
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    
    // Branding - Main Title
    display.setTextSize(2);
    display.setCursor(10, 8);
    display.print(F("EL-BASEET"));

    // Subtitles
    display.setTextSize(1);
    display.setCursor(12, 30);
    display.print(F("HAIR PLANTING PEN"));
    display.setCursor(55, 42);
    display.print(F("V1.0"));

    display.display();
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
 */
void refreshOLED(int16_t speed, float volt, float amp, bool currentTrip, bool voltageTrip, uint32_t rpm, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint8_t screen, String ip, uint8_t clients, bool debug, uint32_t count, bool wifiConnecting) {
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

    // Debug Indicator (Inverted Tag) - Flashing every 500ms for high visibility
    if (debug && (millis() / 500) % 2 == 0) {
        display.fillRect(82, 0, 35, 9, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(84, 1); display.print(F("DEBUG"));
        display.setTextColor(SSD1306_WHITE);
    }
    
    // Simple Power Status (Text-Only)
    display.setCursor(105, 0);
    if (isCharging) {
        display.print(F("CHG"));
    } else if (isBatConnected) {
        display.print(batPct); display.print(F("%"));
    } else {
        display.print(F("USB"));
    }

    // Battery Percentage
    if (isBatConnected && !anyTrip && !debug) {
        display.setCursor(batPct == 100 ? 95 : 101, 2);
        display.print(batPct); display.print(F("%"));
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

    if (screen == 0) {
        // --- SCREEN 0: DRIVE DASHBOARD (Text-Only Stability Mode) ---
        display.setTextSize(2);
        display.setCursor(0, 15);
        display.print(F("SPD: ")); display.print(abs(speed));
        display.setCursor(0, 32);
        display.print(F("RPM: ")); display.print(rpm);

        // FOOTER ZONE (Y: 50-64)
        display.drawLine(0, 50, 127, 50, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(0, 55); 
        display.print(F("GRAFT: ")); display.print(count);
        display.setCursor(85, 55);
        display.print(speed >= 0 ? F("FWD") : F("REV"));

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
}

/**
 * @brief Sets a confirmation message to be displayed on the OLED.
 * @param msg The message string to display.
 */
void displayWebConfirmation(String msg) {
    s_confirmationMessage = msg;
    s_confirmationTimer = millis();
}

#endif