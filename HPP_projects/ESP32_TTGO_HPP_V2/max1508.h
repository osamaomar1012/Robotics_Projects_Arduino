/**
 * MODULE: MAX1508 (MX1508) Motor Driver Control
 * DESCRIPTION: This module provides functions to initialize and control a DC motor
 *              using the MAX1508 / MX1508 H-bridge driver. It maps 0..100% speed to
 *              0..255 PWM duty cycle on the ESP32's LEDC peripheral.
 * DEPENDENCIES: Arduino.h
 * Created by Eng.Osama Omar
 * Senior Integration Lead
 * Cairo,Egypt
 */
#ifndef MAX1508_H
#define MAX1508_H

#include <Arduino.h>

#define MOTOR_IN1     13 // ESP32 GPIO 13 connected to MX1508 INA1 (Carrier Pad 20)
#define MOTOR_IN2     25 // ESP32 GPIO 25 connected to MX1508 INB1 (Carrier Pad 5)

#define PWM_FREQ      5000 // PWM frequency in Hz
#define PWM_CHAN_A    0    // Channel for Forward (IN1)
#define PWM_CHAN_B    1    // Channel for Reverse (IN2)
#define PWM_RES       8    // PWM resolution (8 bits = 0-255 duty cycle)

void setupMAX1508() {
    ledcSetup(PWM_CHAN_A, PWM_FREQ, PWM_RES);
    ledcSetup(PWM_CHAN_B, PWM_FREQ, PWM_RES);

    ledcAttachPin(MOTOR_IN1, PWM_CHAN_A);
    ledcAttachPin(MOTOR_IN2, PWM_CHAN_B);

    ledcWrite(PWM_CHAN_A, 0);
    ledcWrite(PWM_CHAN_B, 0);
}

/**
 * @brief Sets the motor speed by mapping -100..100% to 0..255 PWM duty cycle.
 * @param speed Signed value (-100 to 100). Positive = Fwd, Negative = Rev.
 */
void setMotorSpeed(int16_t speed) {
    int16_t duty = (abs(speed) * 255) / 100;
    if (duty > 255) duty = 255;

    if (speed >= 0) {
        ledcWrite(PWM_CHAN_A, duty);
        ledcWrite(PWM_CHAN_B, 0);
    } else {
        ledcWrite(PWM_CHAN_A, 0);
        ledcWrite(PWM_CHAN_B, duty);
    }
}

#endif
