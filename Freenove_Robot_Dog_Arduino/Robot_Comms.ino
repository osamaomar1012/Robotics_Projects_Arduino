#include "ard_Robot_Comms.h"
#include "ard_Robot_Drivers.h"
#include "ard_Robot_Motion.h"
#include <esp_now.h>
#include <esp_wifi.h>

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
                // Enforce Modem Sleep to prevent radio crashes during coexistence
                esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
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
            // CRITICAL FIX: Enable Modem Sleep BEFORE BLE init.
            // Initializing BT controller while WiFi is in WIFI_PS_NONE causes an abort.
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
            BLEDevice::init("Freenove-Dog");
            pServer = BLEDevice::createServer();
            pServer->setCallbacks(new ServerCallbacks());
            BLEService *pService = pServer->createService("0000ffe0-0000-1000-8000-00805f9b34fb");
            pTx = pService->createCharacteristic("0000ffe1-0000-1000-8000-00805f9b34fb", BLECharacteristic::PROPERTY_NOTIFY);
            pTx->addDescriptor(new BLE2902());
            BLECharacteristic *pRx = pService->createCharacteristic("0000ffe1-0000-1000-8000-00805f9b34fb", BLECharacteristic::PROPERTY_WRITE);
            pRx->setCallbacks(new Callbacks());
            pService->start();
            
            // OPTIMIZATION: Increase Advertising Interval to 100ms (Default is ~20ms)
            // This frees up radio time for ESP-NOW video streaming.
            BLEAdvertising *pAdvertising = pServer->getAdvertising();
            pAdvertising->setMinInterval(0xA0); // 0xA0 * 0.625ms = 100ms
            pAdvertising->setMaxInterval(0xA0); 
            pAdvertising->start();
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
    //  RADIO MANAGER
    // ===================================================================================
    namespace Radio {
        volatile unsigned long lastRemoteActivity = 0;
        bool isRemoteOnline = false;

        // Called by OnDataRecv when packet arrives
        void touchRemote() {
            lastRemoteActivity = millis();
        }

        // Background task to monitor connection states
        void taskMonitor(void *pvParameters) {
            Serial.println("[Radio] Manager Started");
            static bool lastBleState = false;
            static bool lastRemoteState = false;

            while(1) {
                // 1. Check ESP-NOW Remote Heartbeat (Timeout 2s)
                bool currentRemote = false;
                if (millis() - lastRemoteActivity < 2000 && lastRemoteActivity != 0) {
                    currentRemote = true;
                }

                // Handle Remote State Changes
                if (currentRemote != isRemoteOnline) {
                    isRemoteOnline = currentRemote;
                    if (isRemoteOnline) {
                        Serial.println("[Radio] State: REMOTE_CONNECTED");
                        // If BLE is already online, play Remote Request Tone
                        if (BLE::deviceConnected) {
                            Drivers::Buzzer::play(MELODY_REMOTE_REQUEST);
                        }
                    } else {
                        Serial.println("[Radio] State: REMOTE_LOST");
                    }
                }

                // 2. Check Bluetooth State
                if (BLE::deviceConnected != lastBleState) {
                    lastBleState = BLE::deviceConnected;
                    Serial.printf("[Radio] State: BLE_%s\n", lastBleState ? "CONNECTED" : "DISCONNECTED");
                    
                    if (lastBleState) {
                        // BLE Just Connected. If Remote is already online, play BLE Request Tone
                        if (isRemoteOnline) {
                            Drivers::Buzzer::play(MELODY_BLE_REQUEST);
                        }
                    }
                }

                vTaskDelay(200 / portTICK_PERIOD_MS); // Faster polling for responsiveness
            }
        }

        void begin() {
            xTaskCreateUniversal(taskMonitor, "RadioMgr", 4096, NULL, 1, NULL, 1);
        }
    }

    // ===================================================================================
    //  CAMERA
    // ===================================================================================
    bool Camera::isCameraNormal = false;
    
    // Define servers globally
    WiFiServer serverCamera(8000);
    WiFiServer serverCmd(5000);
    
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    volatile bool needSwitchRes = false;
    volatile bool frameRequested = false;

    // --- ESP-NOW Callback ---
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
    #else
    void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    #endif
        // Update Radio Manager (Heartbeat)
        Radio::touchRemote();

        // PRIORITY: If BLE is connected, ignore ESP-NOW commands to prevent radio conflict
        // This ensures the single radio isn't overwhelmed by video requests while handling BLE.
        if (BLE::deviceConnected) return;

        // 1. Handle Camera Commands
        if (len == 1) {
            if (incomingData[0] == 0x01) needSwitchRes = true;
            if (incomingData[0] == 0x02) frameRequested = true;
        }
    }

    // Configures the ESP32-CAM module pins and settings
    void Camera::begin() {
        // 1. Force De-init to clear previous state (Soft Reset fix)
        esp_camera_deinit();

        // 2. Manually Reset I2C Bus (Clear stuck lines)
        pinMode(26, OUTPUT); digitalWrite(26, HIGH); // SDA
        pinMode(27, OUTPUT); digitalWrite(27, HIGH); // SCL
        delay(10);

        // 3. Configure Camera
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_4; // Changed to avoid conflict with other PWM
        config.ledc_timer = LEDC_TIMER_2;
        config.pin_d0 = 4; config.pin_d1 = 5; config.pin_d2 = 18; config.pin_d3 = 19;
        config.pin_d4 = 36; config.pin_d5 = 39; config.pin_d6 = 34; config.pin_d7 = 35;
        config.pin_xclk = 21; config.pin_pclk = 22; config.pin_vsync = 25; config.pin_href = 23;
        config.pin_sccb_sda = 26; config.pin_sccb_scl = 27; config.pin_pwdn = -1; config.pin_reset = -1;
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_JPEG;
        config.frame_size = FRAMESIZE_QQVGA; // Default low res for speed
        config.jpeg_quality = 14;
        config.fb_count = 1; // Reduce to 1 to ensure enough contiguous RAM for BLE init
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_location = CAMERA_FB_IN_PSRAM; // Force PSRAM to save DRAM for BLE/WiFi

        esp_err_t err = esp_camera_init(&config);

        if (err == ESP_OK) {
            Serial.println("Camera Init Success");
            sensor_t * s = esp_camera_sensor_get();
            s->set_vflip(s, 1); 
            s->set_hmirror(s, 1);
            isCameraNormal = true;
            
            // --- Init WiFi AP & ESP-NOW ---
            // We use AP mode on Channel 1 so both App and Remote can connect
            setupWiFi(); 
            // Use NONE during init to prevent resource exhaustion/crashes during peer addition
            esp_wifi_set_ps(WIFI_PS_NONE);
            startCmdServer();

            if (esp_now_init() == ESP_OK) {
                Serial.println("ESP-NOW Init Success");
                esp_now_peer_info_t peerInfo;
                memset(&peerInfo, 0, sizeof(peerInfo));
                memcpy(peerInfo.peer_addr, broadcastAddress, 6);
                peerInfo.channel = 0;  
                peerInfo.encrypt = false;
                // Explicitly use the Station interface for ESP-NOW (works best with AP_STA)
                peerInfo.ifidx = WIFI_IF_STA;
                esp_now_add_peer(&peerInfo);
                esp_now_register_recv_cb(OnDataRecv);
                
                // Start the Camera Task
                xTaskCreateUniversal(taskServer, "CamTask", 4096, NULL, 2, NULL, 0);
            } else {
                Serial.println("ESP-NOW Init Failed");
            }
        } else {
            Serial.println("Camera Init Failed");
            isCameraNormal = false;
        }
    }

    void Camera::setupWiFi() {
        // Use AP_STA mode. ESP-NOW defaults to the STA interface.
        // Even if we don't connect to a router, STA must be active for ESP-NOW to work reliably alongside AP.
        WiFi.mode(WIFI_AP_STA);
        WiFi.disconnect(); // Ensure STA interface is in a clean state
        String ssid = "FreenoveDog-" + getRobotId();
        // Force Channel 1 so ESP-NOW receiver knows where to listen
        WiFi.softAP(ssid.c_str(), "12345678", 1, 0, 4);
        Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
        Serial.print("AP SSID: "); Serial.println(ssid);
    }

    void Camera::startCmdServer() {
        xTaskCreateUniversal(taskCmdServer, "CmdServer", 4096, NULL, 1, NULL, 0);
    }

    /**
     * @brief Task to stream camera frames via ESP-NOW AND WiFi (HTTP).
     */
    void Camera::taskServer(void *pvParameters) {
        serverCamera.begin();
        serverCamera.setNoDelay(true);
        WiFiClient client;

        while(1) {
            // 1. Handle Resolution Switch
            if (needSwitchRes) {
                needSwitchRes = false;
                sensor_t * s = esp_camera_sensor_get();
                framesize_t currentSize = s->status.framesize;
                if (currentSize == FRAMESIZE_QQVGA) s->set_framesize(s, FRAMESIZE_QVGA);
                else if (currentSize == FRAMESIZE_QVGA) s->set_framesize(s, FRAMESIZE_HQVGA);
                else s->set_framesize(s, FRAMESIZE_QQVGA);
                vTaskDelay(100);
            }

            // 2. Check for new WiFi Client (Phone App)
            if (serverCamera.hasClient()) {
                if (client) client.stop();
                client = serverCamera.available();
            }

            bool wifiActive = (client && client.connected());
            // Only enable ESP-NOW if remote is online, requesting frames, AND BLE is NOT connected
            bool espnowActive = frameRequested && Radio::isRemoteOnline && !BLE::deviceConnected;

            // If no one wants a frame, sleep briefly
            if (!wifiActive && !espnowActive) {
                vTaskDelay(10);
                continue;
            }

            // 3. Capture & Send
            camera_fb_t *fb = esp_camera_fb_get();
            if (fb) {
                // --- A. Send to ESP-NOW Remote (Priority) ---
                if (espnowActive) {
                size_t offset = 0;
                uint16_t chunkSeq = 0;
                uint8_t packetData[250];

                while (offset < fb->len) {
                    size_t chunkSize = (fb->len - offset) > 248 ? 248 : (fb->len - offset);
                    packetData[0] = chunkSeq & 0xFF;
                    packetData[1] = (chunkSeq >> 8) & 0xFF;
                    memcpy(&packetData[2], fb->buf + offset, chunkSize);
                    
                    esp_now_send(broadcastAddress, packetData, chunkSize + 2);
                    
                    offset += chunkSize;
                    chunkSeq++;
                    delayMicroseconds(500); // Throttle for receiver
                }
                
                // End of Frame Signal
                if (fb->len > 0 && fb->len % 248 == 0) {
                    packetData[0] = chunkSeq & 0xFF;
                    packetData[1] = (chunkSeq >> 8) & 0xFF;
                    esp_now_send(broadcastAddress, packetData, 2);
                }
                frameRequested = false;
                }

                // --- B. Send to WiFi App (MJPEG Stream) ---
                if (wifiActive) {
                    String response = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fb->len) + "\r\n\r\n";
                    client.write(response.c_str(), response.length());
                    client.write(fb->buf, fb->len);
                    client.write("\r\n", 2);
                }

                esp_camera_fb_return(fb);
            }
        }
    }

    /**
     * @brief Task to handle WiFi commands from the Phone App.
     * @details Listens on Port 5000.
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