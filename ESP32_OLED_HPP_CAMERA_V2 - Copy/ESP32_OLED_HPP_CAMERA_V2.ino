/**
 * MAIN SKETCH: EL-BASEET Hair Pen V2 (Camera Enhanced)
 * DESCRIPTION: Orchestrates motor control, safety monitoring, OLED feedback,
 *              and the Web Dashboard for both TTGO OLED and ESP32-CAM boards.
 * Created by Eng. Osama Omar
 */

#include "BoardConfig.h"  // <--- Must be first for preprocessor selection
#include "max471.h"
#include "drv8833.h"
#include "oled.h"
#include "web_server.h"

#include "camera_handler.h"

// --- GLOBAL VARIABLE DEFINITIONS (Linked to web_server.h externs) ---
int16_t  motorSpeed = 0;
volatile float batteryVoltage = 0.0, batteryCurrent = 0.0, batteryPower = 0.0;
float    maxCurrentThreshold = 2.5, minVoltageThreshold = 3.2, baselineCurrent = 0.0;
float    penetrationOffset = 0.04, penetrationHysteresis = 0.02;
volatile float currentFPS = 0.0;
volatile uint32_t penetrationCount = 0;
uint8_t  batteryPercent = 100, webBaseSpeed = 200;
uint32_t screenSwitchTime = 5000;

volatile bool calibrationNeeded = false;
volatile bool systemEnabled = false, forwardDirection = true, safetyTripped = false;
volatile bool lowBatteryTripped = false, debugMode = false, isPenetrating = false, wifiConnecting = false;
bool cameraAvailable = false;
volatile bool lampState = false;
volatile bool sensorError = false; 
volatile bool pendingCountSave = false; // Flag to defer NVM writes to loop()

unsigned long lampStartTime = 0; 
uint32_t lampAutoOffDuration = 30000;
Preferences preferences;

// Timer variables for UI and Task Management
unsigned long lastOLEDUpdate = 0;
unsigned long lastScreenSwap = 0;
uint8_t activeScreen = 0;

// RTOS Task Handles
TaskHandle_t SafetyTaskHandle = NULL;
SemaphoreHandle_t telemetryMutex;

void safetyTaskCode(void * pvParameters);

void setup() {
    // Disable brownout detector immediately to handle I2C/Flash LED current spikes
    #include "soc/soc.h"
    #include "soc/rtc_cntl_reg.h"
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Create Mutex for thread-safe telemetry access
    telemetryMutex = xSemaphoreCreateMutex();

    // 1. Initialize System Communications
    Serial.begin(115200);
    delay(500); // Wait for Serial to stabilize
    Serial.println(F("BOOTING..."));

    // --- HARDWARE IDENTITY PROBE ---
    Serial.println(F("\n=============================="));
    Serial.print(F("IDE Board Definition: "));
    #ifdef ARDUINO_BOARD
        Serial.println(ARDUINO_BOARD);
    #else
        Serial.println(F("Undefined (Generic)"));
    #endif
    Serial.println(F("==============================\n"));
    
    // 2. Initialize NVM (Flash Memory)
    preferences.begin("hpp-v1", false);
    
    // 3. Load Persistent Settings
    webBaseSpeed = preferences.getUChar("motSpd", 200);
    maxCurrentThreshold = preferences.getFloat("maxCurr", DEFAULT_MAX_CURRENT);
    minVoltageThreshold = preferences.getFloat("minVolt", DEFAULT_MIN_VOLTAGE);
    penetrationOffset = preferences.getFloat("penOff", DEFAULT_PEN_OFFSET);
    baselineCurrent = preferences.getFloat("baseCurr", 0.0); // Load persisted baseline
    penetrationHysteresis = preferences.getFloat("penHyst", DEFAULT_PEN_HYST);
    penetrationCount = preferences.getUInt("pCnt", 0);
    debugMode = preferences.getBool("debug", false);
    lampAutoOffDuration = preferences.getUInt("lmoTime", 30000);

    // 4. Initialize Hardware Modules (HAL)
    setupDRV8833();

    // --- PRE-FLIGHT SENSOR SEEDING ---
    // Read sensors BEFORE WiFi/Camera start to avoid ADC2 conflicts
    Serial.println(F("Seeding initial battery telemetry..."));
    for(int i=0; i<10; i++) {
        readMAX471(batteryVoltage, batteryCurrent);
        delay(10);
    }
    
    if (setupCamera()) {
        Serial.println(F("Camera OK"));
        cameraAvailable = true;
    }
    delay(500); // Stagger power draw

    setupWeb(); // WiFi is the most power-hungry, start it before the LED-conflicting I2C
    delay(500);

    // Start OLED last to prevent I2C pull-ups from causing a brownout during WiFi/Cam init
    #if HAS_DISPLAY
    setupOLED();
    showWelcomeLogo();
    #endif

    // 5. Create the High-Priority Safety Task on Core 1
    xTaskCreatePinnedToCore(
        safetyTaskCode,       /* Task function */
        "SafetyTask",         /* Name */
        4096,                 /* Stack size */
        NULL,                 /* Parameter */
        10,                   /* Priority (Medium): Surgical safety must be reliable */
        &SafetyTaskHandle,    /* Handle */
        1                     /* Core 1 */
    );

    Serial.println(F("SYSTEM READY"));
}

/**
 * @brief FreeRTOS Task: High-speed Safety and Graft Detection
 */
void safetyTaskCode(void * pvParameters) {
    float localV, localI, localP;
    for(;;) {
    // 1. Read to local variables first to keep critical sections short
    readMAX471(localV, localI);
    localP = localV * localI;

    // 2. Safety Logic Engine (Using local copies)
    if (!debugMode) {
        if (localI > maxCurrentThreshold) safetyTripped = true;
        if (localV < minVoltageThreshold && localV > 1.0) lowBatteryTripped = true;
    }

    // 3. Update Shared Globals under Mutex
    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        batteryVoltage = localV;
        batteryCurrent = localI;
        batteryPower = localP;
        xSemaphoreGive(telemetryMutex);
    }

    // 4. Calibration Logic
    if (calibrationNeeded && systemEnabled && !safetyTripped) {
        float burstSum = 0;
        int validSamples = 0;
        
        for (int i = 0; i < 10; i++) {
            float bV, bI;
            readMAX471(bV, bI);
            if (bV > 1.0) {
                burstSum += bI;
                validSamples++;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }

        if (validSamples >= 10 && !safetyTripped) {
            baselineCurrent = burstSum / (float)validSamples;
            calibrationNeeded = false;
            // Trigger deferred NVM save instead of direct write
            pendingCountSave = true; 
            displayWebConfirmation("Calib: " + String(baselineCurrent, 3) + "A");
        }
    }

    // 5. Graft Counting Logic
    if (systemEnabled && !calibrationNeeded && !safetyTripped) {
        float threshold = baselineCurrent + penetrationOffset;
        if (!isPenetrating && localI > threshold) {
            isPenetrating = true;
            penetrationCount++;
            pendingCountSave = true; // Signal loop() to save NVM
            triggerGraftFeedback(); // Visual trigger for OLED
        } else if (isPenetrating && localI < (threshold - penetrationHysteresis)) {
            isPenetrating = false;
        }
    }

    // Battery Percentage Calculation (18650 Curve 3.2V - 4.2V)
    if (batteryVoltage > 3.0) {
        float pct = ((batteryVoltage - 3.2) / (4.2 - 3.2)) * 100.0;
        batteryPercent = (uint8_t)constrain(pct, 0, 100);
    } else {
        batteryPercent = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz: Medical-grade response time
    }
}

void loop() {
    // Safe NVM Management: Only write to Preferences on the main loop core
    if (pendingCountSave) {
        preferences.putUInt("pCnt", penetrationCount);
        preferences.putFloat("baseCurr", baselineCurrent);
        pendingCountSave = false;
    }

    // A. Handle Web Server Client Requests (Core 1 handles this, Core 0 handles WiFi)
    server.handleClient();

    // B. Safety and Telemetry are now handled by safetyTaskCode on Core 1

    // D. Motor Control Logic
    if (safetyTripped || lowBatteryTripped || !systemEnabled) {
        setMotorSpeed(0);
        motorSpeed = 0;
    } else {
        // Set speed based on direction
        motorSpeed = forwardDirection ? (int16_t)webBaseSpeed : -(int16_t)webBaseSpeed;
        setMotorSpeed(motorSpeed);
    }

    // E. UI State Management (Screen Rotation)
    if (millis() - lastScreenSwap > screenSwitchTime) {
        activeScreen = (activeScreen + 1) % 3;
        lastScreenSwap = millis();
    }

    // F. OLED Refresh (4Hz Refresh Rate)
    if (millis() - lastOLEDUpdate > 250) {
        float snapV, snapI, snapP;
        // Take a consistent snapshot for the UI
        if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            snapV = batteryVoltage;
            snapI = batteryCurrent;
            snapP = batteryPower;
            xSemaphoreGive(telemetryMutex);
        } else {
            return; // Skip this UI frame if data is being updated
        }

        refreshOLED(
            motorSpeed, 
            snapV, 
            snapI, 
            safetyTripped, 
            lowBatteryTripped, 
            snapP, 
            snapV > 2.0, 
            batteryPercent, 
            false,                // Charging status
            activeScreen, 
            WiFi.softAPIP().toString(), 
            WiFi.softAPgetStationNum(), 
            debugMode, 
            penetrationCount, 
            wifiConnecting,
            cameraAvailable
        );
        lastOLEDUpdate = millis();
    }

    // G. Surgical Light Safety Timer (ESP32-CAM only)
    // Auto-off after configured duration to prevent overheating
    if (PIN_LAMP_FLASH != -1) {
        if (lampState && (millis() - lampStartTime > lampAutoOffDuration)) {
            lampState = false;
            digitalWrite(PIN_LAMP_FLASH, LOW);
            displayWebConfirmation("Lamp: Auto-Off");
        }
    }

    // Small delay to prevent watchdog triggers and allow background tasks
    delay(1); 
}