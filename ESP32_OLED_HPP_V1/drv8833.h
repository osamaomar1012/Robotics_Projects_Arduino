/**
 * MODULE: DRV8833 Motor Driver Control
 * DESCRIPTION: This module provides functions to initialize and control a DC motor
 *              using the DRV8833 H-bridge driver. It leverages the ESP32's LEDC
 *              (LED Controller) peripheral for PWM generation to control motor speed.
 * DEPENDENCIES: Arduino.h
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef DRV8833_H
#define DRV8833_H

#include <Arduino.h>

// Pins configuration for DRV8833
#define MOTOR_IN1     13 // ESP32 GPIO connected to DRV8833 IN1 (PWM for speed)
#define MOTOR_IN2     12 // ESP32 GPIO connected to DRV8833 IN2 (Digital for direction)

// PWM Settings for ESP32's LEDC peripheral
#define PWM_FREQ      5000 // PWM frequency in Hz (e.g., 5000 Hz for DC motors)
#define PWM_CHAN_A    0    // Channel for Forward (IN1)
#define PWM_CHAN_B    1    // Channel for Reverse (IN2)
#define PWM_RES       8    // PWM resolution in bits (e.g., 8 bits = 0-255 duty cycle)

/**
 * @brief Initializes the DRV8833 motor driver control pins.
 */
void setupDRV8833() {
    // Configure LEDC for both pins to allow bidirectional control
    ledcSetup(PWM_CHAN_A, PWM_FREQ, PWM_RES);
    ledcSetup(PWM_CHAN_B, PWM_FREQ, PWM_RES);

    ledcAttachPin(MOTOR_IN1, PWM_CHAN_A);
    ledcAttachPin(MOTOR_IN2, PWM_CHAN_B);

    // Initialize both pins to LOW (Stop)
    ledcWrite(PWM_CHAN_A, 0);
    ledcWrite(PWM_CHAN_B, 0);
}

/**
 * @brief Sets the motor speed by adjusting the PWM duty cycle.
 * @param speed Signed value (-255 to 255). Positive = Fwd, Negative = Rev.
 */
void setMotorSpeed(int16_t speed) {
    if (speed >= 0) {
        // Forward: IN1 gets PWM, IN2 is LOW
        ledcWrite(PWM_CHAN_A, speed);
        ledcWrite(PWM_CHAN_B, 0);
    } else {
        // Reverse: IN1 is LOW, IN2 gets PWM
        ledcWrite(PWM_CHAN_A, 0);
        ledcWrite(PWM_CHAN_B, -speed); // Use absolute value for duty cycle
    }
}

#endif