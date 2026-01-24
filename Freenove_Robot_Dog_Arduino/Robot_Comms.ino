#include "ard_Robot_Comms.h"
#include "ard_Robot_Drivers.h"
#include "ard_Robot_Motion.h"

namespace Comms {

    // ===================================================================================
    //  PARSER
    // ===================================================================================
    /**
     * @brief Parses incoming command strings.
     * @param msg Command string in format "CMD#Param1#Param2#...".
     */
    void Parser::parse(String msg) {
        cmdChar = 0;
        paramCount = 0;
        if (msg.length() == 0) return;

        // Format: CMD#P1#P2#...#
        int idx = msg.indexOf('#');
        if (idx > 0) {
            String cmdStr = msg.substring(0, idx);
            cmdChar = cmdStr.charAt(0);
            msg = msg.substring(idx + 1);
            
            while ((idx = msg.indexOf('#')) != -1 && paramCount < 10) {
                rawParams[paramCount] = msg.substring(0, idx);
                params[paramCount] = rawParams[paramCount].toFloat();
                paramCount++;
                msg = msg.substring(idx + 1);
            }
        }
    }

    // ===================================================================================
    //  BLE
    // ===================================================================================
    namespace BLE {
        BLEServer *pServer = NULL;
        BLECharacteristic *pTx = NULL;
        bool deviceConnected = false;

        // Handles BLE connection events
        class ServerCallbacks : public BLEServerCallbacks {
            void onConnect(BLEServer* pServer) { 
                deviceConnected = true; 
                Drivers::Buzzer::play(MELODY_BLE_CONNECT_SUCCESS);
            };
            void onDisconnect(BLEServer* pServer) { deviceConnected = false; pServer->startAdvertising(); }
        };

        // Handles incoming data from the BLE App
        class Callbacks : public BLECharacteristicCallbacks {
            void onWrite(BLECharacteristic *pCharacteristic) {
                String rxValue = pCharacteristic->getValue().c_str();
                if (rxValue.length() > 0) {
                    String s = rxValue;
                    // Simple newline splitting
                    int idx;
                    while((idx = s.indexOf('\n')) != -1) {
                        enterMessageQueue(s.substring(0, idx));
                        s = s.substring(idx+1);
                    }
                }
            }
        };

        // Initializes the BLE Server, Service, and Characteristics
        void begin() {
            BLEDevice::init("Freenove-Dog");
            pServer = BLEDevice::createServer();
            pServer->setCallbacks(new ServerCallbacks());
            BLEService *pService = pServer->createService("0000ffe0-0000-1000-8000-00805f9b34fb");
            pTx = pService->createCharacteristic("0000ffe1-0000-1000-8000-00805f9b34fb", BLECharacteristic::PROPERTY_NOTIFY);
            pTx->addDescriptor(new BLE2902());
            BLECharacteristic *pRx = pService->createCharacteristic("0000ffe1-0000-1000-8000-00805f9b34fb", BLECharacteristic::PROPERTY_WRITE);
            pRx->setCallbacks(new Callbacks());
            pService->start();
            pServer->getAdvertising()->start();
        }

        // Sends a string notification to the connected BLE device
        void send(String msg) {
            if (deviceConnected) {
                pTx->setValue((uint8_t*)msg.c_str(), msg.length());
                pTx->notify();
            }
        }

        void taskUpload(void *pvParameters) {
            if (!mqTx.isEmpty() && deviceConnected) {
                send(mqTx.out());
            }
        }
    }

    // ===================================================================================
    //  CAMERA
    // ===================================================================================
    bool Camera::isCameraNormal = false;
    
    // Define servers globally to prevent stack/heap issues during task creation
    WiFiServer serverCamera(8000);
    WiFiServer serverCmd(5000);
    
    // Configures the ESP32-CAM module pins and settings
    void Camera::begin() {
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = 4; config.pin_d1 = 5; config.pin_d2 = 18; config.pin_d3 = 19;
        config.pin_d4 = 36; config.pin_d5 = 39; config.pin_d6 = 34; config.pin_d7 = 35;
        config.pin_xclk = 21; config.pin_pclk = 22; config.pin_vsync = 25; config.pin_href = 23;
        config.pin_sccb_sda = 26; config.pin_sccb_scl = 27; config.pin_pwdn = -1; config.pin_reset = -1;
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_JPEG;
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 10;
        config.fb_count = 1;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_location = CAMERA_FB_IN_PSRAM; // Force PSRAM to save DRAM for WiFi

        if (esp_camera_init(&config) == ESP_OK) {
            Serial.println("Camera Init Success");
            isCameraNormal = true;
            setupWiFi();
        } else {
            Serial.println("Camera Init Failed");
            isCameraNormal = false;
        }
    }

    void Camera::setupWiFi() {
        WiFi.mode(WIFI_AP);
        String ssid = "FreenoveDog-" + getRobotId();
        WiFi.softAP(ssid.c_str(), "12345678");
        Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
        startServer();
        startCmdServer();
    }

    void Camera::startServer() {
        xTaskCreateUniversal(taskServer, "CamServer", 8192, NULL, 2, NULL, 0); // Priority 2 (Medium)
    }

    void Camera::startCmdServer() {
        xTaskCreateUniversal(taskCmdServer, "CmdServer", 4096, NULL, 1, NULL, 0);
    }

    /**
     * @brief Task to stream camera frames to connected WiFi clients.
     * @details Listens on Port 8000. Sends JPEG frames from the ESP32-CAM driver.
     */
    void Camera::taskServer(void *pvParameters) {
        serverCamera.begin();
        while(1) {
            WiFiClient client = serverCamera.available();
            if (client) {
                while (client.connected()) {
                    camera_fb_t *fb = esp_camera_fb_get();
                    if (fb) {
                        client.write(fb->buf, fb->len); // Simplified stream
                        esp_camera_fb_return(fb);
                    }
                    vTaskDelay(10);
                }
                client.stop();
            }
            vTaskDelay(100);
        }
    }

    /**
     * @brief Task to handle WiFi commands.
     * @details Listens on Port 5000. Receives control commands from the App via WiFi.
     */
    void Camera::taskCmdServer(void *pvParameters) {
        serverCmd.begin();
        while(1) {
            WiFiClient client = serverCmd.available();
            if (client) {
                client.setTimeout(2000);
                while (client.connected()) {
                    if (client.available()) {
                        String line = client.readStringUntil('\n');
                        enterMessageQueue(line);
                    }
                    if (!mqTx.isEmpty()) {
                        client.print(mqTx.out());
                    }
                    vTaskDelay(10);
                }
                client.stop();
            }
            vTaskDelay(100);
        }
    }

    // ===================================================================================
    //  COMMAND TASK
    // ===================================================================================
    Parser parser;
    
    /**
     * @brief Processes non-motion commands from the Info Queue.
     * @details Handles settings, LED control, Buzzer, and Network commands.
     */
    void taskCommand(void *pvParameters) {
        static bool isLegalController = false;
        if (!mqInfo.isEmpty()) {
            parser.parse(mqInfo.out());
            
            switch (parser.cmdChar) {
                case ACTION_RGB:
                    Drivers::LEDs::setRGB(parser.params[0], parser.params[1], parser.params[2], parser.params[3]);
                    break;
                case ACTION_BUZZER:
                    Drivers::Buzzer::setFreq(parser.params[0]);
                    break;
                case ACTION_GET_VOLTAGE:
                    // Handled by Battery Task automatically
                    break;
                case ACTION_SET_NVS:
                    if (parser.params[0] == 1) Drivers::LEDs::saveConfig();
                    if (parser.params[0] == 2) Drivers::NVS::clearAll();
                    break;
                case ACTION_AUTO_WALKING:
                    Drivers::Sonar::setAutoWalk(parser.params[0] == 1);
                    if (parser.params[0] == 0) enterMessageQueue(String(ACTION_MOVE_ANY) + "#0#0#0#0#");
                    break;
                case ACTION_SET_ROBOT:
                    // Sets global speed, handled in Motion or ignored if speed is passed in Move command
                    break;
                case ID_CHECK: // W#...
                    if (parser.paramCount >= 1) {
                        String s = "";
                        int subCmd = parser.params[0]; // W#0#...
                        if (subCmd == 0) s = String(ID_CHECK) + "#0#" + SW_VERSION + "#" + ROBOT_NAME + "#\n";
                        else if (subCmd == 1) s = String(ID_CHECK) + "#1#" + SW_VERSION + "#\n";
                        else if (subCmd == 2) s = String(ID_CHECK) + "#2#" + ROBOT_NAME + "#\n";
                        else if (subCmd == 3) s = String(ID_CHECK) + "#3#" + INTERNAL_CODE + "#\n";
                        // Simplified controller check
                        else if (subCmd == 4) { isLegalController = true; s = "Controller is legal."; }
                        
                        if (s.length() > 0) BLE::send(s);
                    }
                    break;
                case ACTION_NETWORK: // N#...
                    // Placeholder for WiFi logic matching TaskCommandService.cpp
                    if (parser.paramCount >= 1) {
                        int subCmd = parser.params[0];
                        if (subCmd == 0) { /* Scan WiFi */ }
                        else if (subCmd == 1) { /* Connect STA */ }
                        else if (subCmd == 2) { /* Disconnect */ }
                        else if (subCmd == 3) { Camera::setupWiFi(); } // Open AP
                        else if (subCmd == 4) { /* Close AP */ }
                        else if (subCmd == 5) { 
                            // Send Status
                            // BLE::send(...);
                        }
                    }
                    break;
            }
        }
    }
}