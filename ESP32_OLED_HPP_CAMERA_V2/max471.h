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
#include "BoardConfig.h"

#define MAX471_VT_PIN PIN_MAX471_VT
#define MAX471_AT_PIN PIN_MAX471_AT
#define MAX471_USE_RETRY false

// EMA Filter coefficient (0.0 to 1.0).
// Lower = smoother but slower response; Higher = faster but noisier.
#define FILTER_ALPHA 0.2 // Increased for better transient detection of penetration spikes

// Battery Chemistry Constants (18650 Li-ion)
#define BATT_MAX_VOLTAGE 4.20
#define BATT_MIN_VOLTAGE 3.20

extern volatile bool sensorError; // Global flag to indicate sensor reading issues

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
void readMAX471(volatile float &voltage, volatile float &current) {
    #if (CAMERA_ONLY_DEBUG == 1)
    // Completely bypass ADC reads to prevent WiFi/Camera DMA interference
    if (voltage < 0.1) voltage = 12.0; 
    if (current < 0.01) current = 0.0;
    sensorError = false; // No sensor error in debug mode
    return;
#endif

    int rawVT = -1;
    int rawAT = -1;

    rawVT = analogRead(MAX471_VT_PIN);
    rawAT = analogRead(MAX471_AT_PIN);

    // Differentiate between WiFi conflict (-1) and hardware failure (0)
    // Only flag as sensorError if rawVT is 0 (battery disconnected)
    // If it's -1, we simply skip the update to avoid "Sensor Unreliable" false alarms
    if (rawVT == 0) sensorError = true;
    else if (rawVT > 0) sensorError = false;

    if (rawVT < 0 || rawAT < 0) return; // Skip update if WiFi is locking ADC2
    
    // Use analogReadMilliVolts for factory-calibrated accuracy
    // This compensates for the individual Vref variances of your specific ESP32 chip.
    float instantV = (analogReadMilliVolts(MAX471_VT_PIN) / 1000.0) * 5.0;
    float instantI = (analogReadMilliVolts(MAX471_AT_PIN) / 1000.0);

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

/**
 * @brief Converts battery voltage to a 0-100 percentage.
 * 
 * Uses a linear mapping based on standard Li-ion discharge cutoffs.
 * Note: For better accuracy on ESP32, ensure you are using the 
 * internal calibration (analogReadMilliVolts) if supported.
 * 
 * @param v Current filtered voltage.
 * @return uint8_t Percentage (0-100).
 */
uint8_t calculateBatteryPercentage(float v) {
    if (v >= BATT_MAX_VOLTAGE) return 100;
    if (v <= BATT_MIN_VOLTAGE) return 0;
    
    float pct = (v - BATT_MIN_VOLTAGE) / (BATT_MAX_VOLTAGE - BATT_MIN_VOLTAGE) * 100.0;
    return (uint8_t)constrain(pct, 0, 100);
}

#endif // MAX471_H