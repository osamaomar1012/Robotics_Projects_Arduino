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
    Wire.setTimeOut(30); // 30ms non-blocking timeout
    
    // Probe I2C bus at address 0x40
    Wire.beginTransmission(0x40);
    byte error = Wire.endTransmission();
    if (error == 0) {
        Serial.printf("[I2C] INA219 found at address 0x40 (SDA=%d, SCL=%d)\n", I2C_SDA_PIN, I2C_SCL_PIN);
    } else {
        Serial.printf("[I2C WARNING] No response from 0x40 (err=%d). Check SDA=%d, SCL=%d\n", error, I2C_SDA_PIN, I2C_SCL_PIN);
    }

    // Initialize the INA219
    if (!ina219.begin()) {
        Serial.println("Failed to find INA219 chip. Using fallback telemetry.");
    } else {
        Serial.println("INA219 Initialized successfully.");
        ina219.setCalibration_32V_2A();
    }
}

/**
 * @brief Reads the INA219 voltage and current sensor outputs.
 * 
 * @param voltage A float reference to store the calculated battery voltage.
 * @param current A float reference to store the calculated battery current.
 */
inline void readINA219(float &voltage, float &current) {
    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    
    // Guard against NaN / I2C bus read dropouts
    if (isnan(shuntvoltage) || isnan(busvoltage) || isnan(current_mA)) {
        return;
    }

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