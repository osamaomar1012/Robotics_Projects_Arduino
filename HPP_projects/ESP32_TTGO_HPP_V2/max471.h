/**
 * MODULE: MAX471 Voltage and Current Sensor (Calibrated with esp_adc_cal)
 * DESCRIPTION: This module provides functions to read and convert analog signals
 *              from the MAX471 sensor using characterized ESP32 ADC curves.
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef MAX471_H
#define MAX471_H

#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

// Pins configuration for MAX471 sensor
#define MAX471_VT_PIN 36  // ESP32 GPIO connected to MAX471 VT (Voltage Output) - ADC1_CHANNEL_0
#define MAX471_AT_PIN 32  // ESP32 GPIO connected to MAX471 AT (Current Output) - ADC1_CHANNEL_4

// EMA Filter coefficient (0.0 to 1.0). 
#define FILTER_ALPHA 0.1

// Extern of the calibration characteristics structure
extern esp_adc_cal_characteristics_t adc_chars;

/**
 * @brief Initialize the ADC configuration and characterize the ADC calibration curve.
 */
inline void setupADCCalibration() {
    // Characterize ADC1 at 11dB attenuation, 12-bit width, using default Vref = 1100mV
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    
    // Configure ADC1 width
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    // Configure ADC1 channels with 11dB attenuation (up to ~3.1V input)
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // GPIO 36 (VT)
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); // GPIO 32 (AT)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO 34 (Internal battery)
}

/**
 * @brief Reads the MAX471 voltage and current sensor outputs using characterized calibration.
 * 
 * @param voltage A float reference to store the calculated battery voltage.
 * @param current A float reference to store the calculated battery current.
 */
inline void readMAX471(float &voltage, float &current) {
    // Perform characterized millivolt readings from ADC1
    uint32_t mvVT = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc_chars);
    uint32_t mvAT = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_4), &adc_chars);
    
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
        voltage = (instantV * FILTER_ALPHA) + (voltage * (1.0 - FILTER_ALPHA));
        current = (instantI * FILTER_ALPHA) + (current * (1.0 - FILTER_ALPHA));
    }
}

#endif // MAX471_H