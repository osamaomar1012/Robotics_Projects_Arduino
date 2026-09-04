/**
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */

// Module Imports
#include "drv8833.h"
#include "max471.h"
#include "display.h"
#include "web_server.h"
#include <SPIFFS.h>
#include <TJpg_Decoder.h>
#include <Preferences.h>
#include <time.h>

// LILYGO T-Display-S3 Button Definitions
#define BTN_LEFT  0   // BOOT Button (GPIO 0)
#define BTN_RIGHT 14  // Key Button (GPIO 14)

// JGA12-N20 Encoder Definitions & Wire Colors
// - Red Wire    -> Motor Positive (+)  -> DRV8833 OUT1
// - White Wire  -> Motor Negative (-)  -> DRV8833 OUT2
// - Black Wire  -> Encoder VCC (3.3V)  -> ESP32 3V3
// - Blue Wire   -> Encoder GND         -> ESP32 GND
// - Yellow Wire -> Encoder Channel A   -> ESP32 GPIO 10 (Interrupt pin)
// - Green Wire  -> Encoder Channel B   -> ESP32 GPIO 11 (State/Direction pin)
#define ENCODER_PIN_A 10
#define ENCODER_PIN_B 11
#define ENCODER_PPR   360.0f  // Gearmotor pulses per output shaft revolution (7 CPR * Gear Ratio)
volatile long encoderTicks = 0;
uint8_t motorVersion = 1;      // 1 = Version 1 (Motor + Encoder), 2 = Version 2 (Motor without Encoder)

void IRAM_ATTR encoderISR() {
    if (digitalRead(ENCODER_PIN_B) == HIGH) {
        encoderTicks++;
    } else {
        encoderTicks--;
    }
}

void setMotorVersion(uint8_t version) {
    motorVersion = version;
    preferences.putUChar("motVer", motorVersion);
    if (motorVersion == 1) {
        // --- Version 1: Motor + Encoder ---
        pinMode(ENCODER_PIN_A, INPUT_PULLUP);
        pinMode(ENCODER_PIN_B, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, RISING);
        Serial.println(F("Motor Version Configured: Version 1 (Motor + Encoder)"));
    } else {
        // --- Version 2: Motor without Encoder ---
        detachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A));
        pinMode(ENCODER_PIN_A, INPUT); // Float to save power/drain
        pinMode(ENCODER_PIN_B, INPUT);
        baselineRPM = 0.0;
        Serial.println(F("Motor Version Configured: Version 2 (Motor without Encoder)"));
    }
}

// Global Variables
TaskHandle_t TelemetryTaskHandle;        // FreeRTOS background task handle
float maxCurrentThreshold = 2.5; // Safety limit in Amperes (adjust to motor specs)
float minVoltageThreshold = 3.2; // 18650 safe discharge limit (Volts)
int16_t motorSpeed = 0;   // Signed actual speed
int16_t targetSpeed = 0;  // Signed target speed
float batteryVoltage = 0.0;
float batteryCurrent = 0.0;
float batteryPower = 0.0;
float internalVoltage = 0.0;     // Board-level internal ADC voltage (GPIO 34)
bool isCharging = false;          // Dynamic USB charging state
unsigned long lastUpdate = 0;

// Penetration Counter Calibration Variables
float baselineCurrent = 0.0; // Dynamically calibrated current at current speed
float baselineRPM = 0.0;     // Dynamically calibrated baseline idle RPM from N20 encoder
float speedDipThresholdPercent = 10.0; // Drop percentage in RPM to confirm graft penetration
bool calibrationNeeded = true;
unsigned long calibrationStartTime = 0;
float calibrationSum = 0.0;
float calibrationRPMSum = 0.0; // Accumulator for baseline RPM calibration
uint16_t calibrationSamples = 0;
uint32_t estimatedRPM = 0;
float penetrationOffset = 0.04;        // Amps above baseline to trigger
float penetrationHysteresis = 0.02;    // Amps below spike to reset
uint32_t penetrationCount = 0;
bool isPenetrating = false;
uint8_t batteryPercent = 0;
uint8_t webBaseSpeed = 200;    // Dynamic magnitude from web
uint32_t screenSwitchTime = 5000; // Default 5 seconds
bool forwardDirection = true;  // Toggle state
uint8_t currentScreen = 0;      // Re-enable Screen 0
unsigned long lastScreenSwitch = 0;
bool safetyTripped = false; // Flag to indicate a safety shutdown
bool lowBatteryTripped = false; // Flag for low voltage protection
bool systemEnabled = false; // Default to OFF for safety on boot
bool debugMode = false;     // Engineering mode to bypass safety stops
bool oscillatingMode = false; // Master flag for oscillation mode
uint32_t oscillationDuration = 2000; // Default total duration for web UI compatibility
bool wifiConnecting = false; // Flag to indicate if STA connection is in progress
bool inMenuMode = false;      // True if navigating the on-device menu
uint8_t currentMenuItem = 0;  // Current selected menu option (0-4)
Preferences preferences;

// --- Oscillation Control Variables ---
uint32_t oscillationDurationCW = 2000; // Duration for Clockwise rotation (ms)
uint32_t oscillationDurationCCW = 2000; // Duration for Counter-Clockwise rotation (ms)
bool oscillationDirectionFwd = true;   // Current direction in oscillation cycle
unsigned long lastOscillationSwitch = 0; // Timer for oscillation

// --- Button State Variables for Long/Short Press ---
unsigned long btnLeftPressTime = 0;
unsigned long btnRightPressTime = 0;
bool btnLeftLongPressHandled = false;
bool btnRightLongPressHandled = false;
unsigned long bothButtonsPressTime = 0;
bool bothButtonsLongPressHandled = false;

// --- WiFi Scan State Machine ---
volatile ScanState scanState = SCAN_IDLE;
String initialScanResultsJson = "[]"; // Cache for startup scan results

// --- Operation State Machine ---
OperationState operationState = IDLE;

// --- Report Generation Variables ---
String patientName = "N/A";
String patientAge = "N/A";
String patientNationality = "N/A";
String patientMobile = "N/A";
String doctorName = "N/A";
unsigned long operationTimeAccumulator = 0; // in milliseconds
unsigned long lastTimeCapture = 0;
uint32_t pauseCount = 0;
bool wasSystemEnabled = false; // To detect transitions for pause counting

// --- NTP Time Variables ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // Use 0 for UTC, which is standard for logging
const int   daylightOffset_sec = 0;
bool timeSynchronized = false; // Flag to ensure we only sync time once per connection


// --- Function Prototypes for new functions ---
void handleSafetyChecks();
void handleButtons();
void handleMotorControl();
void handleCalibration();
void calculateTelemetry();
void handleGraftCounter();
void updateUI();
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
String getFormattedDate();
void showAnimatedLogo();
void displayGraftFlash(uint32_t count); // Forward declare the new function

/**
 * @brief Callback function for the TJpg_Decoder library.
 * This function is called by the decoder to render blocks of the JPEG image.
 */
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

/**
 * @brief Displays an animated logo by iterating through JPEG files in SPIFFS.
 */
void showAnimatedLogo() {
    tft.fillScreen(TFT_BLACK);

    // --- Setup for text and JPEG decoding ---
    tft.setTextDatum(MC_DATUM); // Middle-Center datum for easy centering
    // Use a smoother vector font instead of the default scaled-up font.
    tft.setTextFont(2);

    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // --- Strings for the typewriter effect ---
    String line1 = "EL-BASEET";
    String line2 = "HAIR PEN";
    int totalChars = line1.length() + line2.length();
    int framesPerChar = 111 / totalChars; // Draw one char every ~6 frames

    // --- Main animation and typewriter loop ---
    for (int i = 1; i <= 111; i++) {
        // 1. Draw the current animation frame on the left
        TJpgDec.drawFsJpg(0, 3, "/Image_" + String(i) + ".jpg");

        // 2. Calculate how many characters of text should be visible
        int charsToShow = i / framesPerChar;

        // 3. Draw the first line of text
        if (charsToShow > 0) {
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.drawString(line1.substring(0, min(charsToShow, (int)line1.length())), 184, 45);
        }
        // 4. Draw the second line of text
        if (charsToShow > line1.length()) {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.drawString(line2.substring(0, charsToShow - line1.length()), 184, 75);
        }
    }
}

/**
 * @brief High-priority background task pinned to Core 0 for real-time telemetry, 
 *        motor overcurrent safety protection, speed control, and sensor fusion graft counting.
 */
void telemetryTaskCode(void * pvParameters) {
    Serial.printf("Real-time telemetry and safety task running on Core: %d\n", xPortGetCoreID());
    
    // We execute the telemetry/safety loop at 100Hz (every 10ms)
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); 
    
    while (true) {
        // Block until exactly 10ms have elapsed
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // 1. Read calibrated electrical telemetry (Current and Voltage)
        readMAX471(batteryVoltage, batteryCurrent);
        
        // 2. Perform fast safety checks (Overcurrent and Low Voltage trips)
        handleSafetyChecks();
        
        // 3. Perform speed control calculations and ramp outputs
        handleMotorControl();
        
        // 4. Sample baseline variables for self-calibration
        handleCalibration();
        
        // 5. Evaluate motor RPM and battery state of charge
        calculateTelemetry();
        
        // 6. Run the Sensor-Fusion graft counter algorithm
        handleGraftCounter();
    }
}

void setup() {
    Serial.begin(115200);

    // Power on T-Display-S3 onboard peripherals (LCD Screen, Battery divider)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH); // Keeping HIGH is required to power the onboard LCD screen!
    
    // Enable LCD Backlight (GPIO 38) for battery power keep-alive
    pinMode(38, OUTPUT);
    digitalWrite(38, HIGH); // Turn on the backlight!
    
    delay(50); // Allow power rail to stabilize
    
    // Initialize NVM Storage
    preferences.begin("hpp_v1", false);
    
    // Initialize Characterized ADC Calibration
    setupADCCalibration();
    
    // Load saved values, or use defaults if not found
    webBaseSpeed = preferences.getUChar("motSpd", 200);
    screenSwitchTime = preferences.getUInt("scrTime", 5000);
    if (screenSwitchTime < 1000) screenSwitchTime = 5000; // Sanity check for rotation
    maxCurrentThreshold = preferences.getFloat("maxCurr", 2.5);
    minVoltageThreshold = preferences.getFloat("minVolt", 3.2);
    penetrationCount = preferences.getUInt("pCnt", 0);
    penetrationOffset = preferences.getFloat("penOff", 0.04);
    penetrationHysteresis = preferences.getFloat("penHyst", 0.02);
    speedDipThresholdPercent = preferences.getFloat("spdDip", 10.0); // Load speed dip threshold
    debugMode = preferences.getBool("debug", false);
    oscillationDurationCW = preferences.getUInt("oscDurCw", 2000);
    oscillationDurationCCW = preferences.getUInt("oscDurCcw", 2000);
    
    // Initialize Display
    setupOLED();

    // Initialize TTGO internal battery ADC enable pin
    pinMode(14, OUTPUT);
    digitalWrite(14, LOW); // Disable to save power initially

    // Load and initialize Motor Version (1 = Version 1: Motor + Encoder, 2 = Version 2: Motor without Encoder)
    motorVersion = preferences.getUChar("motVer", 1); // Default to 1
    setMotorVersion(motorVersion);

    // Initialize SPIFFS and play animation
    if (SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mounted successfully.");
        showAnimatedLogo();
    } else {
        Serial.println("An Error has occurred while mounting SPIFFS");
        showWelcomeLogo(); // Fallback to static logo
    }

    // Initialize the motor driver pins
    setupDRV8833();

    // Setup button pins with internal pull-ups
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    
    // --- Perform Initial WiFi Scan at Startup ---
    Serial.println("Performing initial WiFi scan...");
    WiFi.mode(WIFI_AP_STA); // Ensure correct mode for scanning
    int n = WiFi.scanNetworks(); // Synchronous scan
    Serial.printf("Initial scan found %d networks.\n", n);
    if (n > 0) {
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
        json += "]";
        initialScanResultsJson = json;
    }
    WiFi.scanDelete(); // Free memory after caching results
    // --- End Initial Scan ---

    delay(500); // Breathe before WiFi
    setupWeb(); 

    // Initial sensor read to seed the EMA filter
    readMAX471(batteryVoltage, batteryCurrent);

    // Load report data from NVM
    patientName = preferences.getString("pName", "N/A");
    patientAge = preferences.getString("pAge", "N/A");
    patientNationality = preferences.getString("pNat", "N/A");
    patientMobile = preferences.getString("pMob", "N/A");
    doctorName = preferences.getString("pDoc", "N/A");

    // Spin up real-time telemetry task pinned to Core 0 (Priority 3 - Higher than normal)
    xTaskCreatePinnedToCore(
        telemetryTaskCode,       /* Task function. */
        "TelemetryTask",         /* name of task. */
        8192,                    /* Stack size of task (8KB is generous and safe) */
        NULL,                    /* parameter of the task */
        3,                       /* priority of the task */
        &TelemetryTaskHandle,    /* Task handle to keep track of created task */
        0                        /* pin task to Core 0 */
    );
    Serial.println(F("Real-time telemetry and safety task launched on Core 0."));

    // Synchronize timers to prevent immediate execution of loop logic
    lastUpdate = millis();
    lastScreenSwitch = millis();
}

void loop() {
    server.handleClient(); // Process web requests on Core 1

    // Non-blocking timed loop. Executes approximately every 100ms on Core 1.
    // Pinning UI/Buttons here allows the web server to handle loads smoothly.
    const unsigned long LOOP_INTERVAL = 100;
    if (millis() - lastUpdate >= LOOP_INTERVAL) {
        lastUpdate += LOOP_INTERVAL; // Move the timer forward by a fixed interval

        // --- Robust WiFi Status Handling ---
        static unsigned long lastWifiCheck = 0;
        if (millis() - lastWifiCheck > 5000) { // Check every 5 seconds
            lastWifiCheck = millis();
            String saved_ssid = preferences.getString("sta_ssid", "");
            if (saved_ssid.length() > 0 && WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi is disconnected. Attempting to reconnect...");
                wifiConnecting = true;
                WiFi.reconnect();
                timeSynchronized = false; // Reset flag on disconnect
            } else if (WiFi.status() == WL_CONNECTED) {
                wifiConnecting = false; // Ensure flag is false when connected
                if (!timeSynchronized) {
                    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                    timeSynchronized = true; // Set flag to prevent re-syncing
                }
            }
        }

        // --- WiFi Scan State Machine ---
        if (scanState == SCAN_REQUESTED) {
            Serial.println("Scan requested. Stopping AP and starting scan.");
            WiFi.softAPdisconnect(true); // Stop AP to free radio
            delay(100);
            WiFi.scanNetworks(true); // Start ASYNC scan
            scanState = SCANNING;
        } else if (scanState == SCANNING) {
            int8_t scanResult = WiFi.scanComplete();
            if (scanResult >= 0) {
                Serial.printf("Scan complete. %d networks found.\n", scanResult);
                scanState = SCAN_COMPLETE;
                // Restart the AP immediately so the client can reconnect
                WiFi.softAP(ssid_ap, pass_ap);
                Serial.println("AP restarted at: " + WiFi.softAPIP().toString());
            } else {
                Serial.println("...scanning...");
            }
        }

        handleButtons(); // Check for button presses on Core 1
        updateUI();      // Refresh local TFT Display on Core 1

        // Debug Heartbeat: Verifies the code hasn't frozen
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 5000) { Serial.println(F("System Running...")); lastHeartbeat = millis(); }
    }
}

void handleButtons() {
    const long LONG_PRESS_DURATION = 1000; // 1s for long press
    const long DEBOUNCE_DELAY = 50;      // 50ms debounce

    bool btnLeftState = (digitalRead(BTN_LEFT) == LOW);
    bool btnRightState = (digitalRead(BTN_RIGHT) == LOW);

    // --- 1. Check for Dual Long Press (Menu Mode Toggle) ---
    if (btnLeftState && btnRightState) {
        if (bothButtonsPressTime == 0) {
            bothButtonsPressTime = millis();
        } else if (!bothButtonsLongPressHandled && (millis() - bothButtonsPressTime > LONG_PRESS_DURATION)) {
            inMenuMode = !inMenuMode;
            displayWebConfirmation(inMenuMode ? "Menu Opened" : "Menu Closed");
            Serial.println("DUAL LONG PRESS: Menu Mode toggled to " + String(inMenuMode ? "OPEN" : "CLOSED"));
            bothButtonsLongPressHandled = true; // Prevent re-triggering
        }
        return; // Prioritize dual press, skip single button logic
    } else {
        bothButtonsPressTime = 0;
        bothButtonsLongPressHandled = false;
    }

    // --- 2. Handle Menu Mode Navigation & Control ---
    if (inMenuMode) {
        // Left Button (Scroll Down)
        if (btnLeftState) {
            if (btnLeftPressTime == 0) {
                btnLeftPressTime = millis();
            } else if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > LONG_PRESS_DURATION)) {
                // Left button long-press exits the menu
                inMenuMode = false;
                displayWebConfirmation("Menu Closed");
                btnLeftLongPressHandled = true;
            }
        } else if (btnLeftPressTime > 0) {
            if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > DEBOUNCE_DELAY)) {
                // Short press moves selection down
                currentMenuItem = (currentMenuItem + 1) % 5;
            }
            btnLeftPressTime = 0; btnLeftLongPressHandled = false;
        }

        // Right Button (Modify/Select Item)
        if (btnRightState) {
            if (btnRightPressTime == 0) btnRightPressTime = millis();
        } else if (btnRightPressTime > 0) {
            if (millis() - btnRightPressTime > DEBOUNCE_DELAY) {
                // Short press triggers selected menu action
                if (currentMenuItem == 0) {
                    // Adjust speed (Speed wraps 50 -> 250 -> 50)
                    webBaseSpeed += 10;
                    if (webBaseSpeed > 250) webBaseSpeed = 50;
                    preferences.putUChar("motSpd", webBaseSpeed);
                    calibrationNeeded = true;
                    displayWebConfirmation("Speed: " + String(webBaseSpeed));
                } else if (currentMenuItem == 1) {
                    // Toggle oscillation mode
                    oscillatingMode = !oscillatingMode;
                    preferences.putBool("oscMode", oscillatingMode);
                    displayWebConfirmation(oscillatingMode ? "Mode: OSC" : "Mode: NORMAL");
                } else if (currentMenuItem == 2) {
                    // Toggle Motor Version (1 vs 2)
                    uint8_t newVer = (motorVersion == 1) ? 2 : 1;
                    setMotorVersion(newVer);
                } else if (currentMenuItem == 3) {
                    // Reset counter
                    penetrationCount = 0;
                    preferences.putUInt("pCnt", 0);
                    displayWebConfirmation("Reset Count");
                } else if (currentMenuItem == 4) {
                    // Exit Menu
                    inMenuMode = false;
                    displayWebConfirmation("Menu Closed");
                }
            }
            btnRightPressTime = 0;
        }
        return; // Skip normal button operations while in Menu Mode
    }

    // --- 3. Handle Normal Single Button Presses (Context-Aware) ---
    if (oscillatingMode) {
        // --- OSCILLATION MODE: Adjust Timings ---
        const uint16_t timeStep = 500;
        // Left Button (CCW Time)
        if (btnLeftState) {
            if (btnLeftPressTime == 0) btnLeftPressTime = millis();
            else if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > LONG_PRESS_DURATION)) {
                if (oscillationDurationCCW > timeStep) oscillationDurationCCW -= timeStep;
                preferences.putUInt("oscDurCcw", oscillationDurationCCW);
                displayWebConfirmation("CCW: " + String(oscillationDurationCCW) + "ms");
                btnLeftLongPressHandled = true;
            }
        } else if (btnLeftPressTime > 0) {
            if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > DEBOUNCE_DELAY)) {
                oscillationDurationCCW += timeStep;
                if (oscillationDurationCCW > 10000) oscillationDurationCCW = 500;
                preferences.putUInt("oscDurCcw", oscillationDurationCCW);
                displayWebConfirmation("CCW: " + String(oscillationDurationCCW) + "ms");
            }
            btnLeftPressTime = 0; btnLeftLongPressHandled = false;
        }
        // Right Button (CW Time)
        if (btnRightState) {
            if (btnRightPressTime == 0) btnRightPressTime = millis();
            else if (!btnRightLongPressHandled && (millis() - btnRightPressTime > LONG_PRESS_DURATION)) {
                if (oscillationDurationCW > timeStep) oscillationDurationCW -= timeStep;
                preferences.putUInt("oscDurCw", oscillationDurationCW);
                displayWebConfirmation("CW: " + String(oscillationDurationCW) + "ms");
                btnRightLongPressHandled = true;
            }
        } else if (btnRightPressTime > 0) {
            if (!btnRightLongPressHandled && (millis() - btnRightPressTime > DEBOUNCE_DELAY)) {
                oscillationDurationCW += timeStep;
                if (oscillationDurationCW > 10000) oscillationDurationCW = 500;
                preferences.putUInt("oscDurCw", oscillationDurationCW);
                displayWebConfirmation("CW: " + String(oscillationDurationCW) + "ms");
            }
            btnRightPressTime = 0; btnRightLongPressHandled = false;
        }
    } else {
        // --- NORMAL MODE: Adjust Speed ---
        const uint8_t speedStep = 10; // Speed increment/decrement step

        // --- Left Button Logic (Decrease Speed / Pause Operation) ---
        if (btnLeftState) {
            if (btnLeftPressTime == 0) { // First detection
                btnLeftPressTime = millis();
            } else if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > LONG_PRESS_DURATION)) {
                // LONG PRESS ACTION: PAUSE OPERATION
                if (operationState == RUNNING) {
                    operationState = PAUSED;
                    operationTimeAccumulator += millis() - lastTimeCapture;
                    pauseCount++;
                    systemEnabled = false; // Ensure motor is off
                    displayWebConfirmation("Operation Paused");
                    Serial.println("LEFT LONG PRESS: Operation Paused.");
                }
                btnLeftLongPressHandled = true; // Mark as handled
            }
        } else if (btnLeftPressTime > 0) { // Button was just released
            if (!btnLeftLongPressHandled && (millis() - btnLeftPressTime > DEBOUNCE_DELAY)) {
                // SHORT PRESS ACTION: DECREASE SPEED
                if (webBaseSpeed > speedStep) webBaseSpeed -= speedStep; else webBaseSpeed = 0;
                preferences.putUChar("motSpd", webBaseSpeed);
                calibrationNeeded = true;
                displayWebConfirmation("Speed: " + String(webBaseSpeed));
            }
            btnLeftPressTime = 0; btnLeftLongPressHandled = false; // Reset state
        }

        // --- Right Button Logic (Increase Speed / Start Operation) ---
        if (btnRightState) {
            if (btnRightPressTime == 0) { // First detection
                btnRightPressTime = millis();
            } else if (!btnRightLongPressHandled && (millis() - btnRightPressTime > LONG_PRESS_DURATION)) {
                // LONG PRESS ACTION: START OPERATION
                if (operationState == IDLE) { // Start a new operation
                    operationState = RUNNING;
                    operationTimeAccumulator = 0;
                    pauseCount = 0;
                    lastTimeCapture = millis();
                    systemEnabled = true; // Also enable the motor system
                    displayWebConfirmation("Operation Started");
                    Serial.println("RIGHT LONG PRESS: Operation Started.");
                } else if (operationState == PAUSED) { // Resume a paused operation
                    operationState = RUNNING;
                    lastTimeCapture = millis(); // Reset the timer for the new running phase
                    systemEnabled = true; // Re-enable the motor system
                    displayWebConfirmation("Operation Resumed");
                    Serial.println("RIGHT LONG PRESS: Operation Resumed.");
                }
                btnRightLongPressHandled = true; // Mark as handled
            }
        } else if (btnRightPressTime > 0) { // Button was just released
            if (!btnRightLongPressHandled && (millis() - btnRightPressTime > DEBOUNCE_DELAY)) {
                // SHORT PRESS ACTION: INCREASE SPEED
                if (webBaseSpeed < 255 - speedStep) webBaseSpeed += speedStep; else webBaseSpeed = 255;
                preferences.putUChar("motSpd", webBaseSpeed);
                calibrationNeeded = true;
                displayWebConfirmation("Speed: " + String(webBaseSpeed));
            }
            btnRightPressTime = 0; btnRightLongPressHandled = false; // Reset state
        }
    }
}

void handleSafetyChecks() {
    if (batteryCurrent > maxCurrentThreshold) {
        if (!debugMode) {
            safetyTripped = true;
            Serial.println(F("CRITICAL: Overcurrent detected! Motor Halted."));
        } else {
            Serial.println(F("DEBUG: Overcurrent detected but bypassed."));
        }
    }

    if (batteryVoltage < minVoltageThreshold && batteryVoltage > 2.5) {
        if (!lowBatteryTripped && !debugMode) {
            lowBatteryTripped = true;
            Serial.println(F("CRITICAL: Low Battery! Motor Halted to protect cell."));
        }
    } else {
        lowBatteryTripped = false; // Auto-recover if healthy or disconnected
    }
}

void handleMotorControl() {
    if ((safetyTripped || lowBatteryTripped) && !debugMode) {
        targetSpeed = 0;
        motorSpeed = 0; // Immediate halt for safety
        setMotorSpeed(0);
    } else {
        if (systemEnabled && operationState == RUNNING && oscillatingMode) {
            // --- OSCILLATION MODE LOGIC ---
            uint32_t currentPhaseDuration = oscillationDirectionFwd ? oscillationDurationCW : oscillationDurationCCW;

            if (millis() - lastOscillationSwitch > currentPhaseDuration) {
                oscillationDirectionFwd = !oscillationDirectionFwd; // Switch direction
                lastOscillationSwitch = millis();
                Serial.println(oscillationDirectionFwd ? "Oscillating -> FWD" : "Oscillating -> REV");
            }

            targetSpeed = oscillationDirectionFwd ? webBaseSpeed : -webBaseSpeed;

        } else {
            // --- NORMAL MODE LOGIC ---
            if (systemEnabled && operationState == RUNNING) {
                targetSpeed = forwardDirection ? webBaseSpeed : -webBaseSpeed;
            } else {
                if (motorSpeed != 0) { // If motor was running and is now being turned off
                    preferences.putUInt("pCnt", penetrationCount); // Save count on motor stop
                    Serial.println(F("Motor stopped. Final graft count saved to NVM."));
                }
                targetSpeed = 0;
            }
        }
        
        // Shared ramp logic for both modes
        const int16_t RAMP_STEP = 10;
        if (motorSpeed < targetSpeed) {
            motorSpeed += RAMP_STEP;
            if (motorSpeed > targetSpeed) motorSpeed = targetSpeed;
        } else if (motorSpeed > targetSpeed) {
            motorSpeed -= RAMP_STEP;
            if (motorSpeed < targetSpeed) motorSpeed = targetSpeed;
        }

        if (!systemEnabled) {
            motorSpeed = 0; // Ensure motor is fully off
        }
        setMotorSpeed(motorSpeed);
    }
}

void handleCalibration() {
    if (calibrationNeeded) {
        isPenetrating = false;
        if (abs(motorSpeed - targetSpeed) > 10 || targetSpeed == 0) {
            calibrationStartTime = 0;
        } else {
            if (calibrationStartTime == 0) {
                calibrationStartTime = millis();
                calibrationSum = 0.0;
                calibrationRPMSum = 0.0; // Reset RPM sum
                calibrationSamples = 0;
                Serial.println(F("Motor at speed. Starting baseline calibration..."));
            }

            if (millis() - calibrationStartTime < 2000) { // Calibrate for 2 seconds
                calibrationSum += batteryCurrent;
                calibrationRPMSum += (float)estimatedRPM; // Sample current measured RPM
                calibrationSamples++;
            } else {
                baselineCurrent = calibrationSum / (float)calibrationSamples;
                baselineRPM = calibrationRPMSum / (float)calibrationSamples; // Calibrate baseline RPM
                calibrationNeeded = false;
                calibrationStartTime = 0; 
                Serial.print(F("Calibration complete. Baseline Current: "));
                Serial.print(baselineCurrent, 3);
                Serial.print(F("A, Baseline RPM: "));
                Serial.println(baselineRPM, 1);
            }
        }
    }
}

/**
 * @brief Reads the TTGO board's internal battery voltage (via GPIO 34 and GPIO 14) using esp_adc_cal characterized readings.
 * @return The measured internal voltage in Volts.
 */
float readInternalBatteryVoltage() {
    // Read the characterized millivolts directly from the ESP32-S3 analog API
    uint32_t mv = analogReadMilliVolts(4);
    
    // The onboard divider is 100k/100k (ratio 1:2), so multiply by 2.0.
    float volt = (mv / 1000.0f) * 2.0f;
    return volt;
}

void calculateTelemetry() {
    batteryPower = batteryVoltage * batteryCurrent;

    if (motorVersion == 1) {
        // --- PREMIUM VERSION: Physical Encoder Tracking ---
        // Thread-safe copy of encoderTicks and reset
        noInterrupts();
        long ticks = encoderTicks;
        encoderTicks = 0;
        interrupts();

        // Calculate actual RPM from encoder ticks
        static unsigned long lastRPMCalcTime = 0;
        unsigned long now = millis();
        unsigned long dt = now - lastRPMCalcTime;
        if (dt == 0) dt = 1; // Prevent division by zero

        if (systemEnabled && abs(motorSpeed) > 10) {
            float shaftRevs = abs(ticks) / ENCODER_PPR;
            uint32_t measuredRPM = (uint32_t)(shaftRevs * (60000.0f / dt));
            
            // Low pass filter to smooth out noise (30% new value, 70% previous)
            static float smoothedRPM = 0.0f;
            smoothedRPM = (measuredRPM * 0.3f) + (smoothedRPM * 0.7f);
            estimatedRPM = (uint32_t)smoothedRPM;
        } else {
            estimatedRPM = 0;
        }
        lastRPMCalcTime = now;
    } else {
        // --- STANDARD VERSION: Voltage-Based Estimation ---
        if (systemEnabled && abs(motorSpeed) > 10) {
            float appliedVoltage = batteryVoltage * (abs(motorSpeed) / 255.0f);
            estimatedRPM = (uint32_t)(appliedVoltage * (13000.0f / 3.0f)); // Estimated motor speed constant
        } else {
            estimatedRPM = 0;
        }
    }

    // Read the internal battery voltage
    internalVoltage = readInternalBatteryVoltage();

    bool isBatteryConnected = (batteryVoltage > 2.5);
    
    // Compute battery percentage based on the actual battery terminal voltage (MAX471)
    if (isBatteryConnected) {
        float pc = (batteryVoltage - minVoltageThreshold) / (4.2f - minVoltageThreshold) * 100.0f;
        batteryPercent = (uint8_t)constrain(pc, 0, 100);
    } else {
        batteryPercent = 0;
    }

    // Charging & USB connection detection logic:
    // Comparing the onboard charger's VBAT line (internalVoltage) with the actual cell terminal voltage (batteryVoltage).
    if (isBatteryConnected) {
        // If internal voltage is > 4.25V, or if it is > 0.12V higher than the external battery voltage,
        // it means USB is plugged in and the charger is actively boosting/supplying power.
        isCharging = (internalVoltage > 4.25f) || ((internalVoltage - batteryVoltage) > 0.12f);
    } else {
        // If no battery is connected, but we have a high voltage reading internally (from the float charger), USB is connected.
        isCharging = (internalVoltage > 4.0f);
    }
}

void handleGraftCounter() {
    // In oscillation mode, we can ignore the calibrationNeeded flag as the baseline is relatively stable.
    bool canCount = (oscillatingMode) ? true : !calibrationNeeded;

    if (systemEnabled && abs(motorSpeed) > 50 && canCount && baselineCurrent > 0.05) {
        float dynamicSpikeThreshold = baselineCurrent + penetrationOffset;
        float dynamicFallBackThreshold = dynamicSpikeThreshold - penetrationHysteresis;

        // Speed Dip Verification:
        // Detect a mechanical load spike (skin penetration) by checking if the actual RPM
        // dips by at least speedDipThresholdPercent below the calibrated baseline RPM.
        bool isSpeedDipped = false;
        if (motorVersion == 1) {
            if (baselineRPM > 50.0f) {
                float rpmDropPercent = ((baselineRPM - (float)estimatedRPM) / baselineRPM) * 100.0f;
                isSpeedDipped = (rpmDropPercent >= speedDipThresholdPercent);
            } else {
                // Fallback if baseline RPM isn't fully calibrated (e.g., startup transients or very low speeds)
                isSpeedDipped = true;
            }
        } else {
            // Standard Version: Skip mechanical verification, trust pure current-spike
            isSpeedDipped = true;
        }

        // SENSOR FUSION DECISION:
        // A successful graft count (skin penetration) is triggered when:
        // 1. Motor current SPIKES above dynamic current threshold (electrical load increase).
        // 2. Motor actual RPM DIPS below speed threshold (mechanical resistance increase).
        if (batteryCurrent > dynamicSpikeThreshold && isSpeedDipped && !isPenetrating) {
            isPenetrating = true;
            Serial.print(F("FUSION PENETRATION START: Current=")); Serial.print(batteryCurrent);
            Serial.print(F("A (Thresh=")); Serial.print(dynamicSpikeThreshold);
            Serial.print(F("A), RPM=")); Serial.print(estimatedRPM);
            Serial.print(F(" (Base=")); Serial.print(baselineRPM);
            Serial.println(F(")"));
        } else if (batteryCurrent < dynamicFallBackThreshold && isPenetrating) {
            // Return of motor current to normal baseline resets the lock and registers the count
            isPenetrating = false;
            penetrationCount++;
            displayGraftFlash(penetrationCount);
            Serial.print(F("FUSION PENETRATION END: Count=")); Serial.println(penetrationCount);
        }
    }
}

void updateUI() {
    if (millis() - lastScreenSwitch > screenSwitchTime) {
        int maxScreens = 4;
        if (patientName != "N/A") {
            maxScreens = 5; // Add patient screen if a patient is selected
        }
        currentScreen = (currentScreen + 1) % maxScreens;
        lastScreenSwitch = millis();
    }

    String displayIP = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    uint8_t clients = WiFi.softAPgetStationNum();
    bool isBatteryConnected = (batteryVoltage > 2.5);
    
    refreshOLED(motorSpeed, batteryVoltage, batteryCurrent, safetyTripped, lowBatteryTripped, 
                estimatedRPM, batteryPower, isBatteryConnected, batteryPercent, isCharging, 
                currentScreen, displayIP, clients, debugMode, penetrationCount, wifiConnecting,
                oscillatingMode, oscillationDurationCW, oscillationDurationCCW, patientName, patientAge, patientNationality);
}

/**
 * @brief Gets the current date from the synchronized clock.
 * @return A string with the date formatted as YYYY-MM-DD, or a placeholder if time is not set.
 */
String getFormattedDate() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo, 5000)){ // 5s timeout
    Serial.println("Failed to obtain time from NTP.");
    return "YYYY-MM-DD";
  }
  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}
