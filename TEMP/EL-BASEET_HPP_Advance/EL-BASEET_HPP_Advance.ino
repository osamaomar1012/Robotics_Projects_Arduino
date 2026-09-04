#include "BoardConfig.h"
#include "camera_handler.h"
#include "web_server.h"
#include <Preferences.h>
#include <Wire.h>

#if USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

// Global Control & Safety Variables
Preferences preferences;
uint8_t targetMac[6];
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
uint32_t screenSwitchTime = 5000; 
bool forwardDirection = true;  
uint8_t currentScreen = 0;      
unsigned long lastScreenSwitch = 0;
bool safetyTripped = false; 
bool lowBatteryTripped = false; 
bool systemEnabled = false; 
bool debugMode = false;     
bool wifiConnecting = false; 

#if USE_OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#endif

void displayWebConfirmation(String msg) {
    Serial.println("HPP CMD: " + msg);
#if USE_OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.println(msg);
    display.display();
#endif
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
    screenSwitchTime = preferences.getUInt("scrTime", 5000);
    maxCurrentThreshold = preferences.getFloat("maxCurr", 2.5);
    minVoltageThreshold = preferences.getFloat("minVolt", 3.2);
    penetrationCount = preferences.getUInt("pCnt", 0);
    penetrationOffset = preferences.getFloat("penOff", 0.04);
    penetrationHysteresis = preferences.getFloat("penHyst", 0.02);
    debugMode = preferences.getBool("debug", false);

#if USE_OLED
    // Initialize OLED for Local Telemetry
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.println("HPP_V3 BOOTING...");
        display.display();
    }
#endif

    // Camera Feature Disabled for Sensor Validation
    // if (setupCamera()) {
    //     Serial.println("Camera Engine: OK");
    //     cameraActive = true;
    // } else {
    //     Serial.println("Camera Engine: FAILED");
    // }
    cameraActive = false;

    // Initialize Surgical Pen Motor Driver
    ledcSetup(0, 5000, 8); 
    ledcSetup(1, 5000, 8); 
    ledcAttachPin(PIN_MOTOR_IN1, 0);
    ledcAttachPin(PIN_MOTOR_IN2, 1);
    setMotorSpeed(0);

    pinMode(CAM_PIN_FLASH, OUTPUT); // Pre-init flash pin

#if USE_SENSORS
    // Initialize Sensors
    analogReadResolution(12);
#endif

    setupWeb();
    
    Serial.print("\n--- SENDER CONFIGURATION ---\n");
    Serial.printf("Device MAC: %s\n", WiFi.softAPmacAddress().c_str());

    // ESP-NOW and Camera features disabled for control/reading debugging
    // if (esp_now_init() == ESP_OK) {
    //     Serial.println("ESP-NOW Engine: OK");
    //     esp_now_register_recv_cb(onEspNowRecv); 

    //     memcpy(targetMac, broadcastAddress, 6);
    //     esp_now_peer_info_t peerInfo = {};
    //     memcpy(peerInfo.peer_addr, targetMac, 6);
    //     peerInfo.channel = 1; 
    //     peerInfo.encrypt = false;
    //     peerInfo.ifidx = WIFI_IF_AP; 
    //     esp_now_add_peer(&peerInfo);
    //     xTaskCreatePinnedToCore(espNowStreamTask, "esp_now_strm", 4096, NULL, 5, NULL, 0);
    // }

    lastUpdate = millis();
    lastScreenSwitch = millis();
    Serial.println("System Ready. Connect to WiFi HPP_ADVANCE_SURGERY");
}

void loop() {
    server.handleClient();

    // High-Precision Control Loop (50ms)
    if (millis() - lastUpdate > 50) {
        lastUpdate = millis();

        #if USE_SENSORS
        // 1. Read and Smooth Telemetry
        float rawI = (analogRead(PIN_MAX471_AT) / 4095.0) * 3.3; 
        currentDraw = (rawI * CURRENT_ALPHA) + (currentDraw * (1.0 - CURRENT_ALPHA));
        
        float rawV = (analogRead(PIN_MAX471_VT) / 4095.0) * 3.3 * 5.0; 
        batteryVoltage = (rawV * CURRENT_ALPHA) + (batteryVoltage * (1.0 - CURRENT_ALPHA));
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
                preferences.putUInt("pCnt", penetrationCount);
                Serial.printf("Graft Detected! Total: %u\n", penetrationCount);
            }
        }

        // 6. Mechanical Stats
        float appliedV = batteryVoltage * (abs(motorSpeed) / 255.0);
        estimatedRPM = (uint32_t)(appliedV * (13000.0 / 3.0));
        batteryPower = batteryVoltage * currentDraw;

        // 7. Battery Heuristic
        if (batteryVoltage > 2.5) {
            float pc = (batteryVoltage - minVoltageThreshold) / (4.2f - minVoltageThreshold) * 100.0f;
            batteryPercent = (uint8_t)constrain(pc, 0, 100);
        } else {
            batteryPercent = 0;
        }
    }

#if USE_OLED
    // Update Local OLED
    static unsigned long lastOLED = 0;
    if(millis() - lastOLED > 500) {
        display.clearDisplay();
        display.setCursor(0,0);
        display.printf("V: %.2fV\nA: %.2fA", batteryVoltage, currentDraw);
        display.display();
        lastOLED = millis();
    }
#endif

    // Minimal delay to prevent Watchdog Triggers
    delay(1);
}