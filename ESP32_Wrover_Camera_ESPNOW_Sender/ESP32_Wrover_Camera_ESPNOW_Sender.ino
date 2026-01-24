/**
 * @file ESP32CAM_Sender.ino
 * @author Gemini Code Assist
 * @brief ESP32-CAM sketch to capture and send video frames via ESP-NOW.
 * 
 * This sketch initializes the camera, connects to a WiFi network to establish a
 * communication channel, and then continuously captures frames. Each frame is
 * broken down into small, sequenced packets and broadcasted over ESP-NOW.
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ============================================================================
//  1. CAMERA MODEL CONFIGURATION
// ============================================================================
// #define CAMERA_MODEL_AI_THINKER // Standard ESP32-CAM
#define CAMERA_MODEL_WROVER_KIT // Freenove Robot Dog (ESP32 Wrover Module)

// ============================================================================
//  2. PIN DEFINITIONS
// ============================================================================
#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     21
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       19
  #define Y4_GPIO_NUM       18
  #define Y3_GPIO_NUM        5
  #define Y2_GPIO_NUM        4
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

// ============================================================================
// Use broadcast address to send to all ESP-NOW devices
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Flag to trigger resolution switch in main loop
volatile bool needSwitchRes = false;
volatile bool frameRequested = false;

// Receive Callback
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  // Check for 1-byte commands
  if (len == 1) {
    if (incomingData[0] == 0x01) needSwitchRes = true;
    if (incomingData[0] == 0x02) frameRequested = true;
  }
}

// ============================================================================
//  SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // --- 1. Camera Initialization ---
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_4;
  config.ledc_timer = LEDC_TIMER_2;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QQVGA; // 160x120
  config.jpeg_quality = 14; // Lower quality for higher FPS
  config.fb_count = 2; // Use 2 buffers for higher FPS
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // Flip the image vertically
  s->set_hmirror(s, 1); // Flip the image horizontally

  // --- 2. WiFi & ESP-NOW Initialization ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Disconnect from any saved WiFi networks
  // Disable WiFi power saving to reduce latency
  esp_wifi_set_ps(WIFI_PS_NONE);

  // Force channel 1 for direct connection
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.print("Sender MAC Address: "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Add the broadcast peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register receive callback to listen for commands from Receiver
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Initialized. Sending frames...");
}

// ============================================================================
//  LOOP
// ============================================================================
void loop() {
  // --- Check for Resolution Switch Command ---
  if (needSwitchRes) {
    needSwitchRes = false;
    sensor_t * s = esp_camera_sensor_get();
    framesize_t currentSize = s->status.framesize;
    
    if (currentSize == FRAMESIZE_QQVGA) { // 160x120
      s->set_framesize(s, FRAMESIZE_QVGA); // 320x240
    } else if (currentSize == FRAMESIZE_QVGA) {
      s->set_framesize(s, FRAMESIZE_HQVGA); // 240x176
    } else {
      s->set_framesize(s, FRAMESIZE_QQVGA); // Back to 160x120
    }
    Serial.printf("Switched resolution to: %d\n", s->status.framesize);
    delay(100); // Give sensor time to settle
  }

  // Wait for request from receiver
  if (!frameRequested) {
    delay(1);
    return;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // --- Packetize and send the frame ---
  size_t offset = 0;
  uint16_t chunkSeq = 0;
  uint8_t packetData[250]; // 2 bytes for seq, 248 for data

  while (offset < fb->len) {
    size_t chunkSize = (fb->len - offset) > 248 ? 248 : (fb->len - offset);
    
    packetData[0] = chunkSeq & 0xFF;
    packetData[1] = (chunkSeq >> 8) & 0xFF;
    memcpy(&packetData[2], fb->buf + offset, chunkSize);
    
    esp_now_send(broadcastAddress, packetData, chunkSize + 2);
    
    offset += chunkSize;
    chunkSeq++;
    delayMicroseconds(500); // Reduced delay for faster transmission (receiver is ready)
  }

  // If the frame length is a multiple of 248, the last packet was full.
  // Send a final packet with only the sequence number (length 2) to signal the end.
  if (fb->len > 0 && fb->len % 248 == 0) {
    packetData[0] = chunkSeq & 0xFF;
    packetData[1] = (chunkSeq >> 8) & 0xFF;
    esp_now_send(broadcastAddress, packetData, 2);
  }

  esp_camera_fb_return(fb);
  frameRequested = false; // Reset flag, wait for next request
  // delay(20); // Removed, flow control is now handled by request-response
}