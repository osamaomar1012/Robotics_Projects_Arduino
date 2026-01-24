/**
 * @file TTGO_Receiver.ino
 * @author Gemini Code Assist
 * @brief TTGO T-Display sketch to receive and display video frames from ESP-NOW.
 * 
 * This sketch connects to a WiFi network to establish a channel, then listens
 * for incoming ESP-NOW packets. It reassembles the packets into a complete
 * JPEG image, decodes it using a secondary core, and displays it on the TFT screen.
 * It also displays live RSSI and FPS statistics.
 */

#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ============================================================================
//  2.1 HARDWARE DEFINITIONS
// ============================================================================
#define BUTTON_PIN 0 // Onboard button on TTGO T-Display

// ============================================================================
//  2. GLOBALS
// ============================================================================

// --- ESP-NOW Globals ---
#define MAX_JPG_BUFFER_SIZE (15 * 1024) // 15KB buffer for the JPEG
uint8_t jpg_buffer[MAX_JPG_BUFFER_SIZE];
size_t jpg_buffer_len = 0;
volatile bool esp_now_frame_ready = false;
volatile unsigned long last_packet_time = 0;
volatile uint16_t expectedSeq = 0;
volatile bool frameCorrupt = false;
uint8_t senderMac[6];
bool senderMacKnown = false;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// --- TFT & JPEG Decoder Globals ---
TFT_eSPI tft = TFT_eSPI();
TaskHandle_t Task1 = NULL;          
const uint8_t* arrayName;           
uint16_t arrayLength;               
volatile bool doDecoding = false;            
volatile bool mcuReady = false;              
uint16_t mcuBuffer[16*16];          
int32_t mcu_x, mcu_y, mcu_w, mcu_h; 
bool isFirstFrame = true;

// --- Stats Globals ---
uint32_t frameCount = 0;
unsigned long lastFpsTime = 0;
int currentRes = 0; // 0: QQVGA, 1: QVGA, 2: HQVGA

// ============================================================================
//  3. ESP-NOW CALLBACK
// ============================================================================

/**
 * @brief Callback function executed when ESP-NOW data is received.
 * Handles packet reassembly and sequence checking.
 */
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  const uint8_t * mac = info->src_addr;
#else
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
#endif
  // A valid packet must have at least 2 bytes for the sequence number.
  if (len < 2) return;

  // Capture Sender MAC Address to reply later
  if (!senderMacKnown) {
    memcpy(senderMac, mac, 6);
    senderMacKnown = true;
    // Register sender as a peer to enable sending back
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, senderMac, 6);
    peerInfo.channel = 0; // Use current channel
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  uint16_t seq = incomingData[0] | (incomingData[1] << 8);

  // A packet with sequence 0 marks the beginning of a new frame.
  if (seq == 0) {
    // Only start a new frame if the previous one has been processed.
    if (!esp_now_frame_ready) {
      jpg_buffer_len = 0;
      expectedSeq = 0;
      frameCorrupt = false;
    }
  }

  if (esp_now_frame_ready) return; // Buffer is busy, ignore packet.
  if (frameCorrupt) return;        // Frame is corrupt, ignore subsequent packets.

  // Check if the packet sequence is the one we expect.
  if (seq != expectedSeq) {
    // Serial.printf("Sequence mismatch! Expected %d, got %d. Dropping frame.\n", expectedSeq, seq); // Commented out to prevent crash
    frameCorrupt = true;
    return;
  }

  int dataLen = len - 2;
  // Check for buffer overflow.
  if (jpg_buffer_len + dataLen > MAX_JPG_BUFFER_SIZE) {
    // Serial.println("ESP-NOW buffer overflow! Discarding frame."); // Commented out to prevent crash
    frameCorrupt = true;
    return;
  }

  // Append the image data (after the 2-byte header) to our buffer.
  memcpy(&jpg_buffer[jpg_buffer_len], incomingData + 2, dataLen);
  jpg_buffer_len += dataLen;
  last_packet_time = millis();
  expectedSeq++;

  // If the packet is smaller than the max size (250), it's the last one.
  // This signals that the frame is complete and ready for decoding.
  if (len < 250) {
    esp_now_frame_ready = true;
  }
}

// ============================================================================
//  4. SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // --- TFT & JPEG Decoder Initialization ---
  tft.begin();
  tft.setRotation(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true); // Correct color byte order for JPEG rendering
  tft.setTextFont(4);

  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(mcu_decoded);

  lastFpsTime = millis(); // Initialize FPS timer
  
  // --- WiFi & ESP-NOW Initialization ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Disconnect from any saved WiFi networks
  // Disable WiFi power saving to reduce latency and packet loss
  esp_wifi_set_ps(WIFI_PS_NONE);

  // Force channel 1 for direct connection
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  
  tft.println("MAC Address:");
  tft.println(WiFi.macAddress());
  Serial.print("Receiver MAC: "); Serial.println(WiFi.macAddress());
  
  tft.println("> Mode: ESP-NOW");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    tft.println("ESP-NOW Init FAILED");
    return;
  }
  
  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);

  // Add broadcast peer for initial request
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Send initial request for a frame
  uint8_t cmd = 0x02;
  esp_now_send(broadcastAddress, &cmd, 1);
}

// ============================================================================
//  5. LOOP
// ============================================================================
void loop() {
  // Pin the JPEG decoding task to Core 0 for smoother video rendering on Core 1.
  if (Task1 == NULL) {
    xTaskCreatePinnedToCore(decodeJpg, "decodeJpg", 10000, NULL, 2, &Task1, 0);
  }

  // --- Frame Processing ---
  if (esp_now_frame_ready) {
    frameCount++; // A new frame has been fully received.

    if (isFirstFrame) {
      tft.fillScreen(TFT_BLACK);
      isFirstFrame = false;
    }
    drawStats(); // Update RSSI and FPS on screen

    // --- Trigger JPEG Decoding on Core 0 ---
    arrayName = (const uint8_t*)jpg_buffer;
    arrayLength = jpg_buffer_len;
    mcuReady = false; 
    doDecoding = true; // Signal Core 0 to start

    // While Core 0 decodes, Core 1 (this loop) renders the decoded blocks.
    while(doDecoding || mcuReady) {
      if (mcuReady) {
        // Draw image with offset to prevent cropping
        tft.pushImage(mcu_x + 15, mcu_y, mcu_w, mcu_h, mcuBuffer);
        mcuReady = false;
      }
      yield(); // Allow other tasks to run
    }
    
    // Reset for the next frame
    jpg_buffer_len = 0;
    esp_now_frame_ready = false;
    
    // Request the next frame
    uint8_t cmd = 0x02;
    if (senderMacKnown) esp_now_send(senderMac, &cmd, 1);
    else esp_now_send(broadcastAddress, &cmd, 1);

  } else if (millis() - last_packet_time > 200) {
    // TIMEOUT: If no data received for 200ms, the request was likely lost.
    // We reset and retry.
    
    if (jpg_buffer_len > 0) {
      Serial.println("ESP-NOW frame timeout. Discarding.");
      jpg_buffer_len = 0;
      frameCorrupt = false; 
    }

    // Retry request
    uint8_t cmd = 0x02;
    if (senderMacKnown) esp_now_send(senderMac, &cmd, 1);
    else esp_now_send(broadcastAddress, &cmd, 1);
    
    last_packet_time = millis(); // Reset timer to prevent flooding
  }

  // --- Button Handling ---
  static unsigned long lastBtnTime = 0;
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (millis() - lastBtnTime > 500) { // Debounce 500ms
      if (senderMacKnown) {
        uint8_t cmd = 0x01; // Command to switch resolution
        esp_now_send(senderMac, &cmd, 1);
        Serial.println("Sent switch resolution command");
        
        currentRes++;
        if (currentRes > 2) currentRes = 0;
      }
      lastBtnTime = millis();
    }
  }
}

// ============================================================================
//  6. HELPER & DUAL-CORE FUNCTIONS
// ============================================================================

/**
 * @brief Callback from TJpg_Decoder, called on Core 0 for each decoded block.
 */
bool mcu_decoded(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
   if (y >= tft.height()) return 0;

   // Wait if the previous block hasn't been rendered by Core 1 yet.
   while(mcuReady) vTaskDelay(1); // Use vTaskDelay to feed WDT

   // Copy the decoded block data to a shared buffer.
   memcpy(mcuBuffer, bitmap, w * h * 2);
   mcu_x = x;
   mcu_y = y;
   mcu_w = w;
   mcu_h = h;
   mcuReady = true; // Signal Core 1 that a block is ready to be rendered.

   return 1; // Continue decoding
}

/**
 * @brief Task running on Core 0, dedicated to JPEG decoding.
 */
void decodeJpg(void* pvParameter) {
  for(;;) {
    if (doDecoding) { 
      TJpgDec.drawJpg(0, 7, arrayName, arrayLength); 
      doDecoding = false; // Decoding is complete
    }
    vTaskDelay(1); // Yield to IDLE task to feed WDT
  }
}

/**
 * @brief Draws RSSI and FPS stats on the screen, updated once per second.
 */
void drawStats() {
  static unsigned long lastStatTime = 0;
  if (millis() - lastStatTime > 1000) {
    // --- Resolution ---
    tft.setTextFont(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    
    // Draw Resolution on the right side
    tft.fillRect(175, 15, 65, 40, TFT_BLACK);
    tft.setCursor(180, 15);
    tft.print("RES");
    tft.setCursor(180, 33);
    
    String resName = "QQVGA";
    if (currentRes == 1) resName = "QVGA";
    else if (currentRes == 2) resName = "HQVGA";
    tft.print(resName);

    // --- FPS ---
    unsigned long currentTime = millis();
    float fps = (float)frameCount / ((currentTime - lastFpsTime) / 1000.0);
    frameCount = 0;
    lastFpsTime = currentTime;
    
    // Draw FPS on the right side
    tft.fillRect(175, 70, 65, 40, TFT_BLACK);
    tft.setCursor(180, 70);
    tft.print("FPS");
    tft.setCursor(180, 88);
    tft.print(String(fps, 1));

    lastStatTime = millis();
  }
}