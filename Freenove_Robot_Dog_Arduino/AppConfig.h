#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- System Identity ---
#define ROBOT_NAME      "FREENOVE-DOG"
#define SW_VERSION      "100"
#define INTERNAL_CODE   "FNK006201"

// --- Robot Dimensions (mm) ---
#define ROBOT_L1        23.0  // [mm] Root length (Shoulder to Thigh pivot)
#define ROBOT_L2        55.0  // [mm] Thigh length (Upper Leg)
#define ROBOT_L3        59.0  // [mm] Calf length (Lower Leg)
#define ROBOT_DR        100.0 // [mm] Active radius limit (Max leg extension)
#define ROBOT_LEN_BD    68.2  // [mm] Body Length / 2 (Center to Shoulder X-offset)
#define ROBOT_WID_BD    40.0  // [mm] Body Width / 2 (Center to Shoulder Z-offset)

// --- Pin Definitions ---
#define PIN_SDA         13
#define PIN_SCL         14
#define PIN_BUZZER      33
#define PIN_BATT        32
#define PIN_TRIG        32    // Shared with Battery (Mutex required)
#define PIN_ECHO        12
#define PIN_RGBLED      0
#define PIN_TOUCH_PAD   TOUCH_PAD_NUM3 // GPIO 15
#define PIN_LED_BUILTIN 2

// --- Battery Settings (mV) ---
#define BATT_MAX_VOLTAGE    8400 // [mV] Fully charged (2S LiPo)
#define BATT_MIN_VOLTAGE    6000 // [mV] Empty (3.0V per cell)
#define BATT_LOW_WARNING    5900 // [mV] Trigger low battery alarm
#define BATT_CRITICAL       3000 // [mV] Emergency shutdown threshold

#endif