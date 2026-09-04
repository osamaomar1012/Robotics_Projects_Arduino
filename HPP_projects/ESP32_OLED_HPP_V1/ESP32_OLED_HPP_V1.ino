/**
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */

// Module Imports
#include "drv8833.h"
#include "max471.h"
#include "oled.h"
#include "web_server.h"
#include <Preferences.h>

// Global Variables
float maxCurrentThreshold = 2.5; // Safety limit in Amperes (adjust to motor specs)
float minVoltageThreshold = 3.2; // 18650 safe discharge limit (Volts)
int16_t motorSpeed = 0;   // Signed actual speed
int16_t targetSpeed = 0;  // Signed target speed
float batteryVoltage = 0.0;
float batteryCurrent = 0.0;
float batteryPower = 0.0;
unsigned long lastUpdate = 0;

// Penetration Counter Calibration Variables
float baselineCurrent = 0.0; // Dynamically calibrated current at current speed
bool calibrationNeeded = true;
unsigned long calibrationStartTime = 0;
float calibrationSum = 0.0;
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
bool wifiConnecting = false; // Flag to indicate if STA connection is in progress
Preferences preferences;

void setup() {
    Serial.begin(115200);
    
    // Initialize NVM Storage
    preferences.begin("hpp_v1", false);
    
    // Load saved values, or use defaults if not found
    webBaseSpeed = preferences.getUChar("motSpd", 200);
    screenSwitchTime = preferences.getUInt("scrTime", 5000);
    if (screenSwitchTime < 1000) screenSwitchTime = 5000; // Sanity check for rotation
    maxCurrentThreshold = preferences.getFloat("maxCurr", 2.5);
    minVoltageThreshold = preferences.getFloat("minVolt", 3.2);
    penetrationCount = preferences.getUInt("pCnt", 0);
    penetrationOffset = preferences.getFloat("penOff", 0.04);
    penetrationHysteresis = preferences.getFloat("penHyst", 0.02);
    debugMode = preferences.getBool("debug", false);
    
    // calibrationNeeded will be triggered when motor is first turned ON or speed changes
    setupOLED();
    showWelcomeLogo(); // Safe to re-enable now

    setupDRV8833();
    
    delay(500); // Breathe before WiFi
    setupWeb(); 
    
    // Initial sensor read to populate values before first display refresh
    // This also seeds the EMA filter
    readMAX471(batteryVoltage, batteryCurrent);

    // Synchronize timers to prevent immediate execution of loop logic
    lastUpdate = millis();
    lastScreenSwitch = millis();
}

void loop() {
    server.handleClient(); // Process web requests

    // Check WiFi status and update wifiConnecting flag
    if (wifiConnecting) {
        if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_CONNECT_FAILED) {
            wifiConnecting = false; // Connection attempt resolved
        }
    }

    // Execute every 100ms for better I2C stability
    if (millis() - lastUpdate > 100) {
        lastUpdate = millis();

        // 0. Screen Management:
        // Cycle between all screens: 0 (Drive), 1 (Energy), 2 (Network)
        if (millis() - lastScreenSwitch > screenSwitchTime) {
            currentScreen = (currentScreen + 1) % 3;
            lastScreenSwitch = millis();
        }

        // 1. Telemetry Update: Reads smoothed voltage and current values
        //    from the MAX471 sensor. These values are updated by reference.
        //    The filter inside max471.h prevents false trips from motor noise.
        readMAX471(batteryVoltage, batteryCurrent);

        // 2. Overcurrent Safety Check
        if (batteryCurrent > maxCurrentThreshold) {
            if (!debugMode) {
                safetyTripped = true;
                Serial.println(F("CRITICAL: Overcurrent detected! Motor Halted."));
            } else {
                Serial.println(F("DEBUG: Overcurrent detected but bypassed."));
            }
        }

        // 3. Low Voltage Protection: Only trip if battery is present (2.5V to 3.2V)
        // Anything below 2.5V is treated as "No Battery" (USB power mode).
        if (batteryVoltage < minVoltageThreshold && batteryVoltage > 2.5) {
            if (!lowBatteryTripped && !debugMode) {
                lowBatteryTripped = true;
                Serial.println(F("CRITICAL: Low Battery! Motor Halted to protect cell."));
            }
        } else {
            lowBatteryTripped = false; // Auto-recover if healthy or disconnected
        }

        if ((safetyTripped || lowBatteryTripped) && !debugMode) {
            // Force motor to stop if safety is tripped
            targetSpeed = 0;
            motorSpeed = 0; // Immediate halt for safety
            setMotorSpeed(0);
        } else {
            // 3. Soft-Start / Soft-Stop Logic
            if (systemEnabled) {
                // Set target based on web slider and direction toggle
                targetSpeed = forwardDirection ? webBaseSpeed : -webBaseSpeed;
            } else {
                targetSpeed = 0;
            }
            
            const int16_t RAMP_STEP = 10; // Adjust this value to control acceleration (higher = faster)
            if (motorSpeed < targetSpeed) {
                motorSpeed += RAMP_STEP;
                if (motorSpeed > targetSpeed) motorSpeed = targetSpeed; // Prevent overshoot
            } else if (motorSpeed > targetSpeed) {
                motorSpeed -= RAMP_STEP;
                if (motorSpeed < targetSpeed) motorSpeed = targetSpeed; // Prevent overshoot
            }
            setMotorSpeed(motorSpeed);
        }

        // 4. Current Baseline Calibration Logic
        if (calibrationNeeded) {
            isPenetrating = false; // Force penetration state to false to allow clean baseline
            
            // Wait for the motor to reach target speed before starting calibration
            if (motorSpeed != targetSpeed || targetSpeed == 0) {
                calibrationStartTime = 0;
                calibrationSamples = 0;
                calibrationSum = 0.0;
            } else {
                if (calibrationStartTime == 0) { // Start calibration timer
                    calibrationStartTime = millis();
                    calibrationSum = 0.0;
                    calibrationSamples = 0;
                    Serial.println(F("Motor at speed. Starting baseline calibration..."));
                }

                // Collect exactly 40 samples (approx 2 seconds at 50ms intervals)
                if (calibrationSamples < 40) {
                    calibrationSum += batteryCurrent;
                    calibrationSamples++;
                } else {
                    // Calibration finished
                    baselineCurrent = calibrationSum / 40.0;
                    calibrationNeeded = false;
                    calibrationStartTime = 0; 
                    Serial.print(F("Calibration complete. Baseline Current: "));
                    Serial.print(baselineCurrent, 3);
                    Serial.println(F("A"));
                }
            }
        }

        // 5. RPM Calculation
        // Formula: Applied Voltage = Battery Voltage * (Duty Cycle / 255)
        // RPM = Applied Voltage * (No-Load RPM / Nominal Voltage)
        float appliedVoltage = batteryVoltage * (abs(motorSpeed) / 255.0);
        estimatedRPM = (uint32_t)(appliedVoltage * (13000.0 / 3.0));

        // 5. Power Calculation (Watts = Volts * Amps)
        batteryPower = batteryVoltage * batteryCurrent;

        // 6. Source Detection: Identify if we are on Battery or USB Power
        bool isBatteryConnected = (batteryVoltage > 2.5);
        bool isCharging = (batteryVoltage > 4.25); // Heuristic for charging state

        // 7. Battery Percentage Calculation
        // Mapping 3.2V (0%) to 4.2V (100%)
        if (isBatteryConnected) {
            float pc = (batteryVoltage - minVoltageThreshold) / (4.2f - minVoltageThreshold) * 100.0f;
            batteryPercent = (uint8_t)constrain(pc, 0, 100);
        } else {
            batteryPercent = 0;
        }

        // 9. Penetration Counter Logic
        // Detects a current spike (increase then decrease)
        // Only detect if system is enabled, motor is moving, and calibration is done
        if (systemEnabled && abs(motorSpeed) > 50 && !calibrationNeeded && baselineCurrent > 0.05) {
            float dynamicSpikeThreshold = baselineCurrent + penetrationOffset;
            float dynamicFallBackThreshold = dynamicSpikeThreshold - penetrationHysteresis;

            if (batteryCurrent > dynamicSpikeThreshold && !isPenetrating) {
                isPenetrating = true; // Entered surface, start penetration state
                Serial.print(F("Penetration START detected. Current: ")); Serial.print(batteryCurrent); Serial.print(F("A, Baseline: ")); Serial.print(baselineCurrent); Serial.println(F("A"));
            } else if (batteryCurrent < dynamicFallBackThreshold && isPenetrating) {
                isPenetrating = false; // Successfully penetrated, current dropped below hysteresis
                penetrationCount++; // Increment counter
                Serial.print(F("Penetration END detected. Count: ")); Serial.println(penetrationCount);
                // Save immediately to NVM for persistence
                preferences.putUInt("pCnt", penetrationCount);
            }
        }

        // 8. UI Update: Refreshes the OLED display with the latest stats
        //    battery voltage, and battery current.
        String displayIP = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        uint8_t clients = WiFi.softAPgetStationNum();
        
        refreshOLED(motorSpeed, batteryVoltage, batteryCurrent, safetyTripped, lowBatteryTripped, 
                    estimatedRPM, batteryPower, isBatteryConnected, batteryPercent, isCharging, 
                    currentScreen, displayIP, clients, debugMode, penetrationCount, wifiConnecting);

        // Debug Heartbeat: Verifies the code hasn't frozen
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 5000) { Serial.println(F("System Running...")); lastHeartbeat = millis(); }
    }
}
