#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <Arduino.h>
#include <esp_now.h>

// --- Board Selection ---
#define BOARD_WROVER    0
#define BOARD_ESP32_CAM 1

#define SELECTED_BOARD  BOARD_ESP32_CAM  // Set this to BOARD_ESP32_CAM for AI-Thinker module

// Hardware Selection: ESP32-WROVER-E (PSRAM)
#define BOARD_HAS_PSRAM

// Feature Toggles (Set to 0 to disable and free up resources/pins)
#define USE_SENSORS  0

// WiFi Identity
#define WIFI_SSID_AP  "HPP_ADVANCE_SURGERY"
#define WIFI_PASS_AP  "12345678"

#if (SELECTED_BOARD == BOARD_ESP32_CAM)
// Camera Pinout (AI-Thinker ESP32-CAM)
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// GPIO 4 is the on-board high-power Flash LED on ESP32-CAM.
// It is shared with the SD Card and Camera D0 in some configs. 
// Keep at -1 to prevent camera interference.
#define CAM_PIN_FLASH    4 

// Surgical Pen Control Pins (ESP32-CAM available headers)
// Note: These pins are shared with the SD Card Slot. 
// Ensure no SD card is inserted when using these pins.
#define PIN_MOTOR_IN1    2  
#define PIN_MOTOR_IN2   12
#define PIN_I2C_SDA     14
#define PIN_I2C_SCL     15

#else
// --- DEFAULT: ESP32-WROVER-E ---

// Camera Pinout (Standard WROVER-E Dev Board)
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    21
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      19
#define CAM_PIN_D2      18
#define CAM_PIN_D1       5
#define CAM_PIN_D0       4
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// GPIO 4 is used for Camera D0. On-board Flash LED is often also on GPIO 4.
// To avoid corruption, we disable software control of the Flash pin while camera is active.
#define CAM_PIN_FLASH   -1 

// Surgical Pen Control Pins
//#define PIN_MAX471_VT   34 // Unused: Using ADS1115 A0
//#define PIN_MAX471_AT   32 // Unused: Using ADS1115 A1
#define PIN_MOTOR_IN1    2  
#define PIN_MOTOR_IN2   13
#define PIN_I2C_SDA     14
#define PIN_I2C_SCL     15

#endif

// ADS1115 Channels (External ADC)
#define ADS_CH_VOLTAGE  0  // MAX471 VT connected to ADS1115 A0
#define ADS_CH_CURRENT  1  // MAX471 AT connected to ADS1115 A1

// Sensor Tuning
#define CURRENT_ALPHA   0.2 // Smoothing factor: 0.1 (very smooth/slow) to 1.0 (raw/fast)

// Streaming Constants
#define STREAM_PORT     80
#define FPS_INTERVAL    1000 // Update FPS counter every second

// ESP-NOW Broadcast Address
const uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

// Validation
#if !defined(ARDUINO_ARCH_ESP32)
  #error "Select 'ESP32 Wrover Module' in Tools -> Board."
#endif

#endif