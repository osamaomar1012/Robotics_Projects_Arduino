/**
 * MODULE: Integrated TTGO T-Display & LilyGO T-Display-S3 (ST7789)
 * DESCRIPTION: 2026 Hybrid Medical UI:
 *              - Glowing Deep Clinical Sapphire theme for all normal screens, menus,
 *                and action confirmations.
 *              - Pure White seamless background EXCLUSIVELY for the Screensaver to blend
 *                with the white-background hair animation frames.
 *              - Screen 0 (Surgical Dash): Right side features a dedicated high-capacity Motor Card
 *                moved left to x=114, with increased width, labeled "MOTOR CW/CCW" (not turbine),
 *                housing the modern 2026 3-blade aerodynamic micro-turbine animation.
 *              - Telemetry card on left has increased width and increased Font 2 sizes.
 *              - Top splitter line lowered to y=26 for spacious header layout.
 *              - Text beautifully CENTERED across all menus and telemetry cards.
 *              - Header layout with non-overlapping zones: Title -> LED -> Robot -> Battery.
 *              - Interactive AI Robot Companion ("El-Baseet Buddy") on the RIGHT side of
 *                all menus and screens, with text cleanly positioned to its LEFT.
 *              - Full-screen borderless 2-column detailed summary in screensaver on pure white.
 * DEPENDENCIES: TFT_eSPI.h, Free_Fonts.h, SPI.h, WiFi.h, TJpg_Decoder.h
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include <SPI.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>

// Forward declaration of callback for TJpg_Decoder
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

// Frame Counts for Animated Transitions
#define WELCOME_FRAMES    105
#define START_OP_FRAMES   68
#define STOP_OP_FRAMES    60
#define REPORT_FRAMES     25

void showWelcomeAnimation();
void showStartOperationAnimation();
void showStopOperationAnimation();
void showReportAnimation();
void drawReportCountdownFrame(int secondsRemaining, int totalSeconds = 5);
void displayReportGeneratingCountdown(int totalSeconds = 5);

// Extern global references for local menu system & telemetry
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
extern bool forwardDirection;

// Instantiate TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

// Sprite object for flicker-free double buffering
TFT_eSprite spr = TFT_eSprite(&tft);

// Dedicated double-buffered sprite for auxiliary panels
TFT_eSprite clockSpr = TFT_eSprite(&tft);

// =============================================================================
// --- COLOR PALETTE DEFINITIONS ---
// =============================================================================
// 1. Glowing Clinical Sapphire Theme (Used for all Screens, Menus & Confirmations)
#define COLOR_BG          0x08C5   // Deep Clinical Sapphire (#0c192e - rich non-black)
#define COLOR_CARD        0x1149   // Translucent Clinical Card (#162a4a)
#define COLOR_CARD_HI     0x19ED   // Active Card Glass Highlight (#203d68)
#define COLOR_TXT_MAIN    TFT_WHITE// Crisp Pure White Text (#ffffff)
#define COLOR_TXT_WHITE   TFT_WHITE
#define COLOR_TXT_MUTED   0x9D17   // Ice Slate Grey (#94a3b8)
#define COLOR_GLOW_CYAN   0x079F   // Radiant Laser Cyan (#00f0ff - glowing highlight)
#define COLOR_GLOW_BLUE   0x231D   // Electric Medical Blue (#2563eb)
#define COLOR_EMERALD     0x15D0   // Surgical Vivid Green (#10b981)
#define COLOR_AMBER       0xF4E1   // Luminous Gold / Amber (#f59e0b)
#define COLOR_CORAL       0xFA64   // Coral Alert / Reverse (#f97316)
#define COLOR_ROSE        0xE0E9   // Neon Rose Smile (#e11d48)
#define COLOR_PINK        0xF396   // Blushing Robot Cheeks (#f472b6)
#define COLOR_DIVIDER     0x21EB   // Subtle Glass Divider (#233554)
#define COLOR_MENU_BAR    0x231D   // Electric Blue Selected Pill (#2563eb)

// 2. Pure White Theme (Used EXCLUSIVELY for the Screensaver to blend with white JPG frames)
#define COLOR_SAV_BG      TFT_WHITE                   // Pure White for screensaver canvas
#define COLOR_SAV_CARD    tft.color565(241, 245, 249) // Soft Ice Pearl Card (#f1f5f9)
#define COLOR_SAV_TXT     tft.color565(15, 23, 42)    // Deep Obsidian Navy Text (#0f172a)
#define COLOR_SAV_DIV     tft.color565(203, 213, 225) // Hairline Divider (#cbd5e1)

// --- CONFIRMATION MESSAGE STATE ---
static String s_confirmationMessage = "";
static unsigned long s_confirmationTimer = 0;
static bool s_oledInitialized = false;
#define CONFIRMATION_DURATION 2000

// --- GRAFT FLASH MESSAGE STATE ---
static String s_flashMessage = "";
static unsigned long s_flashTimer = 0;

// =============================================================================
// --- ANIMATED AI ROBOT COMPANION ENGINE ("EL-BASEET BUDDY") ---
// =============================================================================

/**
 * @brief Draws a compact animated robot companion face for screen headers.
 *        Positioned safely to the left of the battery gauge (zero collision).
 */
void drawRobotFaceCompact(TFT_eSprite &target, int16_t x, int16_t y, bool isRunning, int8_t dir) {
    target.fillRoundRect(x, y, 30, 16, 4, COLOR_CARD);
    target.drawRoundRect(x, y, 30, 16, 4, isRunning ? COLOR_GLOW_CYAN : COLOR_GLOW_BLUE);

    // Natural Eyelid Blink Timing (blinks for 150ms every 3.8s)
    uint32_t t = millis() % 3800;
    bool isBlink = (t < 150);

    int16_t lx = x + 8;
    int16_t rx = x + 22;
    int16_t ey = y + 6;

    if (isBlink) {
        target.drawFastHLine(lx - 3, ey, 7, COLOR_GLOW_CYAN);
        target.drawFastHLine(rx - 3, ey, 7, COLOR_GLOW_CYAN);
    } else {
        uint16_t eyeColor = isRunning ? COLOR_EMERALD : COLOR_GLOW_CYAN;
        target.fillCircle(lx, ey, 3, eyeColor);
        target.fillCircle(rx, ey, 3, eyeColor);

        // Gaze Pupil tracking direction
        int8_t pOff = (dir > 0) ? 1 : (dir < 0 ? -1 : 0);
        target.drawPixel(lx + pOff, ey, COLOR_CARD);
        target.drawPixel(rx + pOff, ey, COLOR_CARD);

        // White reflection dot
        target.drawPixel(lx - 1, ey - 1, COLOR_TXT_WHITE);
        target.drawPixel(rx - 1, ey - 1, COLOR_TXT_WHITE);
    }

    // Blushing Cheeks
    target.drawPixel(x + 3, y + 10, COLOR_PINK);
    target.drawPixel(x + 26, y + 10, COLOR_PINK);

    // Cheerful Smile
    int16_t mx = x + 15;
    int16_t my = y + 12;
    target.drawFastHLine(mx - 3, my, 7, COLOR_GLOW_CYAN);
    target.drawPixel(mx - 4, my - 1, COLOR_GLOW_CYAN);
    target.drawPixel(mx + 4, my - 1, COLOR_GLOW_CYAN);
}

/**
 * @brief Draws the dedicated interactive animated robot companion pod on the RIGHT
 *        side of screens and menus, with telemetry/menu text cleanly positioned to its LEFT.
 *        The robot eyes look LEFT at the data/menu choices!
 */
// =============================================================================
// --- UNIVERSAL MORPHING ROBOT COMPANION ENGINE ("EL-BASEET BUDDY") ---
// =============================================================================
enum MorphTargetType {
    MORPH_MOTOR = 0,       // Screen 0: Spinning 3-Blade Motor Turbine
    MORPH_LIGHTNING = 1,   // Screen 1: Glowing High-Voltage Energy Bolt
    MORPH_WIFI = 2,        // Screen 2: Concentric Radiating WiFi Waves
    MORPH_OSCILLATION = 3, // Screen 3: Bi-Directional Harmonic Wave
    MORPH_DIAGNOSTICS = 4, // Screen 4: Medical ECG / Sensor Current Spike
    MORPH_PATIENT = 5,     // Screen 5: Clinical Cross Shield
    MORPH_GEAR = 6         // System Menu: Precision Mechanical Config Gear
};

/**
 * @brief Draws the dynamic morphing robot companion pod.
 *        The robot face blinks, smiles, and smoothly transforms into the screen's
 *        thematic artifact (turbine, lightning, wifi, wave, ecg, shield, gear)
 *        and then transforms back into the friendly smiling robot companion!
 */
void drawUniversalMorphingPod(TFT_eSprite &target, int16_t x, int16_t y, int16_t w, int16_t h, const char* robotTitle, const char* morphTitle, const char* badge, uint16_t badgeColor, uint8_t morphType, int16_t liveSpeed = 0) {
    target.fillRoundRect(x, y, w, h, 6, COLOR_CARD);
    target.drawRoundRect(x, y, w, h, 6, COLOR_GLOW_BLUE);

    int16_t cx = x + w / 2;
    int16_t cy = y + 36;

    // 10-Second Continuous Morphing Cycle:
    // 0..3500ms: Robot Face (0.0)
    // 3500..5000ms: Morphing to Artifact (0.0 -> 1.0)
    // 5000..8500ms: Artifact Mode (1.0)
    // 8500..10000ms: Morphing back to Robot (1.0 -> 0.0)
    uint32_t t = millis() % 10000;
    float morph = 0.0f;

    // If motor is actively running on Screen 0, lock to Turbine mode for live speed visualization!
    if (morphType == MORPH_MOTOR && liveSpeed != 0) {
        morph = 1.0f;
    } else {
        if (t < 3500) morph = 0.0f;
        else if (t < 5000) {
            float p = (t - 3500) / 1500.0f;
            morph = sin(p * 1.5707963f);
        } else if (t < 8500) morph = 1.0f;
        else {
            float p = (t - 8500) / 1500.0f;
            morph = cos(p * 1.5707963f);
        }
    }

    // Dynamic Pod Header
    target.setTextDatum(TC_DATUM);
    if (morph < 0.5f) {
        target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        target.drawString(robotTitle, cx, y + 4, 1);
    } else {
        target.setTextColor(COLOR_AMBER, COLOR_CARD);
        target.drawString(morphTitle, cx, y + 4, 1);
    }

    // Dynamic Visor Shell
    int16_t vw = (int16_t)(38.0f - morph * 4.0f);
    int16_t vh = (int16_t)(28.0f - morph * 2.0f);
    target.fillRoundRect(cx - vw / 2, cy - vh / 2, vw, vh, 5, COLOR_BG);
    target.drawRoundRect(cx - vw / 2, cy - vh / 2, vw, vh, 5, (morph > 0.5f) ? COLOR_GLOW_BLUE : COLOR_GLOW_CYAN);

    // Ear Nodes (fade as morph increases)
    if (morph < 0.6f) {
        int16_t earW = (int16_t)((1.0f - morph / 0.6f) * 4.0f);
        if (earW > 0) {
            target.fillRoundRect(cx - vw / 2 - earW, cy - 5, earW + 1, 10, 2, COLOR_GLOW_BLUE);
            target.fillRoundRect(cx + vw / 2 - 1, cy - 5, earW + 1, 10, 2, COLOR_GLOW_BLUE);
        }
    }

    // --- RENDER VISUAL BASED ON MORPH PROGRESS ---
    if (morph < 0.45f) {
        // =====================================================================
        // ROBOT AVATAR FACE (Blinking, Looking Left at Telemetry, Smiling)
        // =====================================================================
        uint32_t bt = millis() % 3600;
        bool isBlink = (bt < 150) && (morph < 0.2f);

        int16_t lx = cx - 7;
        int16_t rx = cx + 7;
        int16_t ey = cy - 2;

        if (isBlink) {
            target.drawFastHLine(lx - 3, ey, 7, COLOR_GLOW_CYAN);
            target.drawFastHLine(rx - 3, ey, 7, COLOR_GLOW_CYAN);
        } else {
            target.fillCircle(lx, ey, 3, COLOR_GLOW_CYAN);
            target.fillCircle(rx, ey, 3, COLOR_GLOW_CYAN);
            // Pupils looking LEFT at the menu/data
            target.fillCircle(lx - 1, ey, 1, COLOR_BG);
            target.fillCircle(rx - 1, ey, 1, COLOR_BG);
            // Reflection sparkle
            target.drawPixel(lx, ey - 1, COLOR_TXT_WHITE);
            target.drawPixel(rx, ey - 1, COLOR_TXT_WHITE);
        }

        // Blushing pink cheeks
        target.drawPixel(cx - 12, cy + 5, COLOR_PINK);
        target.drawPixel(cx + 12, cy + 5, COLOR_PINK);

        // Smile
        target.drawFastHLine(cx - 3, cy + 6, 7, COLOR_AMBER);
        target.drawPixel(cx - 4, cy + 5, COLOR_AMBER);
        target.drawPixel(cx + 4, cy + 5, COLOR_AMBER);

    } else {
        // =====================================================================
        // MORPHED SYMBOLIC ARTIFACT (Custom per Screen!)
        // =====================================================================
        if (morphType == MORPH_MOTOR) {
            // --- SCREEN 0: MOTOR TURBINE ---
            int r = 10;
            static float turbineAngle = 0;
            float rotSpeed = (liveSpeed != 0) ? ((float)liveSpeed / 100.0f * 16.0f) : 3.5f;
            turbineAngle += rotSpeed;
            if (turbineAngle >= 360.0f) turbineAngle -= 360.0f;

            target.drawCircle(cx, cy, r + 2, COLOR_DIVIDER);
            for (int i = 0; i < 3; i++) {
                float a = (turbineAngle + i * 120) * DEG_TO_RAD;
                int tx = cx + (int)(r * cos(a));
                int ty = cy + (int)(r * sin(a));
                target.drawLine(cx, cy, tx, ty, (liveSpeed != 0 && liveSpeed < 0) ? COLOR_CORAL : COLOR_GLOW_CYAN);
            }
            target.fillCircle(cx, cy, 3, COLOR_AMBER);

        } else if (morphType == MORPH_LIGHTNING) {
            // --- SCREEN 1: GLOWING LIGHTNING BOLT ---
            // High-voltage zigzag lightning bolt
            int lx = cx;
            int ly = cy;
            target.fillTriangle(lx + 1, ly - 9, lx - 5, ly, lx, ly, COLOR_AMBER);
            target.fillTriangle(lx, ly, lx - 3, ly, lx - 1, ly + 9, COLOR_AMBER);
            target.fillTriangle(lx, ly, lx + 5, ly, lx - 1, ly + 9, COLOR_GLOW_CYAN);
            // Pulsing energy sparks
            if ((millis() / 120) % 2 == 0) {
                target.drawPixel(lx - 7, ly - 4, COLOR_GLOW_CYAN);
                target.drawPixel(lx + 7, ly + 4, COLOR_AMBER);
            }

        } else if (morphType == MORPH_WIFI) {
            // --- SCREEN 2: RADIATING CONCENTRIC WIFI WAVES ---
            int phase = (millis() / 250) % 3;
            target.fillCircle(cx, cy + 5, 2, COLOR_GLOW_CYAN);
            for (int wIdx = 0; wIdx < 3; wIdx++) {
                int rad = 6 + wIdx * 4;
                uint16_t wColor = (wIdx <= phase) ? COLOR_GLOW_CYAN : COLOR_DIVIDER;
                for (int deg = 225; deg <= 315; deg += 15) {
                    float radVal = deg * DEG_TO_RAD;
                    target.drawPixel(cx + (int)(rad * cos(radVal)), cy + 5 + (int)(rad * sin(radVal)), wColor);
                }
            }

        } else if (morphType == MORPH_OSCILLATION) {
            // --- SCREEN 3: BI-DIRECTIONAL HARMONIC SINE WAVE ---
            static float waveShift = 0;
            waveShift += 0.25f;
            for (int wx = -12; wx <= 12; wx++) {
                int wy = (int)(sin((wx * 0.35f) + waveShift) * 5.0f);
                target.drawPixel(cx + wx, cy + wy, COLOR_GLOW_CYAN);
            }
            // Bi-directional cycling arrows
            target.drawPixel(cx - 10, cy - 6, COLOR_EMERALD);
            target.drawPixel(cx + 10, cy + 6, COLOR_AMBER);

        } else if (morphType == MORPH_DIAGNOSTICS) {
            // --- SCREEN 4: MEDICAL ECG / SENSOR PULSE SPIKE ---
            static int ecgTick = 0;
            ecgTick = (ecgTick + 1) % 24;
            for (int ex = -12; ex <= 12; ex++) {
                int eyOff = 0;
                int pos = ex + 12;
                if (pos == ecgTick) eyOff = -7;
                else if (pos == (ecgTick + 1) % 24) eyOff = 5;
                else if (pos == (ecgTick + 2) % 24) eyOff = -2;
                target.drawPixel(cx + ex, cy + eyOff, COLOR_CORAL);
            }

        } else if (morphType == MORPH_PATIENT) {
            // --- SCREEN 5: GLOWING CLINICAL CROSS SHIELD ---
            target.drawRoundRect(cx - 10, cy - 9, 20, 18, 4, COLOR_GLOW_BLUE);
            target.fillRect(cx - 2, cy - 6, 4, 12, COLOR_EMERALD);
            target.fillRect(cx - 6, cy - 2, 12, 4, COLOR_EMERALD);
            target.drawPixel(cx - 1, cy - 1, COLOR_TXT_WHITE);

        } else if (morphType == MORPH_GEAR) {
            // --- SYSTEM MENU: PRECISION CONFIG GEAR ---
            int r = 8;
            static float gearAngle = 0;
            gearAngle += 3.0f;
            if (gearAngle >= 360.0f) gearAngle -= 360.0f;

            target.drawCircle(cx, cy, r, COLOR_AMBER);
            for (int tooth = 0; tooth < 8; tooth++) {
                float a = (gearAngle + tooth * 45) * DEG_TO_RAD;
                int gx = cx + (int)((r + 2) * cos(a));
                int gy = cy + (int)((r + 2) * sin(a));
                target.drawPixel(gx, gy, COLOR_GLOW_CYAN);
            }
            target.fillCircle(cx, cy, 3, COLOR_BG);
            target.drawCircle(cx, cy, 3, COLOR_GLOW_BLUE);
        }
    }

    // Status Badge at bottom of pod
    target.setTextDatum(BC_DATUM);
    target.setTextColor(badgeColor, COLOR_CARD);
    target.drawString(badge, cx, y + h - 5, 1);
}


/**
 * @brief Initializes the ST7789 display controller on TTGO / S3.
 */
void setupOLED() {
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_LILYGO_T_DISPLAY_S3)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    delay(50);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    delay(10);
    digitalWrite(5, LOW);
    delay(20);
    digitalWrite(5, HIGH);
    delay(50);
    pinMode(38, OUTPUT);
    digitalWrite(38, HIGH);
#endif

    tft.init();
    tft.setRotation(1); // Landscape
    tft.fillScreen(COLOR_BG);

    spr.setColorDepth(16);
    spr.createSprite(tft.width(), tft.height());

    s_oledInitialized = true;
}

/**
 * @brief Draws the branded welcome logo.
 */
void showWelcomeLogo() {
    if (!s_oledInitialized) return;

    // Phase 1: Letter-by-letter writing animation with laser cursor
    const char* welcomeStr = "WELCOME";
    int len = strlen(welcomeStr);

    for (int i = 1; i <= len; i++) {
        spr.fillSprite(COLOR_BG);
        spr.fillRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_CARD);
        spr.drawRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_GLOW_CYAN);
        spr.drawRoundRect(9, 9, spr.width() - 18, spr.height() - 18, 7, COLOR_GLOW_BLUE);

        char tempBuf[12];
        strncpy(tempBuf, welcomeStr, i);
        tempBuf[i] = ' ';

        spr.setTextDatum(MC_DATUM);
        spr.setFreeFont(FF22); // FreeSansBold 12pt
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(String(tempBuf) + "_", spr.width() / 2, 45);

        spr.setTextFont(1);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString("EL-BASEET SURGICAL PEN", spr.width() / 2, 75, 2);

        spr.pushSprite(0, 0);
        delay(60);
    }

    delay(200);

    // Phase 2: "WELCOME" smoothly morphs into the Robot Avatar Face!
    int cx = spr.width() / 2;
    int cy = spr.height() / 2;

    for (int step = 0; step <= 20; step++) {
        float p = step / 20.0f;
        spr.fillSprite(COLOR_BG);
        spr.fillRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_CARD);
        spr.drawRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_GLOW_CYAN);

        // Header brand subtitle
        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString("EL-BASEET SURGICAL PEN", cx, 16, 2);

        // Robot Face Visor expands into view
        int w = (int)(20 + p * 36); // 20 -> 56
        int h = (int)(14 + p * 24); // 14 -> 38
        spr.fillRoundRect(cx - w / 2, cy - h / 2 + 6, w, h, 6, COLOR_BG);
        spr.drawRoundRect(cx - w / 2, cy - h / 2 + 6, w, h, 6, COLOR_GLOW_CYAN);

        // Ear nodes appear
        if (p > 0.4f) {
            spr.fillRoundRect(cx - w / 2 - 4, cy + 2, 4, 12, 2, COLOR_GLOW_BLUE);
            spr.fillRoundRect(cx + w / 2, cy + 2, 4, 12, 2, COLOR_GLOW_BLUE);
        }

        // Eyes open and shine
        if (p > 0.5f) {
            int eyeDist = 12;
            int ey = cy + 2;
            spr.fillCircle(cx - eyeDist, ey, 4, COLOR_GLOW_CYAN);
            spr.fillCircle(cx + eyeDist, ey, 4, COLOR_GLOW_CYAN);
            spr.drawPixel(cx - eyeDist - 1, ey - 1, COLOR_TXT_WHITE);
            spr.drawPixel(cx + eyeDist - 1, ey - 1, COLOR_TXT_WHITE);
        }

        // Cheeks & Smile appear
        if (p > 0.7f) {
            spr.drawPixel(cx - 16, cy + 10, COLOR_PINK);
            spr.drawPixel(cx + 16, cy + 10, COLOR_PINK);

            spr.drawFastHLine(cx - 5, cy + 12, 11, COLOR_AMBER);
            spr.drawPixel(cx - 6, cy + 11, COLOR_AMBER);
            spr.drawPixel(cx + 6, cy + 11, COLOR_AMBER);
        }

        spr.setTextDatum(BC_DATUM);
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString("● READY STANDBY", cx, spr.height() - 14, 2);

        spr.pushSprite(0, 0);
        delay(25);
    }

    // Celebration wink & pause
    delay(400);
}

/**
 * @brief Draws a clean high-contrast battery gauge with glowing charge states.
 *        Separated cleanly to prevent any overlap with the robot face.
 */
void drawBatteryIcon(TFT_eSprite &targetSpr, bool isConnected, bool isCharging, uint8_t batPct) {
    int16_t x = targetSpr.width() - 24;
    int16_t y = 6;

    if (isCharging) {
        targetSpr.drawRoundRect(x, y, 20, 12, 2, COLOR_TXT_WHITE);
        targetSpr.fillRect(x + 20, y + 3, 2, 6, COLOR_TXT_WHITE);

        uint8_t animPct = 20 + ((millis() / 280) % 5) * 20;
        int fillW = map(animPct, 0, 100, 0, 16);
        if (fillW > 0) targetSpr.fillRect(x + 2, y + 2, fillW, 8, COLOR_EMERALD);

        targetSpr.setTextDatum(TR_DATUM);
        targetSpr.setTextColor(COLOR_AMBER, COLOR_BG);
        targetSpr.drawString("CHG", x - 3, y - 1, 2);
    } else if (isConnected) {
        targetSpr.setTextDatum(TR_DATUM);
        uint16_t pctColor = (batPct < 20) ? COLOR_CORAL : COLOR_TXT_WHITE;
        targetSpr.setTextColor(pctColor, COLOR_BG);
        targetSpr.drawString(String(batPct) + "%", x - 3, y - 1, 2);

        targetSpr.drawRoundRect(x, y, 20, 12, 2, COLOR_TXT_WHITE);
        targetSpr.fillRect(x + 20, y + 3, 2, 6, COLOR_TXT_WHITE);

        uint16_t barColor = (batPct < 20) ? COLOR_CORAL : (batPct < 50) ? COLOR_AMBER : COLOR_EMERALD;
        int fillW = map(batPct, 0, 100, 0, 16);
        if (fillW > 0) targetSpr.fillRect(x + 2, y + 2, fillW, 8, barColor);
    }
}

/**
 * @brief Full refresh routine drawing all UI screens with glowing dark theme,
 *        centered menu/data text, non-overlapping header, and FULL-SCREEN white screensaver.
 */
/**
 * @brief Draws the complete, borderless Full-Screen Executive Telemetry Summary.
 *        Used as the dedicated Summary Screen (Screen 6) and in the Screensaver cycle.
 */
void drawSummaryScreen(TFT_eSprite &target, int16_t speed, float volt, float amp, uint32_t rpm, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint32_t count) {
    // 1. Deep Clinical Sapphire Background (Zero white background!)
    target.fillSprite(COLOR_BG);

    // Line 1: ● READY STANDBY (Centered, prominent, Font 2 in Emerald Green)
    target.setTextDatum(TC_DATUM);
    target.setTextColor(COLOR_EMERALD, COLOR_BG);
    target.drawString("● READY STANDBY", target.width() / 2, 4, 2);

    // Line 2: CLINICAL TELEMETRY EXECUTIVE SUMMARY (Centered, Font 1)
    target.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
    target.drawString("CLINICAL TELEMETRY EXECUTIVE SUMMARY", target.width() / 2, 19, 1);

    // Line 3: ─────────────────────────────────────────────────────────────
    target.drawFastHLine(0, 27, target.width(), COLOR_GLOW_BLUE);
    target.drawFastHLine(target.width() / 2 - 25, 27, 50, COLOR_GLOW_CYAN);

    // Lines 4 to 9: TWO CARDS SIDE-BY-SIDE WITH GLOWING BLUE ROUNDED BORDERS
    int margin = 4;
    int gap = 6;
    int colW = (target.width() - (margin * 2) - gap) / 2; // 113px on 240, 153px on 320
    int colH = 82;
    int colY = 30;

    int card1X = margin;
    int card2X = margin + colW + gap;
    int cx1 = card1X + colW / 2;
    int cx2 = card2X + colW / 2;

    // --- LEFT CARD: ╭─────── ENERGY & POWER ────────╮ ---
    target.fillRoundRect(card1X, colY, colW, colH, 6, COLOR_CARD);
    target.drawRoundRect(card1X, colY, colW, colH, 6, COLOR_GLOW_BLUE); // Glowing card border!

    target.setTextDatum(TC_DATUM);
    target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    target.drawString("ENERGY & POWER", cx1, colY + 3, 1);
    target.drawFastHLine(card1X + 4, colY + 13, colW - 8, COLOR_DIVIDER);

    target.setTextDatum(TL_DATUM);
    target.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
    target.drawString("Battery:", card1X + 5, colY + 18, 1);
    target.drawString("State:  ", card1X + 5, colY + 33, 1);
    target.drawString("Current:", card1X + 5, colY + 48, 1);
    target.drawString("Power:  ", card1X + 5, colY + 63, 1);

    uint16_t bCol = (batPct < 20) ? COLOR_CORAL : (batPct < 50) ? COLOR_AMBER : COLOR_EMERALD;
    target.setTextColor(bCol, COLOR_CARD);
    target.drawString(String(volt, 2) + "V (" + String(batPct) + "%)", card1X + 48, colY + 18, 1);

    target.setTextColor(isCharging ? COLOR_EMERALD : COLOR_TXT_MUTED, COLOR_CARD);
    target.drawString(isCharging ? "⚡ USB Active" : "🔋 Discharging", card1X + 48, colY + 33, 1);

    target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    target.drawString(String(amp, 3) + " A", card1X + 48, colY + 48, 1);
    target.drawString(String(pwr, 1) + " W", card1X + 48, colY + 63, 1);
    // ╰───────────────────────────────╯

    // --- RIGHT CARD: ╭────── SURGICAL STATUS ──────╮ ---
    target.fillRoundRect(card2X, colY, colW, colH, 6, COLOR_CARD);
    target.drawRoundRect(card2X, colY, colW, colH, 6, COLOR_GLOW_BLUE); // Glowing card border!

    target.setTextDatum(TC_DATUM);
    target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    target.drawString("SURGICAL STATUS", cx2, colY + 3, 1);
    target.drawFastHLine(card2X + 4, colY + 13, colW - 8, COLOR_DIVIDER);

    target.setTextDatum(TL_DATUM);
    target.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
    target.drawString("Motor: ", card2X + 5, colY + 18, 1);
    target.drawString("Speed: ", card2X + 5, colY + 33, 1);
    target.drawString("Mode:  ", card2X + 5, colY + 48, 1);
    target.drawString("Grafts:", card2X + 5, colY + 63, 1);

    if (speed != 0) {
        target.setTextColor(COLOR_EMERALD, COLOR_CARD);
        target.drawString(forwardDirection ? "RUN (FWD)" : "RUN (REV)", card2X + 46, colY + 18, 1);
    } else {
        target.setTextColor(COLOR_AMBER, COLOR_CARD);
        target.drawString("STANDBY IDLE", card2X + 46, colY + 18, 1);
    }

    int pSpd = webBaseSpeed;
    target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    target.drawString(String(pSpd) + "% (" + String(rpm) + ")", card2X + 46, colY + 33, 1);

    target.setTextColor(COLOR_GLOW_BLUE, COLOR_CARD);
    target.drawString(oscillatingMode ? "OSCILLATING" : "NORMAL", card2X + 46, colY + 48, 1);

    target.setTextColor(COLOR_AMBER, COLOR_CARD);
    target.drawString(String(count) + " completed", card2X + 46, colY + 63, 1);
    // ╰─────────────────────────────╯

    // Line 10: ─────────────────────────────────────────────────────────────
    target.drawFastHLine(0, 115, target.width(), COLOR_GLOW_BLUE);

    // Line 11: 12:45 PM | Oct 04, 2026                 ● CALIBRATION STABLE
    struct tm timeinfo;
    bool hasTime = getLocalTime(&timeinfo, 5);
    target.setTextDatum(TL_DATUM);
    if (hasTime && timeinfo.tm_year >= 120) {
        char timeBuff[28];
        strftime(timeBuff, sizeof(timeBuff), "%I:%M %p | %b %d, %Y", &timeinfo);
        target.setTextColor(COLOR_TXT_MAIN, COLOR_BG);
        target.drawString(timeBuff, 6, 119, 1);
    } else {
        target.setTextColor(COLOR_TXT_MAIN, COLOR_BG);
        target.drawString("EL-BASEET SURGICAL PEN", 6, 119, 1);
    }

    target.setTextDatum(TR_DATUM);
    target.setTextColor(COLOR_EMERALD, COLOR_BG);
    target.drawString("● CALIBRATION STABLE", target.width() - 6, 119, 1);
    target.setTextDatum(TL_DATUM);
}

/**
 * @brief Draws the Morphing Robot-to-Pen Standby Animation Screen.
 *        The animated robot face floats between the left card and right card,
 *        smoothly transforms its shape into a precision surgical hair pen handpiece,
 *        and then morphs back into the cheerful robot face.
 */
void drawMorphingStandbyScreen(TFT_eSprite &target, int16_t speed, float volt, float amp, uint32_t rpm, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint32_t count) {
    target.fillSprite(COLOR_BG);

    // 1. Shared Header
    target.setTextSize(1);
    target.setTextDatum(TL_DATUM);
    target.setTextColor(COLOR_GLOW_CYAN, COLOR_BG);
    target.drawString("STANDBY ACTIVE", 6, 6, 2);

    // Header title placed cleanly on left with zero overlap

    drawBatteryIcon(target, isBatConnected, isCharging, batPct);

    target.drawFastHLine(0, 26, target.width(), COLOR_GLOW_BLUE);
    target.drawFastHLine(target.width() / 2 - 25, 26, 50, COLOR_GLOW_CYAN);

    // 2. Left Bay & Right Bay Staging Cards
    int margin = 6;
    int gap = 12;
    int bayW = (target.width() - (margin * 2) - gap) / 2;
    int bayH = 82;
    int bayY = 30;

    int bay1X = margin;
    int bay2X = margin + bayW + gap;
    int cx1 = bay1X + (bayW / 2);
    int cx2 = bay2X + (bayW / 2);

    target.fillRoundRect(bay1X, bayY, bayW, bayH, 6, COLOR_CARD);
    target.drawRoundRect(bay1X, bayY, bayW, bayH, 6, COLOR_DIVIDER);
    target.setTextDatum(TC_DATUM);
    target.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    target.drawString("MEDICAL AVATAR", cx1, bayY + 3, 1);
    target.drawFastHLine(bay1X + 4, bayY + 13, bayW - 8, COLOR_DIVIDER);

    target.fillRoundRect(bay2X, bayY, bayW, bayH, 6, COLOR_CARD);
    target.drawRoundRect(bay2X, bayY, bayW, bayH, 6, COLOR_DIVIDER);
    target.setTextDatum(TC_DATUM);
    target.setTextColor(COLOR_AMBER, COLOR_CARD);
    target.drawString("SURGICAL PEN", cx2, bayY + 3, 1);
    target.drawFastHLine(bay2X + 4, bayY + 13, bayW - 8, COLOR_DIVIDER);

    // 3. Procedural Morphing Calculations (10-second cycle)
    uint32_t cycle = millis() % 10000;
    float curX = cx1;
    float morph = 0.0f; // 0.0 = Robot Face, 1.0 = Surgical Pen

    if (cycle < 2500) {
        curX = cx1;
        morph = 0.0f;
    } else if (cycle < 4500) {
        float p = (cycle - 2500) / 2000.0f;
        curX = cx1 + p * (cx2 - cx1);
        morph = sin(p * 1.5707963f);
    } else if (cycle < 7000) {
        curX = cx2;
        morph = 1.0f;
    } else if (cycle < 9000) {
        float p = (cycle - 7000) / 2000.0f;
        curX = cx2 - p * (cx2 - cx1);
        morph = cos(p * 1.5707963f);
    } else {
        curX = cx1;
        morph = 0.0f;
    }

    float floatY = sin(millis() / 240.0f) * 2.5f;
    int16_t cy = bayY + 46 + (int16_t)floatY;
    int16_t cx = (int16_t)curX;

    int16_t w = (int16_t)(42.0f + morph * 32.0f);
    int16_t h = (int16_t)(30.0f - morph * 16.0f);
    int16_t vx = cx - w / 2;
    int16_t vy = cy - h / 2;

    // Conical Surgical Punch Needle emerges as morph progresses
    if (morph > 0.08f) {
        int16_t needleBaseX = cx + w / 2;
        int16_t needleLen = (int16_t)(morph * 16.0f);
        int16_t needleH = (int16_t)(morph * 4.0f);
        target.fillTriangle(needleBaseX, cy - needleH, needleBaseX, cy + needleH, needleBaseX + needleLen, cy, COLOR_GLOW_CYAN);
        target.drawPixel(needleBaseX + needleLen, cy, COLOR_TXT_WHITE);
        if (morph > 0.7f && (millis() / 150) % 2 == 0) {
            target.drawPixel(needleBaseX + needleLen + 2, cy, COLOR_GLOW_CYAN);
        }
    }

    // Ear Nodes (Robot state)
    if (morph < 0.6f) {
        int16_t earW = (int16_t)((1.0f - morph / 0.6f) * 5.0f);
        if (earW > 0) {
            target.fillRoundRect(vx - earW, cy - 6, earW + 1, 12, 2, COLOR_GLOW_BLUE);
            target.fillRoundRect(vx + w - 1, cy - 6, earW + 1, 12, 2, COLOR_GLOW_BLUE);
        }
    }

    // Main Shell
    target.fillRoundRect(vx, vy, w, h, (int16_t)(6 - morph * 3), COLOR_CARD_HI);
    target.drawRoundRect(vx, vy, w, h, (int16_t)(6 - morph * 3), COLOR_GLOW_CYAN);

    if (morph < 0.5f) {
        // --- ROBOT FACE MODE ---
        int16_t eyeDist = (int16_t)((1.0f - morph * 1.5f) * 9.0f);
        int16_t ey = cy - 2;

        uint32_t t = millis() % 3500;
        bool isBlink = (t < 150) && (morph < 0.2f);

        if (isBlink) {
            target.drawFastHLine(cx - eyeDist - 3, ey, 7, COLOR_GLOW_CYAN);
            target.drawFastHLine(cx + eyeDist - 3, ey, 7, COLOR_GLOW_CYAN);
        } else {
            target.fillCircle(cx - eyeDist, ey, 3, COLOR_GLOW_CYAN);
            target.fillCircle(cx + eyeDist, ey, 3, COLOR_GLOW_CYAN);
            target.drawPixel(cx - eyeDist - 1, ey - 1, COLOR_TXT_WHITE);
            target.drawPixel(cx + eyeDist - 1, ey - 1, COLOR_TXT_WHITE);
        }

        target.drawPixel(cx - eyeDist - 4, cy + 4, COLOR_PINK);
        target.drawPixel(cx + eyeDist + 4, cy + 4, COLOR_PINK);

        target.drawFastHLine(cx - 3, cy + 6, 7, COLOR_AMBER);
        target.drawPixel(cx - 4, cy + 5, COLOR_AMBER);
        target.drawPixel(cx + 4, cy + 5, COLOR_AMBER);
    } else {
        // --- SURGICAL PEN MODE ---
        int16_t collarX = cx - 4;
        target.fillRoundRect(collarX, vy + 1, 6, h - 2, 2, COLOR_EMERALD);
        target.drawRoundRect(collarX, vy + 1, 6, h - 2, 2, COLOR_GLOW_CYAN);

        target.drawFastVLine(cx - 16, vy + 2, h - 4, COLOR_DIVIDER);
        target.drawFastVLine(cx - 12, vy + 2, h - 4, COLOR_DIVIDER);
        target.drawFastVLine(cx - 8, vy + 2, h - 4, COLOR_DIVIDER);

        target.drawPixel(cx + 12, cy, COLOR_GLOW_CYAN);
        target.drawPixel(cx + 14, cy, COLOR_AMBER);
    }

    target.setTextDatum(BC_DATUM);
    target.setTextColor(COLOR_EMERALD, COLOR_CARD);
    target.drawString("CLINICAL AVATAR", cx1, bayY + bayH - 4, 1);

    target.setTextColor(COLOR_AMBER, COLOR_CARD);
    target.drawString("PRECISION PEN", cx2, bayY + bayH - 4, 1);

    target.drawFastHLine(0, 115, target.width(), COLOR_GLOW_BLUE);
    target.setTextDatum(TL_DATUM);
    target.setTextColor(COLOR_AMBER, COLOR_BG);
    target.drawString("GRAFTS: " + String(count) + " completed", 6, 118, 2);

    target.setTextDatum(TR_DATUM);
    target.setTextColor(COLOR_EMERALD, COLOR_BG);
    target.drawString("STANDBY READY", target.width() - 6, 118, 2);
    target.setTextDatum(TL_DATUM);
}

void refreshOLED(int16_t speed, float volt, float amp, bool currentTrip, bool voltageTrip, uint32_t rpm, float pwr, bool isBatConnected, uint8_t batPct, bool isCharging, uint8_t screen, String ip, uint8_t clients, bool debug, uint32_t count, bool wifiConnecting, bool oscMode, uint32_t oscCW, uint32_t oscCCW, String pName, String pAge, String pNat) {
    if (!s_oledInitialized) return;

    // --- OVERRIDE: WEB / BUTTON CONFIRMATION MESSAGE (CLEAN & MODERN, NO ROBOT FACE) ---
    if (millis() - s_confirmationTimer < CONFIRMATION_DURATION) {
        spr.fillSprite(COLOR_BG);

        // Elegant Glowing Sapphire Card with Double Border
        spr.fillRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_CARD);
        spr.drawRoundRect(8, 8, spr.width() - 16, spr.height() - 16, 8, COLOR_GLOW_CYAN);
        spr.drawRoundRect(9, 9, spr.width() - 18, spr.height() - 18, 7, COLOR_GLOW_BLUE);

        // Subtitle tag
        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("SYSTEM NOTIFICATION", spr.width() / 2, 22, 2);

        // Clean Modern FreeFont message (smooth vector typography in glowing cyan)
        spr.setFreeFont(FF22); // FreeSansBold12pt7b
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(s_confirmationMessage, spr.width() / 2, spr.height() / 2 + 4);

        // Confirmation Status Pill
        spr.setTextDatum(BC_DATUM);
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString("● CONFIRMED", spr.width() / 2, spr.height() - 18, 2);

        spr.setTextFont(1);
        spr.setTextSize(1);
        spr.pushSprite(0, 0);
        return;
    }

    // --- OVERRIDE: GRAFT DETECTION CONFIRMATION FLASH ---
    if (millis() - s_flashTimer < 200) {
        spr.fillSprite(COLOR_CARD_HI);
        spr.setFreeFont(FF22);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD_HI);
        spr.drawString("GRAFT #" + s_flashMessage, spr.width() / 2, spr.height() / 2);
        spr.setTextFont(1);
        spr.setTextSize(1);
        spr.pushSprite(0, 0);
        return;
    }

    // --- LOCAL MENU MODE OVERRIDE (TEXT BEAUTIFULLY CENTERED, ROBOT COMPANION ON RIGHT) ---
    if (inMenuMode) {
        spr.fillSprite(COLOR_BG);
        spr.setTextSize(1);
        spr.setTextDatum(TL_DATUM);

        // Header Title on Left
        spr.setTextColor(COLOR_AMBER, COLOR_BG);
        spr.drawString("SYSTEM MENU", 6, 6, 2);

        // Exit Hint
        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
        spr.drawString("[HOLD EXIT]", spr.width() - 6, 6, 2);
        spr.setTextDatum(TL_DATUM);

        // Lowered splitter line at y=26
        spr.drawFastHLine(0, 26, spr.width(), COLOR_GLOW_BLUE);
        spr.drawFastHLine(spr.width() / 2 - 25, 26, 50, COLOR_GLOW_CYAN);

        // Menu items on the LEFT side (leaving space on the right for robot companion)
        int menuWidth = spr.width() - 64;
        int pillCX = 4 + (menuWidth / 2); // Exact horizontal center of the menu pill!

        const char* labels[] = {
            "1. Speed: ",
            "2. LED Light: ",
            "3. Mode: ",
            "4. Hardw: ",
            "5. Reset Grafts",
            "6. Exit Menu"
        };

        for (uint8_t i = 0; i < 6; i++) {
            int yPos = 30 + (i * 17);

            if (i == currentMenuItem) {
                // Active item: Glowing Electric Blue selection pill with white text
                spr.fillRoundRect(4, yPos - 1, menuWidth, 16, 4, COLOR_MENU_BAR);
                spr.drawRoundRect(4, yPos - 1, menuWidth, 16, 4, COLOR_GLOW_CYAN);
                spr.setTextColor(COLOR_TXT_WHITE, COLOR_MENU_BAR);
            } else {
                // Inactive item: Soft translucent card
                spr.fillRoundRect(4, yPos - 1, menuWidth, 16, 4, COLOR_CARD);
                spr.drawRoundRect(4, yPos - 1, menuWidth, 16, 4, COLOR_DIVIDER);
                spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
            }

            String text = labels[i];
            if (i == 0) {
                int percentSpeed = webBaseSpeed;
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

            // Text BEAUTIFULLY CENTERED horizontally inside the selection card with Font 2!
            spr.setTextDatum(MC_DATUM);
            spr.drawString(text, pillCX, yPos + 7, 2);
            spr.setTextDatum(TL_DATUM);
        }

        // Animated Robot Companion on the RIGHT side! (Looking LEFT at the centered choices!)
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 84, "ASSIST", "CONFIG ⚙", "● READY", COLOR_EMERALD, MORPH_GEAR);

        spr.pushSprite(0, 0);
        return;
    }

    // --- SCREENSAVER MODE: 100% FULL-SCREEN BORDERLESS DETAILED CLINICAL TELEMETRY SUMMARY ---
    bool inScreensaver = (!inMenuMode && !systemEnabled && abs(speed) <= 10 && (millis() - lastInteractionTime > 8000));
    static bool s_wasScreensaver = false;

    if (inScreensaver) {
        if (!s_wasScreensaver) {
            tft.fillScreen(COLOR_BG);
            s_wasScreensaver = true;
        }

        // Dedicated ANIMATING ROBOT SCREEN SAVER!
        // The morphing robot face glides between left and right bays and transforms into the surgical pen!
        drawMorphingStandbyScreen(spr, speed, volt, amp, rpm, pwr, isBatConnected, batPct, isCharging, count);

        spr.pushSprite(0, 0);
        return;
    } else {
        s_wasScreensaver = false;
    }

    // --- SHARED HEADER ZONE (TITLE -> LED -> ROBOT COMPANION -> BATTERY GAUGE) ---
    // Cleanly spaced across the width to guarantee ZERO overlap between Robot and Battery
    spr.fillSprite(COLOR_BG);
    spr.setTextSize(1);
    spr.setTextDatum(TL_DATUM);

    // 1. Header Title on Left
    spr.setTextColor(COLOR_GLOW_CYAN, COLOR_BG);
    const char* screenTitles[] = {
        "SURGICAL DASH",
        "ENERGY STATUS",
        "NET STATUS",
        "OSCILLATION",
        "DIAGNOSTICS",
        "PATIENT INFO",
        "PEN SUMMARY"
    };
    spr.drawString(screenTitles[screen], 6, 6, 2);

    // 2. LED Status Indicator in center (compact, non-overlapping Font 1)
    if (ledBrightness > 0) {
        uint8_t ledPct = round((float)ledBrightness * 100.0f / 255.0f);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_BG);
        spr.drawString("L:" + String(ledPct) + "%", 120, 14, 1);
        spr.setTextDatum(TL_DATUM);
    }

    // 3. Animated Robot Companion: placed at x = spr.width() - 100 (width: 30)
    // Occupies x = spr.width() - 100 to spr.width() - 70
    int8_t dir = (speed > 0) ? 1 : (speed < 0 ? -1 : 0);
    drawRobotFaceCompact(spr, spr.width() - 100, 5, (speed != 0), dir);

    // 4. Battery Icon & Percentage: occupies x = spr.width() - 60 to spr.width() - 2
    // A 10-pixel gap separates the robot face and the battery text (NO OVERWRITE!)
    drawBatteryIcon(spr, isBatConnected, isCharging, batPct);

    // Lowered Splitter Line at y=26 gives plenty of breathing room to the header!
    spr.drawFastHLine(0, 26, spr.width(), COLOR_GLOW_BLUE);
    spr.drawFastHLine(spr.width() / 2 - 25, 26, 50, COLOR_GLOW_CYAN);

        if (screen == 6) {
        // =====================================================================
        // SCREEN 6: FULL-SCREEN EXECUTIVE TELEMETRY SUMMARY
        // =====================================================================
        drawSummaryScreen(spr, speed, volt, amp, rpm, pwr, isBatConnected, batPct, isCharging, count);
        spr.pushSprite(0, 0);
        return;
    }

    if (screen == 0) {
        // =====================================================================
        // SCREEN 0: SURGICAL DRIVE DASHBOARD (PERFECT 50/50 HALF-AND-HALF SPLIT!)
        // Left Card: Surgical Drive Telemetry (50%) | Right Card: Motor Animation (50%)
        // =====================================================================
        int margin = 4;
        int gap = 6;
        int cardW = (spr.width() - (margin * 2) - gap) / 2; // Exact 50% width! (113px on 240, 153px on 320)
        int cardH = 84;
        int cardY = 28;

        int card1X = margin;
        int card2X = margin + cardW + gap;

        int cx1 = card1X + (cardW / 2); // Center of Left Card (x=60 on 240, x=80 on 320)
        int cx2 = card2X + (cardW / 2); // Center of Right Card (x=179 on 240, x=239 on 320)

        // ---------------------------------------------------------------------
        // 1. LEFT CARD (50%): SURGICAL DRIVE TELEMETRY
        // ---------------------------------------------------------------------
        spr.fillRoundRect(card1X, cardY, cardW, cardH, 6, COLOR_CARD);
        spr.drawRoundRect(card1X, cardY, cardW, cardH, 6, COLOR_DIVIDER);

        // Header Title Centered
        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("SURGICAL DRIVE", cx1, cardY + 2, 2);
        spr.drawFastHLine(card1X + 4, cardY + 17, cardW - 8, COLOR_DIVIDER);

        int percentSpeed = abs(speed);

        // Split-Column Centered Data Rows with Font 2
        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("Set Spd:", cx1 - 2, cardY + 20, 2);
        spr.drawString("Act RPM:", cx1 - 2, cardY + 35, 2);
        spr.drawString("Status:", cx1 - 2, cardY + 50, 2);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(String(percentSpeed) + "%", cx1 + 2, cardY + 20, 2);

        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString(String(rpm), cx1 + 2, cardY + 35, 2);

        if (speed != 0) {
            spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
            spr.drawString(speed > 0 ? "FWD" : "REV", cx1 + 2, cardY + 50, 2);
        } else {
            spr.setTextColor(COLOR_AMBER, COLOR_CARD);
            spr.drawString("IDLE", cx1 + 2, cardY + 50, 2);
        }

        // Sub-status mode tag with comfortable spacing (8px gap below Status line!)
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString(oscillatingMode ? "● OSCILLATING" : "● NORMAL DRIVE", cx1, cardY + 73, 1);
        spr.setTextDatum(TL_DATUM);

        // ---------------------------------------------------------------------
        // 2. RIGHT CARD (50%): MOTOR ANIMATION CARD (EXACT SAME SIZE & SHAPE!)
        // ---------------------------------------------------------------------
        spr.fillRoundRect(card2X, cardY, cardW, cardH, 6, COLOR_CARD);
        spr.drawRoundRect(card2X, cardY, cardW, cardH, 6, COLOR_DIVIDER);

        // Morphing Robot Avatar <-> Motor Turbine Engine
        uint32_t s0Time = millis() % 10000;
        float s0Morph = 0.0f;
        if (speed != 0) {
            s0Morph = 1.0f; // Lock to live turbine when motor is running!
        } else {
            if (s0Time < 3500) s0Morph = 0.0f;
            else if (s0Time < 5000) {
                float p = (s0Time - 3500) / 1500.0f;
                s0Morph = sin(p * 1.5707963f);
            } else if (s0Time < 8500) s0Morph = 1.0f;
            else {
                float p = (s0Time - 8500) / 1500.0f;
                s0Morph = cos(p * 1.5707963f);
            }
        }

        // Header Title Centered
        spr.setTextDatum(TC_DATUM);
        if (s0Morph < 0.5f) {
            spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
            spr.drawString("DRIVE BUDDY", cx2, cardY + 2, 2);
        } else {
            if (speed > 0) {
                spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
                spr.drawString("MOTOR CW ↻", cx2, cardY + 2, 2);
            } else if (speed < 0) {
                spr.setTextColor(COLOR_CORAL, COLOR_CARD);
                spr.drawString("MOTOR CCW ↺", cx2, cardY + 2, 2);
            } else {
                spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
                spr.drawString("MOTOR STANDBY", cx2, cardY + 2, 2);
            }
        }
        spr.drawFastHLine(card2X + 4, cardY + 17, cardW - 8, COLOR_DIVIDER);
        spr.setTextDatum(TL_DATUM);

        int cy = cardY + 49;

        if (s0Morph < 0.45f) {
            // --- ROBOT AVATAR FACE MODE (Eyes looking LEFT at Surgical Drive!) ---
            int vw = 44;
            int vh = 30;
            spr.fillRoundRect(cx2 - vw / 2, cy - vh / 2, vw, vh, 6, COLOR_BG);
            spr.drawRoundRect(cx2 - vw / 2, cy - vh / 2, vw, vh, 6, COLOR_GLOW_CYAN);

            // Ear nodes
            spr.fillRoundRect(cx2 - vw / 2 - 4, cy - 6, 4, 12, 2, COLOR_GLOW_BLUE);
            spr.fillRoundRect(cx2 + vw / 2, cy - 6, 4, 12, 2, COLOR_GLOW_BLUE);

            uint32_t bt = millis() % 3600;
            bool isBlink = (bt < 150);

            int lx = cx2 - 9;
            int rx = cx2 + 9;
            int ey = cy - 2;

            if (isBlink) {
                spr.drawFastHLine(lx - 4, ey, 9, COLOR_GLOW_CYAN);
                spr.drawFastHLine(rx - 4, ey, 9, COLOR_GLOW_CYAN);
            } else {
                spr.fillCircle(lx, ey, 4, COLOR_GLOW_CYAN);
                spr.fillCircle(rx, ey, 4, COLOR_GLOW_CYAN);
                // Gaze pupil looking LEFT at the telemetry card
                spr.fillCircle(lx - 2, ey, 2, COLOR_BG);
                spr.fillCircle(rx - 2, ey, 2, COLOR_BG);
                spr.drawPixel(lx - 1, ey - 1, COLOR_TXT_WHITE);
                spr.drawPixel(rx - 1, ey - 1, COLOR_TXT_WHITE);
            }

            // Blushing pink cheeks
            spr.drawPixel(cx2 - 14, cy + 6, COLOR_PINK);
            spr.drawPixel(cx2 + 14, cy + 6, COLOR_PINK);

            // Smiling mouth
            spr.drawFastHLine(cx2 - 4, cy + 8, 9, COLOR_AMBER);
            spr.drawPixel(cx2 - 5, cy + 7, COLOR_AMBER);
            spr.drawPixel(cx2 + 5, cy + 7, COLOR_AMBER);

        } else {
            // --- MOTOR TURBINE MODE ---
            int r = 24;

            // Outer Calibration Dial with 8 radial precision tick marks
            spr.drawCircle(cx2, cy, r + 4, COLOR_DIVIDER);
            for (int a = 0; a < 360; a += 45) {
                float rad = a * DEG_TO_RAD;
                int x1 = cx2 + (int)((r + 2) * cos(rad));
                int y1 = cy + (int)((r + 2) * sin(rad));
                int x2 = cx2 + (int)((r + 4) * cos(rad));
                int y2 = cy + (int)((r + 4) * sin(rad));
                spr.drawLine(x1, y1, x2, y2, (a % 90 == 0) ? COLOR_GLOW_CYAN : COLOR_DIVIDER);
            }

            spr.drawCircle(cx2, cy, r, (speed != 0) ? (speed > 0 ? COLOR_EMERALD : COLOR_CORAL) : COLOR_DIVIDER);

            static float currentAngle = 0;
            if (speed != 0) {
                currentAngle += (speed / 100.0f) * 16.0f;
            } else {
                currentAngle += 3.0f; // Smooth idle rotation
            }
            if (currentAngle >= 360.0f) currentAngle -= 360.0f;
            if (currentAngle < 0.0f) currentAngle += 360.0f;

            for (int i = 0; i < 3; i++) {
                float aTip = (currentAngle + (i * 120)) * DEG_TO_RAD;
                float aTail = (currentAngle + (i * 120) - (speed >= 0 ? 30 : -30)) * DEG_TO_RAD;

                int xt = cx2 + (int)((r - 1) * cos(aTip));
                int yt = cy + (int)((r - 1) * sin(aTip));
                int xm = cx2 + (int)((r - 8) * cos(aTail));
                int ym = cy + (int)((r - 8) * sin(aTail));
                int xr = cx2 + (int)(7 * cos(aTip - 0.2));
                int yr = cy + (int)(7 * sin(aTip - 0.2));

                uint16_t bCol = (speed != 0) ? (speed > 0 ? COLOR_EMERALD : COLOR_CORAL) : COLOR_TXT_MUTED;
                uint16_t sCol = (speed != 0) ? COLOR_GLOW_CYAN : COLOR_DIVIDER;

                spr.drawLine(xt, yt, xm, ym, bCol);
                spr.drawLine(xm, ym, xr, yr, sCol);
                spr.drawLine(xt, yt, xr, yr, bCol);

                if (speed != 0) {
                    float aTrail = (currentAngle + (i * 120) - (speed >= 0 ? 50 : -50)) * DEG_TO_RAD;
                    int xp = cx2 + (int)((r - 1) * cos(aTrail));
                    int yp = cy + (int)((r - 1) * sin(aTrail));
                    spr.drawPixel(xp, yp, COLOR_GLOW_CYAN);
                }
            }

            spr.fillCircle(cx2, cy, 7, COLOR_CARD);
            spr.drawCircle(cx2, cy, 7, COLOR_GLOW_BLUE);
            spr.fillCircle(cx2, cy, 3, (speed != 0) ? (speed > 0 ? COLOR_GLOW_CYAN : COLOR_AMBER) : COLOR_TXT_MUTED);
        }

        // ---------------------------------------------------------------------
        // 3. FOOTER ZONE (SYMMETRICALLY BALANCED)
        // ---------------------------------------------------------------------
        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextColor(COLOR_AMBER, COLOR_BG);
        spr.drawString("GRAFTS: " + String(count) + " completed", 6, 118, 2);

        if (speed != 0) {
            spr.setTextColor(speed > 0 ? COLOR_GLOW_CYAN : COLOR_CORAL, COLOR_BG);
            spr.setTextDatum(TR_DATUM);
            static uint8_t arrowFrame = 0;
            arrowFrame = (arrowFrame + 1) % 4;
            String arrowStr = "";
            if (speed > 0) {
                for (int a = 0; a < 3; a++) arrowStr += (a == arrowFrame) ? ">" : "»";
                spr.drawString("FWD " + arrowStr, spr.width() - 6, 118, 2);
            } else {
                for (int a = 0; a < 3; a++) arrowStr += (a == (3 - arrowFrame)) ? "<" : "«";
                spr.drawString(arrowStr + " REV", spr.width() - 6, 118, 2);
            }
        } else {
            spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
            spr.setTextDatum(TR_DATUM);
            spr.drawString("IDLE", spr.width() - 6, 118, 2);
        }
        spr.setTextDatum(TL_DATUM);
    } else if (screen == 1) {
        // =====================================================================
        // SCREEN 1: ENERGY STATUS (DATA CENTERED ON LEFT, ROBOT ON RIGHT)
        // =====================================================================
        int cardW = spr.width() - 64;
        int cardCX = 4 + (cardW / 2); // Exact horizontal center of data card!

        // Left Data Card
        spr.fillRoundRect(4, 30, cardW, 82, 6, COLOR_CARD);
        spr.drawRoundRect(4, 30, cardW, 82, 6, COLOR_DIVIDER);

        // Card Title Centered
        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("ENERGY STATS", cardCX, 33, 2);
        spr.drawFastHLine(10, 48, cardW - 12, COLOR_DIVIDER);

        // Split-Column Centered Data Rows with Font 2
        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("TTGO ADC:", cardCX - 4, 51, 2);
        spr.drawString("INA Volt:", cardCX - 4, 66, 2);
        spr.drawString("INA Curr:", cardCX - 4, 81, 2);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString(String(internalVoltage, 2) + " V", cardCX + 4, 51, 2);

        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString(String(volt, 2) + " V", cardCX + 4, 66, 2);

        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(String(amp, 3) + " A", cardCX + 4, 81, 2);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_TXT_WHITE, COLOR_CARD);
        spr.drawString("Pwr: " + String(pwr, 1) + "W (" + String(batPct) + "%)", cardCX, 100, 1);
        spr.setTextDatum(TL_DATUM);

        // Right Robot Companion Pod ("POWER BUDDY" looking LEFT at the energy readings!)
        const char* pBadge = isCharging ? "⚡ CHG" : (batPct < 20 ? "LOW BAT" : "BAT OK");
        uint16_t bColor = isCharging ? COLOR_EMERALD : (batPct < 20 ? COLOR_CORAL : COLOR_EMERALD);
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 82, "POWER", "⚡ VOLT", pBadge, bColor, MORPH_LIGHTNING);

        // Lower Status Banner
        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextDatum(MC_DATUM);
        if (isCharging) {
            spr.setTextColor(COLOR_EMERALD, COLOR_BG);
            spr.drawString("⚡ CHARGING (USB ACTIVE)", spr.width() / 2, 122, 1);
        } else {
            spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
            spr.drawString("🔋 DISCHARGING (BATTERY ACTIVE)", spr.width() / 2, 122, 1);
        }
        spr.setTextDatum(TL_DATUM);

    } else if (screen == 2) {
        // =====================================================================
        // SCREEN 2: NETWORK STATUS (DATA CENTERED ON LEFT, ROBOT ON RIGHT)
        // =====================================================================
        int cardW = spr.width() - 64;
        int cardCX = 4 + (cardW / 2);

        // Left Data Card
        spr.fillRoundRect(4, 30, cardW, 82, 6, COLOR_CARD);
        spr.drawRoundRect(4, 30, cardW, 82, 6, COLOR_DIVIDER);

        // Title Centered
        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("NETWORK STATS", cardCX, 33, 2);
        spr.drawFastHLine(10, 48, cardW - 12, COLOR_DIVIDER);

        String ssidName = "AP Mode";
        if (WiFi.status() == WL_CONNECTED) ssidName = WiFi.SSID();
        else if (wifiConnecting) ssidName = "Connecting...";

        if (ssidName.length() > 14) ssidName = ssidName.substring(0, 12) + "..";

        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("SSID:", cardCX - 4, 52, 2);
        spr.drawString("IP:", cardCX - 4, 68, 2);
        spr.drawString("URL:", cardCX - 4, 84, 2);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(ssidName, cardCX + 4, 52, 2);
        spr.drawString(ip, cardCX + 4, 68, 2);
        spr.drawString("elbaseet.local", cardCX + 4, 84, 2);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString(String(clients) + " Active Client(s)", cardCX, 102, 1);
        spr.setTextDatum(TL_DATUM);

        // Right Robot Companion Pod ("NET BUDDY" looking LEFT at IP & URL!)
        const char* nBadge = (WiFi.status() == WL_CONNECTED) ? "ONLINE" : "AP READY";
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 82, "NETWORK", "WIFI 📶", nBadge, COLOR_EMERALD, MORPH_WIFI);

        // Lower Status Banner
        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
        spr.drawString("Web Control: http://elbaseet.local", spr.width() / 2, 122, 1);
        spr.setTextDatum(TL_DATUM);

    } else if (screen == 3) {
        // =====================================================================
        // SCREEN 3: OSCILLATION SETTINGS (DATA CENTERED ON LEFT, ROBOT ON RIGHT)
        // =====================================================================
        int cardW = spr.width() - 64;
        int cardCX = 4 + (cardW / 2);

        spr.fillRoundRect(4, 30, cardW, 82, 6, COLOR_CARD);
        spr.drawRoundRect(4, 30, cardW, 82, 6, COLOR_DIVIDER);

        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("OSCILLATION", cardCX, 33, 2);
        spr.drawFastHLine(10, 48, cardW - 12, COLOR_DIVIDER);

        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("Mode:", cardCX - 4, 52, 2);
        spr.drawString("CW Time:", cardCX - 4, 68, 2);
        spr.drawString("CCW Time:", cardCX - 4, 84, 2);

        spr.setTextDatum(TL_DATUM);
        if (oscMode) {
            spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
            spr.drawString("OSCILLATING", cardCX + 4, 52, 2);
        } else {
            spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
            spr.drawString("CONTINUOUS", cardCX + 4, 52, 2);
        }

        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(String(oscCW) + " ms", cardCX + 4, 68, 2);
        spr.drawString(String(oscCCW) + " ms", cardCX + 4, 84, 2);

        uint32_t totalPeriod = oscCW + oscCCW;
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString("Total Period: " + String(totalPeriod) + " ms", cardCX, 102, 1);
        spr.setTextDatum(TL_DATUM);

        // Right Robot Companion Pod
        const char* oBadge = oscMode ? "BI-DIR" : "UNI-DIR";
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 82, "CYCLE", "OSC ∿", oBadge, COLOR_GLOW_BLUE, MORPH_OSCILLATION);

        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
        spr.drawString(oscMode ? "Bi-Directional Dissection Active" : "Unidirectional Precision Drive Active", spr.width() / 2, 122, 1);
        spr.setTextDatum(TL_DATUM);

    } else if (screen == 4) {
        // =====================================================================
        // SCREEN 4: DIAGNOSTICS & THRESHOLDS (DATA CENTERED ON LEFT, ROBOT ON RIGHT)
        // =====================================================================
        int cardW = spr.width() - 64;
        int cardCX = 4 + (cardW / 2);

        spr.fillRoundRect(4, 30, cardW, 82, 6, COLOR_CARD);
        spr.drawRoundRect(4, 30, cardW, 82, 6, COLOR_DIVIDER);

        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("DIAGNOSTICS", cardCX, 33, 2);
        spr.drawFastHLine(10, 48, cardW - 12, COLOR_DIVIDER);

        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("Base I:", cardCX - 4, 52, 2);
        spr.drawString("Spike I:", cardCX - 4, 68, 2);
        spr.drawString("Reset I:", cardCX - 4, 84, 2);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString(String(baselineCurrent, 3) + " A", cardCX + 4, 52, 2);

        spr.setTextColor(COLOR_CORAL, COLOR_CARD);
        spr.drawString(String(baselineCurrent + penetrationOffset, 3) + " A", cardCX + 4, 68, 2);
        spr.drawString(String((baselineCurrent + penetrationOffset) - penetrationHysteresis, 3) + " A", cardCX + 4, 84, 2);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("Sens: " + String(penetrationOffset, 3) + "A | Hys: " + String(penetrationHysteresis, 3) + "A", cardCX, 102, 1);
        spr.setTextDatum(TL_DATUM);

        // Right Robot Companion Pod
        const char* dBadge = calibrationNeeded ? "CALIB" : "CAL OK";
        uint16_t cColor = calibrationNeeded ? COLOR_CORAL : COLOR_EMERALD;
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 82, "SENSORS", "ECG 💓", dBadge, cColor, MORPH_DIAGNOSTICS);

        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_BG);
        spr.drawString(calibrationNeeded ? "Calibration Required: Keep Pen in Air" : "Sensor Baseline Stable & Calibrated", spr.width() / 2, 122, 1);
        spr.setTextDatum(TL_DATUM);

    } else {
        // =====================================================================
        // SCREEN 5: PATIENT CLINICAL INFO (DATA CENTERED ON LEFT, ROBOT ON RIGHT)
        // =====================================================================
        int cardW = spr.width() - 64;
        int cardCX = 4 + (cardW / 2);

        spr.fillRoundRect(4, 30, cardW, 82, 6, COLOR_CARD);
        spr.drawRoundRect(4, 30, cardW, 82, 6, COLOR_DIVIDER);

        spr.setTextDatum(TC_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString("PATIENT RECORD", cardCX, 33, 2);
        spr.drawFastHLine(10, 48, cardW - 12, COLOR_DIVIDER);

        String shortName = pName;
        if (shortName.length() > 14) shortName = shortName.substring(0, 12) + "..";

        spr.setTextDatum(TR_DATUM);
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("Patient:", cardCX - 4, 52, 2);
        spr.drawString("Age / Gen:", cardCX - 4, 68, 2);
        spr.drawString("Nation:", cardCX - 4, 84, 2);

        spr.setTextDatum(TL_DATUM);
        spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
        spr.drawString(shortName, cardCX + 4, 52, 2);
        spr.drawString(pAge, cardCX + 4, 68, 2);
        spr.drawString(pNat, cardCX + 4, 84, 2);

        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString("● Clinical Session Active", cardCX, 102, 1);
        spr.setTextDatum(TL_DATUM);

        // Right Robot Companion Pod
        drawUniversalMorphingPod(spr, spr.width() - 58, 30, 54, 82, "RECORD", "CLINIC ✚", "LOGGED", COLOR_EMERALD, MORPH_PATIENT);

        spr.drawFastHLine(0, 115, spr.width(), COLOR_GLOW_BLUE);
        spr.setTextDatum(MC_DATUM);
        spr.setTextColor(COLOR_AMBER, COLOR_BG);
        spr.drawString("Clinical Session Active - Telemetry Logged", spr.width() / 2, 122, 1);
        spr.setTextDatum(TL_DATUM);
    }

    spr.pushSprite(0, 0);
}

/**
 * @brief Displays a temporary confirmation message on the screen.
 * @param msg The message string to display.
 */
void displayWebConfirmation(String msg) {
    s_confirmationMessage = msg;
    s_confirmationTimer = millis();
}

/**
 * @brief Displays a confirmation message specifically for graft detection.
 * @param count The new graft count to display.
 */
void displayGraftFlash(uint32_t count) {
    s_flashMessage = String(count);
    s_flashTimer = millis();
}

/**
 * @brief Renders one frame of the report generation countdown on the TTGO TFT screen.
 * @param secondsRemaining Number of seconds left (5, 4, 3, 2, 1, 0)
 * @param totalSeconds Total duration in seconds (default 5)
 */
void drawReportCountdownFrame(int secondsRemaining, int totalSeconds) {
    if (!s_oledInitialized) return;

    spr.fillSprite(COLOR_BG);

    // Glowing Sapphire Card with Double Neon Border
    spr.fillRoundRect(8, 6, spr.width() - 16, spr.height() - 12, 8, COLOR_CARD);
    spr.drawRoundRect(8, 6, spr.width() - 16, spr.height() - 12, 8, COLOR_GLOW_CYAN);
    spr.drawRoundRect(9, 7, spr.width() - 18, spr.height() - 14, 7, COLOR_GLOW_BLUE);

    // Header Tag
    spr.setTextDatum(TC_DATUM);
    spr.setTextFont(1);
    spr.setTextSize(1);
    spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
    spr.drawString("SYSTEM NOTIFICATION", spr.width() / 2, 12, 2);

    // Subtitle Tag
    spr.setTextColor(COLOR_GLOW_CYAN, COLOR_CARD);
    spr.drawString("REPORT GENERATING", spr.width() / 2, 28, 2);

    // Message
    spr.setTextColor(COLOR_TXT_MAIN, COLOR_CARD);
    spr.drawString("Please wait, generating...", spr.width() / 2, 46, 2);

    // Countdown / Status Value
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(FF23); // FreeSansBold18pt7b
    if (secondsRemaining > 0) {
        spr.setTextColor(COLOR_AMBER, COLOR_CARD);
        spr.drawString(String(secondsRemaining) + "s", spr.width() / 2, 74);
    } else {
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString("READY!", spr.width() / 2, 74);
    }

    // Progress Bar
    int barW = spr.width() - 48;
    int barH = 6;
    int barX = 24;
    int barY = spr.height() - 30;
    spr.fillRoundRect(barX, barY, barW, barH, 3, COLOR_BG);
    spr.drawRoundRect(barX, barY, barW, barH, 3, COLOR_DIVIDER);

    int progress = map(totalSeconds - secondsRemaining, 0, totalSeconds, 0, barW);
    if (progress > 2) {
        spr.fillRoundRect(barX + 1, barY + 1, progress - 2, barH - 2, 2, COLOR_GLOW_CYAN);
    }

    // Bottom Status Pill
    spr.setTextFont(1);
    spr.setTextSize(1);
    spr.setTextDatum(BC_DATUM);
    if (secondsRemaining > 0) {
        spr.setTextColor(COLOR_TXT_MUTED, COLOR_CARD);
        spr.drawString("● SAVING TO STORAGE...", spr.width() / 2, spr.height() - 10, 1);
    } else {
        spr.setTextColor(COLOR_EMERALD, COLOR_CARD);
        spr.drawString("● REPORT GENERATED", spr.width() / 2, spr.height() - 10, 1);
    }

    spr.pushSprite(0, 0);
}

#endif // DISPLAY_H
