/**
 * MODULE: INA219 Voltage and Current Sensor
 * DESCRIPTION: This module provides functions to initialize and read from
 *              the INA219 I2C high-precision voltage and current sensor.
 * DEPENDENCIES: Adafruit_INA219 library
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef INA219_H
#define INA219_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

// I2C Pins configuration for INA219
#define I2C_SDA_PIN 43
#define I2C_SCL_PIN 44

Adafruit_INA219 ina219;

// EMA Filter coefficient (0.0 to 1.0)
#define FILTER_ALPHA 0.1

/**
 * @brief Initialize the INA219 sensor on the specified I2C pins.
 */
inline void setupINA219() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    // Initialize the INA219. By default, this uses the maximum range (32V, 2A).
    if (!ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
        // We will not halt here, but we'll print an error.
    } else {
        Serial.println("INA219 Initialized successfully.");
        // Optional: To increase precision for lower current, you can set calibration:
        // ina219.setCalibration_16V_400mA();
    }
}

/**
 * @brief Reads the INA219 voltage and current sensor outputs.
 * 
 * @param voltage A float reference to store the calculated battery voltage.
 * @param current A float reference to store the calculated battery current.
 */
inline void readINA219(float &voltage, float &current) {
    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    
    // Read raw values from the INA219
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    
    // Total voltage is bus voltage + shunt voltage
    float instantV = busvoltage + (shuntvoltage / 1000.0f);
    float instantI = current_mA / 1000.0f; // Convert mA to A

    // Protect against noise flipping sign around zero
    if (instantI < 0) instantI = 0;

    // Apply Exponential Moving Average (EMA) filter
    if (voltage < 0.1) {
        voltage = instantV;
        current = instantI;
    } else {
        voltage = (instantV * FILTER_ALPHA) + (voltage * (1.0 - FILTER_ALPHA));
        current = (instantI * FILTER_ALPHA) + (current * (1.0 - FILTER_ALPHA));
    }
}

#endif // INA219_H