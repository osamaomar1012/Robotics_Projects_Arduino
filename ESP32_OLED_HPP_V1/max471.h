/**
 * MODULE: MAX471 Voltage and Current Sensor
 * DESCRIPTION: This module provides functions to read and convert analog signals
 *              from the MAX471 sensor into real-world voltage and current values.
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef MAX471_H
#define MAX471_H

#include <Arduino.h>

// Pins configuration for MAX471 sensor
#define MAX471_VT_PIN 34  // ESP32 GPIO connected to MAX471 VT (Voltage Output)
#define MAX471_AT_PIN 32  // ESP32 GPIO connected to MAX471 AT (Current Output)

// EMA Filter coefficient (0.0 to 1.0). 
// Lower = smoother but slower response; Higher = faster but noisier.
#define FILTER_ALPHA 0.2 // Increased for better transient detection of penetration spikes

/**
 * @brief Reads the MAX471 voltage and current sensor outputs.
 * 
 * This function performs analog reads on the specified ESP32 GPIO pins
 * connected to the MAX471's VT and AT outputs. It then converts the raw
 * ADC values into actual voltage (Volts) and current (Amperes) based on
 * the sensor's characteristics and the ESP32's ADC properties.
 * 
 * @param voltage A float reference to store the calculated battery voltage.
 * @param current A float reference to store the calculated battery current.
 */
void readMAX471(float &voltage, float &current) {
    uint16_t rawVT = analogRead(MAX471_VT_PIN); // Read raw ADC value from Voltage output
    uint16_t rawAT = analogRead(MAX471_AT_PIN); // Read raw ADC value from Current output
    
    // Calculate instantaneous values
    float instantV = (rawVT / 4095.0) * 3.3 * 5.0;
    float instantI = (rawAT / 4095.0) * 3.3;

    // Apply Exponential Moving Average (EMA) filter
    // If current value is 0 (initialization), seed the filter with raw values
    if (voltage < 0.1) {
        voltage = instantV;
        current = instantI;
    } else {
        // New_Value = (Instant * Alpha) + (Previous * (1 - Alpha))
        voltage = (instantV * FILTER_ALPHA) + (voltage * (1.0 - FILTER_ALPHA));
        current = (instantI * FILTER_ALPHA) + (current * (1.0 - FILTER_ALPHA));
    }
}

#endif // MAX471_H