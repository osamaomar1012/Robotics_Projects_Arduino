#include <WiFi.h>
#include <esp_now.h>
#include <TJpg_Decoder.h>
#include <Preferences.h>
#include <esp_wifi.h>
#include "App_Globals.h"
#include "UI_Components.h"
#include "Menu_Handler.h"
#include "LiveView_Handler.h"
#include "SurgicalConfig_Handler.h"
#include "CameraConfig_Handler.h"
#include "Calibrate_Handler.h"
#include "Info_Handler.h"

// Global Instantiations
LGFX gfx;
AppMode currentMode = MODE_WELCOME;
Preferences prefs;
uint8_t* frameBuffer;
volatile size_t currentFramePos = 0;
volatile bool frameReady = false;
volatile uint16_t lastSeq = 0;
unsigned long lastFrameTime = 0;
volatile int lastRSSI = 0;
// IMPORTANT: Replace with the actual MAC address of your ESP32-CAM Sender
// You can find the ESP32-CAM's MAC in its Serial Monitor output during setup.
uint8_t surgicalPenMac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; // Placeholder MAC

const uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Using pushImageDMA as seen in HMI2-8.ino
bool gfx_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (y >= gfx.height()) return false;
    gfx.pushImage(x, y, w, h, bitmap);
    return true;
}

// --- ESP-NOW Receiver Callback ---
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    const uint8_t* mac = info->src_addr;
    lastRSSI = info->rx_ctrl->rssi; // Capture signal strength
#else
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
#endif
    if (len < 2) return;
    uint16_t seq = incomingData[0] | (incomingData[1] << 8);
    if (seq == 0) {
        currentFramePos = 0;
        frameReady = false;
        lastSeq = 0;
    } else {
        if (seq != lastSeq + 1) { currentFramePos = 0; return; }
        lastSeq = seq;
    }
    size_t payloadLen = len - 2;
    if (frameBuffer && (currentFramePos + payloadLen < MAX_FRAME_SIZE)) {
        memcpy(frameBuffer + currentFramePos, incomingData + 2, payloadLen);
        currentFramePos += payloadLen;
        if (currentFramePos > 2 && 
            frameBuffer[currentFramePos - 2] == 0xFF && 
            frameBuffer[currentFramePos - 1] == 0xD9) {
            frameReady = true;
            lastFrameTime = millis();
        }
    }
}

// Helper to send commands to the Surgical Pen
void sendRemoteCommand(uint8_t cmd, uint8_t val = 0) {
    uint8_t pkt[2] = {cmd, val}; // Command and value
    esp_now_send(surgicalPenMac, pkt, 2); // Send to specific Surgical Pen MAC
    digitalWrite(BUZZER_PIN, HIGH); delay(10); digitalWrite(BUZZER_PIN, LOW); // Haptic feedback
}

void setup() {
    Serial.begin(115200);
    prefs.begin("comms", false);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SPEAKER_EN, OUTPUT);
    digitalWrite(SPEAKER_EN, HIGH);
    pinMode(38, OUTPUT);
    digitalWrite(38, HIGH);
    
    gfx.init();
    gfx.setBrightness(128); 
    gfx.initDMA();
    gfx.startWrite();
    gfx.setRotation(1);
    showWelcomeScreen();

    updateWelcomeProgress(20, "Allocating Memory...");
    frameBuffer = (uint8_t*)ps_malloc(MAX_FRAME_SIZE);
    if (!frameBuffer) { Serial.println("PSRAM Fail!"); while(1); }

    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(gfx_output);

    updateWelcomeProgress(40, "Initializing Radio...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() != ESP_OK) return;

    // Match performance settings on receiver
    esp_wifi_set_ps(WIFI_PS_NONE);

    updateWelcomeProgress(60, "Configuring ESP-NOW...");
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    esp_now_register_recv_cb(OnDataRecv);
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, surgicalPenMac, 6); // Peer for Surgical Pen
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    Serial.printf("CrowPanel MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("Configured Surgical Pen MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                  surgicalPenMac[0], surgicalPenMac[1], surgicalPenMac[2], surgicalPenMac[3], surgicalPenMac[4], surgicalPenMac[5]);

    updateWelcomeProgress(80, "Syncing with Camera...");
    unsigned long startWait = millis();
    while (lastFrameTime == 0 && (millis() - startWait < 8000)) {
        static unsigned long lastRetry = 0;
        if (millis() - lastRetry > 500) {
            uint8_t msg = 0x02; esp_now_send(broadcastAddress, &msg, 1);
            lastRetry = millis();
        }
        int pulse = 80 + ((millis() / 200) % 20);
        updateWelcomeProgress(pulse, "SEARCHING FOR SENDER...");
        delay(50);
    }
    updateWelcomeProgress(100, lastFrameTime > 0 ? "CONNECTION SECURE" : "BYPASS READY");
    delay(1200);
    currentMode = MODE_MENU;
    drawMenu();
}

void loop() {
    static unsigned long lastRequest = 0;
    static unsigned long lastTouch = 0;
    uint16_t tx, ty;
    bool touched = gfx.getTouch(&tx, &ty);

    if (touched && millis() - lastTouch > 300) {
        lastTouch = millis();
        if (currentMode == MODE_MENU) {
            // Menu selection logic
            if (tx >= 50 && tx <= 270) {
                if (ty >= 60 && ty <= 90) { currentMode = MODE_LIVE; gfx.fillScreen(TFT_BLACK); }
                else if (ty >= 96 && ty <= 126) { currentMode = MODE_CONFIG_SURGICAL; drawSurgicalConfigScreen(); }
                else if (ty >= 132 && ty <= 162) { 
                    // Example: Toggle motor power from menu
                    static bool pwr = false; pwr = !pwr;
                    sendRemoteCommand(0x10, pwr ? 1 : 0);
                }
                else if (ty >= 168 && ty <= 198) { currentMode = MODE_INFO; drawInfoScreen(); }
                else if (ty >= 204 && ty <= 234) { currentMode = MODE_CALIBRATE; runTouchCalibration(); }
            }
        }
        else if (currentMode == MODE_LIVE) {
            // Overlays for Live View Control
            if (ty > 200) { currentMode = MODE_MENU; drawMenu(); }
            else if (tx < 60) sendRemoteCommand(0x11); // Left side: Toggle Direction
            else if (tx > 260) sendRemoteCommand(0x14); // Right side: Reset Grafts
        }
        else { currentMode = MODE_MENU; drawMenu(); }
    }

    // Update menu animations if we are sitting on the menu screen
    if (currentMode == MODE_MENU) {
        updateMenuAnimations();
    }

    if (currentMode == MODE_LIVE) {
        processLiveStream();
    }

    // HEARTBEAT ARCHITECTURE:
    // We no longer request frames 1-by-1. We send a "Stay Alive" ping.
    // The Sender (Pen) will Push frames as fast as possible.
    if (currentMode == MODE_LIVE) {
        if (millis() - lastRequest > 500) { 
            sendRemoteCommand(0x02); // Keep-alive for Push Mode
            lastRequest = millis();
        }
    }
    yield(); // Keep watchdog happy
}