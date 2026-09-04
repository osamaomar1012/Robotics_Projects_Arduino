/**
 * FILE: BoardConfig.h
 * DESCRIPTION: Central hardware selection for the HPP Project.
 *              Uncomment only one board definition.
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// ==========================================
// SELECT TARGET HARDWARE
// ==========================================
#include <Arduino.h>

/** @note Optimized exclusively for ESP32-WROVER-E with OV2640 & I2C OLED */
/** @note Optimized for ESP32-WROVER-DEV (Freenove/WROVER-KIT) with OV2640 & I2C OLED */
#define BOARD_ESP32_WROVER_E 
// Hardware Selection: ESP32-WROVER-E (PSRAM)
#define BOARD_HAS_PSRAM

// ==========================================
// DEBUG FLAG: Set to 1 to disable all motor/sensor logic 
// and isolate the Camera/WiFi performance.
// ==========================================
#define CAMERA_ONLY_DEBUG 0 
// WiFi Identity
#define WIFI_SSID_AP  "HPP_ADVANCE_SURGERY"
#define WIFI_PASS_AP  "12345678"

// ==========================================
// PIN MAPPINGS: SENSORS & ACTUATORS
// ==========================================
// MAX471 Sensor (ADC1 - WiFi Safe).
#define PIN_MAX471_VT         33
#define PIN_MAX471_AT         32 // Moved to 32 to free up 39 for the Camera D5 pin
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

// DRV8833 Motor Driver
#define PIN_MOTOR_IN1         2  // LEDC Channel 0 - Safe for boot
#define PIN_MOTOR_IN2         13 // LEDC Channel 1 - Safe for boot
// Reserved for Future Integration (Currently Inactive)
#define PIN_MAX471_VT   33
#define PIN_MAX471_AT   36
#define PIN_MOTOR_IN1    2
#define PIN_MOTOR_IN2   13
#define PIN_OLED_SDA    14
#define PIN_OLED_SCL    15

// I2C Bus (OLED)
#define PIN_I2C_SDA           14 // Standard WROVER header pin
#define PIN_I2C_SCL           15
#define PIN_OLED_RST          -1 // Set to -1 because the OLED has no hardware reset pin
// Streaming Constants
#define STREAM_PORT     80
#define FPS_INTERVAL    1000 // Update FPS counter every second

// OV2640 Camera (Standard Freenove/WROVER-E Pinout)
#define CAM_PIN_PWDN          -1 // Set to -1 to free GPIO 32 for the sensor
#define CAM_PIN_RESET         -1
#define CAM_PIN_XCLK          21 // WROVER_KIT uses 21
#define CAM_PIN_SIOD          26
#define CAM_PIN_SIOC          27
#define CAM_PIN_D7            35 // Restore to native Camera Pin
#define CAM_PIN_D6            34
#define CAM_PIN_D5            39 // Standard WROVER D5 pin restored
#define CAM_PIN_D4            36
#define CAM_PIN_D3            19
#define CAM_PIN_D2            18
#define CAM_PIN_D1             5
#define CAM_PIN_D0             4  // WROVER_KIT uses 4
#define CAM_PIN_VSYNC         25
#define CAM_PIN_HREF          23
#define CAM_PIN_PCLK          22
#define PIN_LAMP_FLASH        -1 // Set to -1 as no physical flash/lamp is connected

// ==========================================
// BUILD-TIME VALIDATION
// ==========================================
// Validation
#if !defined(ARDUINO_ARCH_ESP32)
  #error "This project requires an ESP32. Please select 'ESP32 Wrover Module' in Tools -> Board."
  #error "Select 'ESP32 Wrover Module' in Tools -> Board."
#endif

// ==========================================
// IDE & HARDWARE SYNC VALIDATION
// ==========================================
#if !defined(BOARD_HAS_PSRAM)
  #warning "PSRAM not detected. Ensure 'Tools -> PSRAM: Enabled' is set for high-quality video."
#endif

// ==========================================
// WIFI CREDENTIALS
// ==========================================
#define WIFI_SSID_STA ""
#define WIFI_PASS_STA ""
#define WIFI_SSID_AP  "ELBASEET HAIR_PEN V2.0"
#define WIFI_PASS_AP  "12345678"

// ==========================================
// SURGICAL & SAFETY PARAMETERS (DEFAULTS)
// ==========================================
#define DEFAULT_MAX_CURRENT   2.55  // Max current before safety trip (Amps)
#define DEFAULT_MIN_VOLTAGE   3.20  // Battery cut-off voltage (Volts)

#define DEFAULT_PEN_OFFSET    0.04  // Graft detection spike threshold (Amps)
#define DEFAULT_PEN_HYST      0.02  // Graft detection hysteresis (Amps)

// ==========================================
// MOTOR PWM CONFIGURATION
// ==========================================
#define DEFAULT_PWM_FREQ      5000  // PWM frequency in Hz (e.g., 5000 Hz for DC motors)
#define DEFAULT_PWM_RES       8     // PWM resolution in bits (e.g., 8 bits = 0-255 duty cycle)

// ==========================================

#endif // BOARD_CONFIG_H