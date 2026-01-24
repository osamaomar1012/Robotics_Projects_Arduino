#ifndef ARD_ROBOT_DRIVERS_H
#define ARD_ROBOT_DRIVERS_H

#include "ard_Robot_Global.h"
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include "driver/ledc.h"
#include "driver/touch_pad.h"
#include <nvs_flash.h>

namespace Drivers {

    // --- NVS (Preferences) ---
    namespace NVS {
        extern Preferences prefs;
        void begin();
        void clearAll();
    }

    // --- PCA9685 Servo Driver ---
    class PCA9685 {
    public:
        static void begin();
        static void setPWM(int chn, int on, int off);
        static void setServoAngle(int chn, float angle);
        static void releaseServo(int chn);
        static void releaseAll();
    private:
        static int writeReg(uint8_t reg, uint8_t value);
        static uint8_t readReg(uint8_t reg);
    };

    // --- Buzzer ---
    namespace Buzzer {
        void begin();
        void setFreq(uint16_t freq);
        void play(int melodyId);
        void taskService(void *pvParameters); // Background task
    }

    // --- Battery Monitor ---
    namespace Battery {
        void begin();
        uint32_t getVoltage();
        void taskMonitor(void *pvParameters);
        extern uint32_t voltage;
        extern uint8_t percent;
    }

    // --- Ultrasonic Sensor ---
    namespace Sonar {
        void begin();
        float getDistance();
        void setAutoWalk(bool enable);
        void taskAutoWalk(void *pvParameters);
    }

    // --- Touch Pad ---
    namespace Touch {
        void begin();
        void taskMonitor(void *pvParameters);
    }

    // --- LEDs (WS2812 & Built-in) ---
    namespace LEDs {
        void begin();
        void setRGB(uint8_t mode, uint8_t r, uint8_t g, uint8_t b);
        void loadConfig();
        void saveConfig();
        void taskRGB(void *pvParameters);
        void taskBuiltIn(void *pvParameters);
    }
}

#endif