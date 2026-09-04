/**
 * @file Freenove_Robot_Dog_Arduino.ino
 * @brief Main Firmware Entry Point
 * @details Consolidated firmware for Freenove Robot Dog ESP32.
 *          Organized into .ino tabs for Arduino IDE compatibility.
 * 
 * @attention COMPILATION SETTINGS:
 *  - Board: ESP32 Wrover Module
 *  - Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
 *  - PSRAM: Enabled
 */

// --- Board Check ---
#if !defined(ESP32)
  #error "This code is intended for ESP32 boards. Please select an ESP32 board in the Tools > Board menu."
#endif

#include "ard_Robot_Global.h"
#include "ard_Robot_Drivers.h"
#include "ard_Robot_Comms.h"
#include "ard_Robot_Motion.h"

// ===================================================================================
//  GLOBAL MESSAGE QUEUES
// ===================================================================================
DataQueue<String> mqMotion(2);  // Queue for motion commands (High priority)
DataQueue<String> mqInfo(100);  // Queue for general info/settings
DataQueue<String> mqTx(20);     // Queue for outgoing BLE/WiFi messages
DataQueue<int> mqBz(20);        // Queue for Buzzer melodies

/**
 * @brief System Initialization.
 * @details Configures Serial, Drivers, NVS, Communications, and starts FreeRTOS tasks.
 */
void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true); // Enable detailed camera logs
    delay(1000); // Allow power to stabilize
    Serial.println("\n\n[System] Program begin...");

    if(psramFound()){
        Serial.printf("PSRAM Found! Total: %d bytes, Free: %d bytes\n", ESP.getPsramSize(), ESP.getFreePsram());
    } else {
        Serial.println("PSRAM Not Found! Check Tools > PSRAM > Enabled");
    }

    // 1. Initialize Drivers & NVS
    Drivers::NVS::begin();          // Non-Volatile Storage

    // 2. Initialize Camera FIRST (Critical for I2C/Timer resources)
    // Initialize before BLE to avoid radio noise/power spikes affecting SCCB
    Comms::Camera::begin();
    delay(100);

    // 3. Initialize Communications
    Comms::BLE::begin();            // Bluetooth Low Energy
    delay(100);

    // 4. Initialize Remaining Hardware
    Drivers::PCA9685::begin();      // Servo Driver
    Drivers::PCA9685::releaseAll(); // Relax servos on boot
    
    Drivers::Battery::begin();      // Battery Monitor
    Drivers::Buzzer::begin();       // Audio
    Drivers::Sonar::begin();        // Ultrasonic
    Drivers::LEDs::begin();         // RGB & Built-in LEDs
    Drivers::Touch::begin();        // Capacitive Touch

    // 3. Load Configuration from NVS
    Motion::loadCalibration();      // Load servo offsets
    Drivers::LEDs::loadConfig();    // Load LED patterns

    // 4. Start Background Managers
    Comms::Radio::begin();          // Radio State Manager

    Serial.println("[System] Setup finished!");

    // 5. Start RTOS Tasks
    // Motion Control Task (Core 1) - Handles complex kinematics
    xTaskCreateUniversal(Motion::taskMotion, "Motion_Task", 8192, NULL, 5, NULL, 1); // Priority 5 (High)
    
    // Secondary Loop Task (Core 1) - Handles background sensors/LEDs
    xTaskCreateUniversal(loopSecondary, "Secondary_Task", 8192, NULL, 1, NULL, 1); // Priority 1 (Low)

    // 6. Startup Feedback
    if (!Comms::Camera::isCameraNormal) {
        Serial.println("Camera Init Failed - Playing Failure Melody");
        Drivers::Buzzer::play(MELODY_CAM_FAILURE); 
    }
    Drivers::Buzzer::play(MELODY_POWER_UP);
    
    // Initial Posture
    Serial.println("Executing standUp()...");
    delay(1000); // Pause to ensure power is stable before high-current move
    Motion::standUp();
    Serial.println("standUp() completed.");
}

/**
 * @brief Main Loop (Core 1).
 * @details Handles low-priority tasks like command dispatching and built-in LED blinking.
 */
void loop() {
    // Main Loop: Handles Command Processing and BLE Uploads
    Comms::taskCommand(NULL);       // Process incoming commands
    Comms::BLE::taskUpload(NULL);   // Handle BLE data transmission
    Drivers::LEDs::taskBuiltIn(NULL); // Blink built-in LED

    // Handle Serial Input for Debugging/Wired Control
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        if (input.length() > 0) {
            enterMessageQueue(input);
        }
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
}

/**
 * @brief Secondary Loop Task
 * @details Handles background tasks like battery monitoring, LED effects, 
 *          auto-walking logic, and buzzer sounds.
 */
void loopSecondary(void *pvParameters) {
    while (1) {
        Drivers::Battery::taskMonitor(NULL);
        Drivers::LEDs::taskRGB(NULL);
        Drivers::Sonar::taskAutoWalk(NULL);
        Drivers::Buzzer::taskService(NULL);
        Drivers::Touch::taskMonitor(NULL);
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

// Global Helper to route messages (used by Comms and Serial)
void enterMessageQueue(String msg) {
    Serial.print("Rx: "); Serial.println(msg); // Debug incoming messages
    char cmd = msg.charAt(0);
    // Check if command is related to motion (Action, Install, Calibrate, Height, Move, Twist, Dance)
    if (strchr("ABEFJLO", cmd)) {
        mqMotion.enterForced(msg);
    } else {
        mqInfo.enterForced(msg);
    }
}

String getRobotId() {
    uint64_t chipId = ESP.getEfuseMac();
    String chipIdStr = String((uint32_t)chipId, HEX);
    chipIdStr.toUpperCase();
    return chipIdStr;
}