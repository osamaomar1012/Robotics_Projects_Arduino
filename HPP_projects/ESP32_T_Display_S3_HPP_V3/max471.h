/**
 * MODULE: MAX471 Voltage and Current Sensor (Calibrated with analogReadMilliVolts)
 * DESCRIPTION: This module provides functions to read and convert analog signals
 *              from the MAX471 sensor using the ESP32 characterized analogReadMilliVolts API.
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef MAX471_H
#define MAX471_H

#include <Arduino.h>

// Pins configuration for LILYGO T-Display-S3 header pins
#define MAX471_VT_PIN 1  // ESP32-S3 GPIO 1 connected to MAX471 VT (Voltage Output)
#define MAX471_AT_PIN 2  // ESP32-S3 GPIO 2 connected to MAX471 AT (Current Output)

// EMA Filter coefficient (0.0 to 1.0)
#define FILTER_ALPHA 0.1

/**
 * @brief Initialize the ADC configuration for characterized reads.
 */
inline void setupADCCalibration() {
    // Configure pins as input
    pinMode(MAX471_VT_PIN, INPUT);
    pinMode(MAX471_AT_PIN, INPUT);
}

/**
 * @brief Reads the MAX471 voltage and current sensor outputs using characterized calibration.
 * 
 * @param voltage A float reference to store the calculated battery voltage.
 * @param current A float reference to store the calculated battery current.
 */
inline void readMAX471(float &voltage, float &current) {
    // Perform highly accurate characterized millivolt readings directly from the ESP32 API
    uint32_t mvVT = analogReadMilliVolts(MAX471_VT_PIN);
    uint32_t mvAT = analogReadMilliVolts(MAX471_AT_PIN);
    
    // Convert to real-world units
    // 1. Voltage VT output has a 1:5 ratio divider (1V output VT = 5V input)
    float instantV = (mvVT / 1000.0f) * 5.0f;
    
    // 2. Current AT outputs 1V per 1A (1A = 1000mV across the 1k load resistor)
    float instantI = mvAT / 1000.0f;

    // Apply Exponential Moving Average (EMA) filter
    if (voltage < 0.1) {
        voltage = instantV;
        current = instantI;
    } else {
        voltage = (instantV * FILTER_ALPHA) + (voltage * (1.0f - FILTER_ALPHA));
        current = (instantI * FILTER_ALPHA) + (current * (1.0f - FILTER_ALPHA));
    }
}

#endif // MAX471_H