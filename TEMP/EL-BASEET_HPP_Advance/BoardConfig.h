#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <Arduino.h>
#include <esp_now.h>

// Hardware Selection: ESP32-WROVER-E (PSRAM)
#define BOARD_HAS_PSRAM

// Feature Toggles (Set to 0 to disable and free up resources/pins)
#define USE_OLED     0 
#define USE_SENSORS  1

// WiFi Identity
#define WIFI_SSID_AP  "HPP_ADVANCE_SURGERY"
#define WIFI_PASS_AP  "12345678"

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
#define CAM_PIN_FLASH    4 // On-board LED Flash pin

// Surgical Pen Control Pins
#define PIN_MAX471_VT   34
#define PIN_MAX471_AT   32
#define PIN_MOTOR_IN1    2
#define PIN_MOTOR_IN2   13
#define PIN_OLED_SDA    14
#define PIN_OLED_SCL    15

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