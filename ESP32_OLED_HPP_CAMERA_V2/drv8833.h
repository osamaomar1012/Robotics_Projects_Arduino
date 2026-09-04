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
#include "BoardConfig.h"

#define MOTOR_IN1     PIN_MOTOR_IN1
#define MOTOR_IN2     PIN_MOTOR_IN2

// PWM Settings for ESP32's LEDC peripheral
#define PWM_FREQ      DEFAULT_PWM_FREQ // PWM frequency defined in BoardConfig.h
#define PWM_CHAN_A    0    // Channel for Forward (IN1)
#define PWM_CHAN_B    1    // Channel for Reverse (IN2)
#define PWM_RES       DEFAULT_PWM_RES  // PWM resolution defined in BoardConfig.h

/**
 * @brief Initializes the DRV8833 motor driver control pins.
 */
void setupDRV8833() {
    #if CAMERA_ONLY_DEBUG
    Serial.println(F("DEBUG: Motor Driver Disabled for Camera Isolation"));
    return;
    #endif

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
    #if CAMERA_ONLY_DEBUG
    return;
    #endif
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