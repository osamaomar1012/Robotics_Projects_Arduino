# Freenove Robot Dog Firmware - Arduino Edition

## Overview
This project is the firmware for the Freenove Robot Dog Kit based on the ESP32 microcontroller. It is rewritten to be fully compatible with the Arduino IDE, organizing the code into logical modules (Drivers, Motion, Communication) and utilizing FreeRTOS for multitasking.

## File Structure
- **Freenove_Robot_Dog_Arduino.ino**: The main entry point containing `setup()` and `loop()`.
- **Robot_Global_Definitions.h**: Global constants, pin definitions, and data structures.
- **Robot_Hardware_Drivers.h / Robot_Drivers.ino**: Low-level hardware drivers (Servo, Battery, LED, etc.).
- **Robot_Motion_Control.h / Robot_Motion.ino**: Kinematics, gait algorithms, and dance routines.
- **Robot_Wireless_Communication.h / Robot_Comms.ino**: WiFi, Camera, BLE, and command parsing.

## Recent Software Changes & Fixes

### 1. ESP32 Core 2.0.17 Compatibility
- **BLE String Conversion**: Fixed `std::string` to `String` conversion errors in `Robot_Comms.ino` caused by library updates.
- **LEDC API**: Reverted `ledcAttach` (v3.0 API) to `ledcSetup` and `ledcAttachPin` (v2.x API) in `Robot_Drivers.ino` to ensure compatibility with the current board definition.
- **Camera Pins**: Updated deprecated camera pin definitions (`pin_sscb_sda` -> `pin_sccb_sda`).

### 2. Library Updates
- **WS2812 LEDs**: Switched from the incompatible `Freenove_WS2812_Lib_for_ESP32` to the standard **Adafruit NeoPixel** library. This resolves RMT driver compilation errors.

### 3. Stability & Performance Enhancements
- **Inverse Kinematics**: Added numerical clamping to `asin` and `acos` functions in `Robot_Motion.ino`. This prevents `NaN` (Not a Number) crashes when the robot attempts to reach coordinates slightly outside its physical limits.
- **I2C Speed**: Increased I2C clock from 100kHz to **400kHz** (Fast Mode). This significantly reduces servo update latency, resulting in smoother leg movement.
- **Mutex Standardization**: Replaced mixed `pthread_mutex` usage with native FreeRTOS `SemaphoreHandle_t` for better thread safety between the Battery monitor and Ultrasonic sensor (which share a pin).
- **Touch ISR**: Corrected the interrupt service routine signature to match the ESP32 driver requirements.

---

## Software Sequence & Flow

### 1. Initialization (`setup()`)
When the robot powers on, the `setup()` function in the main sketch executes:
1.  **Serial Communication**: Starts at 115200 baud for debugging.
2.  **Drivers Initialization**: Calls `begin()` for NVS (storage), PCA9685 (servos), Battery, Buzzer, Sonar, LEDs, and Touch.
3.  **Configuration Loading**: Reads saved servo calibration offsets and LED modes from NVS.
4.  **Communication Startup**: Initializes BLE and the Camera/WiFi module.
5.  **Task Creation**: Spawns FreeRTOS tasks:
    *   `TASK_MOTION_SERVICE`: Handles complex movement calculations on Core 1.
    *   `TASK_SECONDARY`: Handles background sensors (Battery, Sonar, Touch) on Core 1.
6.  **Startup Action**: Plays a power-up melody and moves the robot to the standing position.

### 2. Main Loop (`loop()`)
The `loop()` function runs repeatedly on Core 1:
*   **Command Processing**: Checks for incoming non-motion commands (like LED color changes).
*   **BLE Upload**: Sends status updates to the connected app via Bluetooth.
*   **Built-in LED**: Blinks the onboard LED to indicate activity.
*   **Serial Input**: Reads commands from the USB serial monitor for debugging.

### 3. Background Tasks (`loopSecondary`)
Running in parallel to the main loop:
*   **Battery Monitor**: Checks voltage every second.
*   **LED Effects**: Updates RGB LED patterns (Rainbow, Breathing).
*   **Auto-Walking**: If enabled, reads the ultrasonic sensor to avoid obstacles.
*   **Touch**: Detects capacitive touch input to trigger interactions.

---

## Function API Description

### Main Sketch
*   `void setup()`: Initializes the system.
*   `void loop()`: Main execution cycle.
*   `void enterMessageQueue(String msg)`: Parses the first character of a command string and routes it to either the Motion Queue (high priority) or Info Queue.

### Drivers Module (`Drivers::`)
*   **PCA9685**
    *   `begin()`: Initializes I2C and sets PWM frequency to 50Hz.
    *   `setServoAngle(int chn, int angle)`: Moves specific servo (0-15) to an angle (0-180).
    *   `releaseAll()`: Disables PWM to all servos (relax mode).
*   **Buzzer**
    *   `play(int melodyId)`: Queues a predefined melody (e.g., `MELODY_POWER_UP`).
*   **Battery**
    *   `getVoltage()`: Returns battery voltage in millivolts.
*   **Sonar**
    *   `getDistance()`: Returns distance in cm using the ultrasonic sensor.
*   **LEDs**
    *   `setRGB(mode, r, g, b)`: Sets the color and pattern mode (Solid, Breathing, Rainbow).

### Motion Module (`Motion::`)
*   **Kinematics**
    *   `calcAngles(leg, x, y, z, &a, &b, &c)`: **Inverse Kinematics**. Calculates the three servo angles (alpha, beta, gamma) required to place a leg's foot at coordinates (x, y, z).
*   **Movement**
    *   `standUp()`: Moves all legs to the calibrated neutral position.
    *   `moveTo(target[][3], speed)`: Interpolates all legs from current positions to target positions smoothly.
    *   `move(x, y, z, speed)`: Executes a walking gait vector.
    *   `twist(x, y, z)`: Rotates the body body while feet stay planted.
*   **Calibration**
    *   `loadCalibration()`: Loads servo offsets from permanent storage to correct hardware misalignments.

### Communication Module (`Comms::`)
*   **Parser**
    *   `parse(String msg)`: Splits a command string (e.g., "C#1#255#0#0#") into a command char ('C') and parameters.
*   **Camera**
    *   `begin()`: Initializes the ESP32-CAM module and starts the WiFi Access Point.

---

## Guide for Users

### For Beginner Arduino Users
1.  **Installation**: Install the `Freenove_WS2812_Lib_for_ESP32` library via the Library Manager.
2.  **Board Selection**: Select "ESP32 Dev Module" (or similar) in Arduino IDE.
3.  **Upload**: Connect the ESP32 via USB and click Upload.
4.  **Simple Control**: Open the Serial Monitor (115200 baud). Type `G#80#` and press Enter to change the robot's height, or `D#1#` to make it say hello.
5.  **Modifying**: Look at `setup()` in the main file. You can comment out `standUp()` if you want to test without movement.

### For Advanced Users
1.  **FreeRTOS**: The system uses `xTaskCreateUniversal` to pin tasks to specific cores. `loopSecondary` handles blocking sensor reads without freezing the motion engine.
2.  **Inverse Kinematics**: The math is located in `Robot_Motion.ino` -> `calcAngles`. It uses trigonometric functions to resolve the 3-DOF leg linkage.
3.  **Data Queues**: Thread-safe queues (`DataQueue` template in `Global.h`) are used to pass string commands between the communication task (Core 0/1) and the motion task (Core 1).
4.  **Custom Gaits**: To create a new gait, implement a function in `Robot_Motion.ino` that generates a sequence of coordinate points and calls `moveTo`.

## Hardware Pinout Reference

### ESP32-WROVER Pin Mapping

| Component | Pin Name | GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **I2C Bus (Servos)** | SDA | 13 | PCA9685 Driver |
| | SCL | 14 | PCA9685 Driver |
| **Camera (OV2640)** | D0 | 4 | |
| | D1 | 5 | |
| | D2 | 18 | |
| | D3 | 19 | |
| | D4 | 36 | |
| | D5 | 39 | |
| | D6 | 34 | |
| | D7 | 35 | |
| | XCLK | 21 | 20MHz Clock |
| | PCLK | 22 | |
| | VSYNC | 25 | |
| | HREF | 23 | |
| | SDA | 26 | SCCB (I2C) |
| | SCL | 27 | SCCB (I2C) |
| **Sensors & IO** | Buzzer | 33 | |
| | Battery ADC | 32 | Shared with Sonar Trig |
| | Sonar Trig | 32 | Shared with Battery |
| | Sonar Echo | 12 | |
| | Touch Pad | 15 | TOUCH_PAD_NUM3 |
| | RGB LED | 0 | WS2812 Data |
| | Built-in LED | 2 | Blue LED |