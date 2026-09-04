#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <Arduino.h>
#include "LovyanGFX_Driver.h"
#include <Preferences.h>

// Constants
#define MAX_FRAME_SIZE 122880
#define BUZZER_PIN     8
#define SPEAKER_EN     21

// App State
enum AppMode { MODE_WELCOME, MODE_MENU, MODE_LIVE, MODE_CONFIG_SURGICAL, MODE_CONFIG_CAM, MODE_INFO, MODE_CALIBRATE };
extern AppMode currentMode;

// Display and Graphics
extern LGFX gfx;

// Network and Streaming
extern uint8_t* frameBuffer;
extern volatile size_t currentFramePos;
extern volatile bool frameReady;
extern volatile uint16_t lastSeq;
extern unsigned long lastFrameTime;
extern volatile int lastRSSI;
extern uint8_t surgicalPenMac[6];
extern const uint8_t broadcastAddress[6];

#endif