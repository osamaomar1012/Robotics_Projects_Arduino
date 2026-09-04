#include "BoardConfig.h"
#include "camera_handler.h"
#include "web_server.h"
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Global Control & Safety Variables
Adafruit_ADS1115 ads;
Preferences preferences;
// IMPORTANT: Replace with the actual MAC address of your CrowPanel S3 Receiver
// You can find the CrowPanel's MAC in its Serial Monitor output during setup.
uint8_t targetMac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; // Placeholder MAC

volatile bool cameraActive = false;
const char* ssid_sta = ""; // Default V1 empty station
const char* pass_sta = "";

float maxCurrentThreshold = 2.5; 
float minVoltageThreshold = 3.2; 
int16_t motorSpeed = 0;   
int16_t targetSpeed = 0;  
float batteryVoltage = 0.0;
float currentDraw = 0.0;
float batteryPower = 0.0;
unsigned long lastUpdate = 0;

// Penetration Counter Variables
float baselineCurrent = 0.0; 
bool calibrationNeeded = true;
unsigned long calibrationStartTime = 0;
float calibrationSum = 0.0;
uint16_t calibrationSamples = 0;
uint32_t estimatedRPM = 0;
float penetrationOffset = 0.04;        
float penetrationHysteresis = 0.02;    
uint32_t penetrationCount = 0;
bool isPenetrating = false;
uint8_t batteryPercent = 0;
uint8_t webBaseSpeed = 200;
uint8_t flashBrightness = 0;
bool adsFound = false;
bool forwardDirection = true;  
bool safetyTripped = false; 
bool lowBatteryTripped = false; 
bool systemEnabled = false; 
bool debugMode = false;     
bool wifiConnecting = false; 

void displayWebConfirmation(String msg) {
    Serial.println("HPP CMD: " + msg);
}

// Surgical Pen Motor Control (DRV8833)
void setMotorSpeed(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed >= 0) {
        ledcWrite(0, speed);
        ledcWrite(1, 0);
    } else {
        ledcWrite(0, 0);
        ledcWrite(1, abs(speed));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- EL-BASEET HPP_Advance Booting ---");
    preferences.begin("hpp_v3", false);

    // Load Surgical Defaults
    webBaseSpeed = preferences.getUChar("motSpd", 200);
    maxCurrentThreshold = preferences.getFloat("maxCurr", 2.5);
    minVoltageThreshold = preferences.getFloat("minVolt", 3.2);
    penetrationCount = preferences.getUInt("pCnt", 0);
    penetrationOffset = preferences.getFloat("penOff", 0.04);
    penetrationHysteresis = preferences.getFloat("penHyst", 0.02);
    debugMode = preferences.getBool("debug", false);

    // Initialize I2C Bus for ADS1115
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000); // 400kHz Fast Mode
    Wire.setTimeOut(100);  // 100ms Timeout to prevent hanging the Web Server

    // Re-enable Camera Engine
    if (setupCamera()) {
        Serial.println("Camera Engine: OK");
        cameraActive = true;
    } else {
        Serial.println("Camera Engine: FAILED");
        cameraActive = false;
    }

    // Initialize Surgical Pen Motor Driver
    ledcSetup(0, 5000, 8); 
    ledcSetup(1, 5000, 8); 
    ledcAttachPin(PIN_MOTOR_IN1, 0);
    ledcAttachPin(PIN_MOTOR_IN2, 1);
    setMotorSpeed(0);

    if (CAM_PIN_FLASH != -1) {
        ledcSetup(2, 5000, 8); // Channel 2, 5kHz, 8-bit resolution
        ledcAttachPin(CAM_PIN_FLASH, 2);
        ledcWrite(2, 0);
    }

#if USE_SENSORS
    // Initialize Sensors
    Serial.println("Scanning I2C Bus...");
    byte count = 0;
    for (byte i = 8; i < 127; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            Serial.printf("Found I2C device at 0x%02X\n", i);
            count++;
        }
    }
    
    if (count == 0) {
        Serial.println("CRITICAL ERROR: No I2C devices found on the bus!");
        Serial.println("1. Verify SDA is GPIO 14, SCL is GPIO 15");
        Serial.println("2. Try swapping SDA and SCL wires");
        Serial.println("3. Ensure ADS1115 is powered with 3.3V");
        Serial.println("4. Add 4.7k Ohm external pull-up resistors if not present");
    }

    if (!ads.begin()) {
        Serial.println("ADS1115: FAILED (Check Wiring)");
        adsFound = false;
    } else {
        // Set Gain to 2/3 (reads up to +/- 6.144V)
        // Essential because MAX471 VT can output up to 5V
        ads.setGain(GAIN_TWOTHIRDS);
        
        // Increase data rate for surgical spike detection (Default is 128)
        ads.setDataRate(RATE_ADS1115_860SPS); 
        
        adsFound = true;
        Serial.println("ADS1115: OK");
    }
#endif

    setupWeb();
    
    Serial.print("\n--- SENDER CONFIGURATION ---\n");
    Serial.printf("ESP32-CAM MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
    Serial.printf("Configured CrowPanel MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                  targetMac[0], targetMac[1], targetMac[2], targetMac[3], targetMac[4], targetMac[5]);

    // Re-enable ESP-NOW for remote camera control/viewing
    if (esp_now_init() == ESP_OK) { // Ensure ESP-NOW is initialized
        Serial.println("ESP-NOW Engine: OK");
        esp_now_register_recv_cb(onEspNowRecv); 

        memcpy(targetMac, broadcastAddress, 6);
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, targetMac, 6);
        peerInfo.channel = 1; 
        peerInfo.encrypt = false;
        peerInfo.ifidx = WIFI_IF_AP; 
        esp_now_add_peer(&peerInfo);
        xTaskCreatePinnedToCore(espNowStreamTask, "esp_now_strm", 4096, NULL, 5, NULL, 0);
    }

    lastUpdate = millis();
    Serial.println("System Ready. Connect to WiFi HPP_ADVANCE_SURGERY");
}

void loop() {
    server.handleClient();

    // High-Precision Control Loop (50ms)
    if (millis() - lastUpdate > 50) {
        lastUpdate = millis();

        #if USE_SENSORS
        // 1. Read and Smooth Telemetry
        // ADS1115 computeVolts converts the raw 16-bit value to actual Volts based on gain
        if (adsFound) {
            // Current Reading (MAX471 AT: 1V = 1A)
            float instantI = ads.computeVolts(ads.readADC_SingleEnded(ADS_CH_CURRENT));
            currentDraw = (instantI * CURRENT_ALPHA) + (currentDraw * (1.0 - CURRENT_ALPHA));

            // Voltage Reading (MAX471 VT: VT = V_bat / 5)
            float instantV = ads.computeVolts(ads.readADC_SingleEnded(ADS_CH_VOLTAGE)) * 5.0;
            batteryVoltage = (instantV * CURRENT_ALPHA) + (batteryVoltage * (1.0 - CURRENT_ALPHA));
        }
        
        #endif

        // 2. Safety Logic
        if (currentDraw > maxCurrentThreshold && !debugMode) {
            safetyTripped = true;
            systemEnabled = false;
        }

        if (batteryVoltage < minVoltageThreshold && batteryVoltage > 2.5 && !debugMode) {
            lowBatteryTripped = true;
            systemEnabled = false;
        }

        // 3. Motor Ramping (Soft Start/Stop)
        if (systemEnabled && !safetyTripped && !lowBatteryTripped) {
            targetSpeed = forwardDirection ? webBaseSpeed : -webBaseSpeed;
        } else {
            targetSpeed = 0;
        }

        const int16_t RAMP = 15;
        if (motorSpeed < targetSpeed) motorSpeed = min((int)targetSpeed, motorSpeed + RAMP);
        else if (motorSpeed > targetSpeed) motorSpeed = max((int)targetSpeed, motorSpeed - RAMP);
        setMotorSpeed(motorSpeed);

        // 4. Baseline Calibration
        if (calibrationNeeded && systemEnabled) {
            if (motorSpeed == targetSpeed && targetSpeed != 0) {
                if (calibrationStartTime == 0) {
                    calibrationStartTime = millis();
                    calibrationSum = 0;
                    calibrationSamples = 0;
                }
                if (calibrationSamples < 40) {
                    calibrationSum += currentDraw;
                    calibrationSamples++;
                } else {
                    baselineCurrent = calibrationSum / 40.0;
                    calibrationNeeded = false;
                    calibrationStartTime = 0;
                }
            }
        }

        // 5. Penetration (Follicle) Counting
        if (systemEnabled && !calibrationNeeded && baselineCurrent > 0.05) {
            float spikeT = baselineCurrent + penetrationOffset;
            float fallT = spikeT - penetrationHysteresis;

            if (currentDraw > spikeT && !isPenetrating) {
                isPenetrating = true;
            } else if (currentDraw < fallT && isPenetrating) {
                isPenetrating = false;
                penetrationCount++;
                Serial.printf("Graft Detected! Total: %u\n", penetrationCount);
            }
        }

        // 6. Deferred NVM Save (Every 5s) - Prevents Camera Lag
        static unsigned long lastSave = 0;
        static uint32_t lastSavedCount = 0;
        if (millis() - lastSave > 5000) {
            if (penetrationCount != lastSavedCount) {
                preferences.putUInt("pCnt", penetrationCount);
                lastSavedCount = penetrationCount;
            }
            lastSave = millis();
        }

        // 7. Mechanical Stats
        float appliedV = batteryVoltage * (abs(motorSpeed) / 255.0);
        estimatedRPM = (uint32_t)(appliedV * (13000.0 / 3.0));
        batteryPower = batteryVoltage * currentDraw;

        // 8. Battery Heuristic
        if (batteryVoltage > 2.5) {
            float pc = (batteryVoltage - minVoltageThreshold) / (4.2f - minVoltageThreshold) * 100.0f;
            batteryPercent = (uint8_t)constrain(pc, 0, 100);
        } else {
            batteryPercent = 0;
        }
    }

    // Minimal delay to prevent Watchdog Triggers
    delay(1);
}