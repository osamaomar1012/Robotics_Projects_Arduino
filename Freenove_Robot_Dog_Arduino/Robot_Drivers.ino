#include "ard_Robot_Drivers.h"

// Shared Mutex for Pin 32 (Battery ADC vs Sonar Trigger conflict)
SemaphoreHandle_t mutexSharedPin = NULL; 

namespace Drivers {

    // ===================================================================================
    //  NVS
    // ===================================================================================
    namespace NVS {
        Preferences prefs;
        void begin() {
            prefs.begin("STORAGE");
        }
        void clearAll() {
            nvs_flash_erase();
            nvs_flash_init();
        }
    }

    // ===================================================================================
    //  PCA9685
    // ===================================================================================
    #define PCA9685_ADDR 0x40
    #define MODE1 0x00
    #define PRESCALE 0xFE
    #define LED0_ON_L 0x06
    
    /**
     * @brief Initializes the PCA9685 Servo Driver via I2C.
     * @details Sets the PWM frequency to 50Hz (standard for servos) and enables Auto-Increment for faster writes.
     */
    void PCA9685::begin() {
        Serial.println("PCA9685: Initializing...");
        Wire.begin(PIN_SDA, PIN_SCL);
        Wire.setClock(100000); // Revert to 100kHz for stability
        int err = writeReg(MODE1, 0x20); // Wake up & Auto-Increment (AI)
        if (err != 0) Serial.printf("PCA9685 Init Failed: %d\n", err);
        else Serial.println("PCA9685 Init Success");
        
        // Set Frequency to 50Hz
        uint8_t prescale = round(25000000.0 / 4096.0 / 50.0) - 1;
        uint8_t oldmode = readReg(MODE1);
        uint8_t newmode = (oldmode & 0x7F) | 0x10; // Sleep
        writeReg(MODE1, newmode);
        writeReg(PRESCALE, prescale);
        writeReg(MODE1, oldmode);
        delayMicroseconds(500);
        writeReg(MODE1, oldmode | 0x80 | 0x20); // Restart & Auto-Increment
        delay(10); // Wait for oscillator to stabilize
    }

    /**
     * @brief Sets the PWM pulse for a specific channel.
     * @param chn Channel number (0-15).
     * @param on Time to turn ON the pulse (0-4095).
     * @param off Time to turn OFF the pulse (0-4095).
     * @details The PCA9685 has 12-bit resolution (0-4095 steps per cycle).
     */
    void PCA9685::setPWM(int chn, int on, int off) {
        Wire.beginTransmission(PCA9685_ADDR);
        Wire.write(LED0_ON_L + 4 * chn);
        Wire.write(on & 0xFF);
        Wire.write(on >> 8);
        Wire.write(off & 0xFF);
        Wire.write(off >> 8);
        Wire.endTransmission();
    }

    /**
     * @brief Moves a servo to a specific angle.
     * @param chn Servo channel (0-15).
     * @param angle Target angle in degrees [0.0 - 180.0].
     * @details Maps the angle to a pulse width between 500us (0 deg) and 2500us (180 deg).
     */
    void PCA9685::setServoAngle(int chn, float angle) {
        angle = constrain(angle, 0.0f, 180.0f);
        // Map 0-180 to Pulse 500-2500us. 4096 counts = 20000us (50Hz)
        // val = micros * 4096 / 20000
        float micros = 500.0f + (angle / 180.0f) * 2000.0f;
        int off = (int)(micros * 4096.0f / 20000.0f);
        setPWM(chn, 0, off);
    }

    void PCA9685::releaseServo(int chn) {
        setPWM(chn, 0, 0);
    }

    void PCA9685::releaseAll() {
        for (int i = 0; i < 16; i++) releaseServo(i);
    }

    int PCA9685::writeReg(uint8_t reg, uint8_t value) {
        Wire.beginTransmission(PCA9685_ADDR);
        Wire.write(reg);
        Wire.write(value);
        return Wire.endTransmission();
    }

    uint8_t PCA9685::readReg(uint8_t reg) {
        Wire.beginTransmission(PCA9685_ADDR);
        Wire.write(reg);
        Wire.endTransmission();
        Wire.requestFrom(PCA9685_ADDR, 1);
        return Wire.read();
    }

    // ===================================================================================
    //  BUZZER
    // ===================================================================================
    namespace Buzzer {
        #define LEDC_CHAN 2
        #define LEDC_TIMER LEDC_TIMER_1
        #define LEDC_MODE LEDC_HIGH_SPEED_MODE

        // Melodies (Simplified for brevity, full arrays in original context)
        const int tunePowerUp[] = {525, 661, 990, 1248};
        const float beatPowerUp[] = {0.5, 0.5, 0.5, 1.0};

        const int tuneLowPower[] = {330, 262, 221};
        const float beatLowPower[] = {0.5, 0.5, 0.5};

        const int tuneNoPower[] = {330, 262, 441, 525};
        const float beatNoPower[] = {0.5, 0.5, 0.5, 1.0};

        const int tuneCamFailure[] = {165, 0, 165, 0, 165, 0, 165};
        const float beatCamFailure[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

        const int tuneBeep1[] = {525};
        const float beatBeep1[] = {1.0};

        const int tuneBeep2[] = {661, 0, 661, 0};
        const float beatBeep2[] = {0.5, 0.5, 0.5, 0.5};

        const int tuneWifiSucc[] = {1484, 1248, 1484};
        const float beatWifiSucc[] = {0.5, 0.5, 0.5};

        const int tuneWifiFailed[] = {990, 1248, 990};
        const float beatWifiFailed[] = {0.5, 0.5, 0.5};

        const int tuneWifiDis[] = {1248, 1484, 1248};
        const float beatWifiDis[] = {0.5, 0.5, 0.5};

        const int tuneBleSucc[] = {700, 0, 700, 0, 882, 0, 882, 0, 1049};
        const float beatBleSucc[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0};

        const int tuneBleDis[] = {1049, 0, 1049, 0, 882, 0, 882, 0, 700};
        const float beatBleDis[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0};

        const int tuneCamSucc[] = {990, 0, 990, 0, 1248, 0, 1484};
        const float beatCamSucc[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0};

        const int tuneCamDis[] = {1484, 0, 1484, 0, 1248, 0, 990};
        const float beatCamDis[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0};

        const int tuneBb[] = {2000};
        const float beatBb[] = {1.0};
        
        bool isPlaying = false;
        int currentMelody = -1;

        void begin() {
            ledcSetup(LEDC_CHAN, 5000, 10);
            ledcAttachPin(PIN_BUZZER, LEDC_CHAN);
        }

        void setFreq(uint16_t freq) {
            if (freq == 0) {
                ledcWrite(LEDC_CHAN, 0);
            } else {
                ledcWriteTone(LEDC_CHAN, freq);
            }
        }

        void play(int melodyId) {
            mqBz.enterForced(melodyId);
        }

        void playRoutine(const int* tune, const float* beat, int len, float speed) {
            for(int i=0; i<len; i++) {
                setFreq(tune[i]);
                vTaskDelay(beat[i] * speed / portTICK_PERIOD_MS);
                setFreq(0);
            }
        }

        /**
         * @brief Background task to handle buzzer melodies.
         * @details Checks the mqBz queue for new melody requests and plays them non-blocking.
         */
        void taskService(void *pvParameters) {
            if (!mqBz.isEmpty()) {
                int m = mqBz.out();
                switch(m) {
                    case MELODY_POWER_UP: 
                        Serial.println("[Buzzer] Playing MELODY_POWER_UP");
                        playRoutine(tunePowerUp, beatPowerUp, 4, 200); 
                        break;
                    case MELODY_LOW_POWER: 
                        Serial.println("[Buzzer] Playing MELODY_LOW_POWER");
                        playRoutine(tuneLowPower, beatLowPower, 3, 300); 
                        break;
                    case MELODY_NO_POWER:
                        Serial.println("[Buzzer] Playing MELODY_NO_POWER");
                        playRoutine(tuneNoPower, beatNoPower, 4, 500);
                        break;
                    case MELODY_CAM_FAILURE: 
                        Serial.println("[Buzzer] Playing MELODY_CAM_FAILURE");
                        playRoutine(tuneCamFailure, beatCamFailure, 7, 300); 
                        break;
                    case MELODY_BEEP_1:
                        Serial.println("[Buzzer] Playing MELODY_BEEP_1");
                        playRoutine(tuneBeep1, beatBeep1, 1, 300);
                        break;
                    case MELODY_BEEP_2:
                        Serial.println("[Buzzer] Playing MELODY_BEEP_2");
                        playRoutine(tuneBeep2, beatBeep2, 4, 100);
                        break;
                    case MELODY_WIFI_CONNECT_SUCCESS:
                        playRoutine(tuneWifiSucc, beatWifiSucc, 3, 300);
                        break;
                    case MELODY_WIFI_CONNECT_FAILED:
                        playRoutine(tuneWifiFailed, beatWifiFailed, 3, 300);
                        break;
                    case MELODY_WIFI_DISCONNECT:
                        playRoutine(tuneWifiDis, beatWifiDis, 3, 300);
                        break;
                    case MELODY_BLE_CONNECT_SUCCESS:
                        Serial.println("[Buzzer] BLE Connected");
                        playRoutine(tuneBleSucc, beatBleSucc, 9, 100);
                        break;
                    case MELODY_BLE_DISCONNECT:
                        Serial.println("[Buzzer] BLE Disconnected");
                        playRoutine(tuneBleDis, beatBleDis, 9, 100);
                        break;
                    case MELODY_CAM_CONNECT_SUCCESS:
                        playRoutine(tuneCamSucc, beatCamSucc, 7, 100);
                        break;
                    case MELODY_CAM_DISCONNECT:
                        playRoutine(tuneCamDis, beatCamDis, 7, 100);
                        break;
                    case MELODY_BB_CLEAR_1:
                        playRoutine(tuneBb, beatBb, 1, 100);
                        break;
                    case MELODY_BB_CLEAR_2:
                        playRoutine(tuneBb, beatBb, 1, 100);
                        vTaskDelay(100);
                        playRoutine(tuneBb, beatBb, 1, 100);
                        break;

                    // Add other cases here...
                    default: 
                        Serial.printf("[Buzzer] Playing Unknown Melody: %d\n", m);
                        setFreq(1000); vTaskDelay(100); setFreq(0); 
                        break;
                }
            }
        }
    }

    // ===================================================================================
    //  BATTERY
    // ===================================================================================
    namespace Battery {
        uint32_t voltage = 0;
        uint8_t percent = 0;

        // Configures the ADC for battery monitoring
        void begin() {
            // ESP32 Core v3.0+: Use standard Arduino Analog API
            analogReadResolution(12);
            analogSetAttenuation(ADC_11db);
            if (mutexSharedPin == NULL) mutexSharedPin = xSemaphoreCreateMutex();
        }

        /**
         * @brief Reads the battery voltage.
         * @return Voltage in millivolts [mV].
         * @details Uses a mutex because PIN_BATT (32) is shared with the Sonar Trigger pin.
         */
        uint32_t getVoltage() {
            xSemaphoreTake(mutexSharedPin, portMAX_DELAY);
            uint32_t mv = 0;
            // GPIO 32 is the battery pin
            for(int i=0; i<16; i++) mv += analogReadMilliVolts(PIN_BATT);
            mv /= 16;
            xSemaphoreGive(mutexSharedPin);
            return mv * 4; // Ratio 4.0 (Hardware Divider)
        }

        // Background task to monitor battery levels and alert if low
        void taskMonitor(void *pvParameters) {
            static uint32_t lastTime = 0;
            if (millis() - lastTime > 1000) {
                voltage = getVoltage();
                int p = map(voltage, BATT_MIN_VOLTAGE, BATT_MAX_VOLTAGE, 0, 100);
                percent = constrain(percent, 0, 100);
                
                // Upload to App
                String s = "I#" + String(voltage) + "#" + String(percent) + "#\n";
                mqTx.enterForced(s);
                
                // Serial.printf("Battery: %d mV (%d%%)\n", voltage, percent); // Disabled to reduce noise

                if (voltage < BATT_LOW_WARNING && voltage > BATT_CRITICAL) {
                    Serial.printf("[Battery] Low Voltage Detected: %d mV. Triggering Alarm.\n", voltage);
                    // Buzzer::play(MELODY_LOW_POWER); // Muted for debugging
                }
                lastTime = millis();
            }
        }
    }

    // ===================================================================================
    //  SONAR
    // ===================================================================================
    namespace Sonar {
        bool autoWalk = false;

        void begin() {
            if (mutexSharedPin == NULL) mutexSharedPin = xSemaphoreCreateMutex();
        }

        /**
         * @brief Measures distance using the HC-SR04 Ultrasonic Sensor.
         * @return Distance in centimeters [cm].
         * @details Sends a 10us trigger pulse and measures the echo duration.
         */
        float getDistance() {
            xSemaphoreTake(mutexSharedPin, portMAX_DELAY);
            pinMode(PIN_TRIG, OUTPUT);
            pinMode(PIN_ECHO, INPUT);
            digitalWrite(PIN_TRIG, LOW);
            delayMicroseconds(2);
            digitalWrite(PIN_TRIG, HIGH);
            delayMicroseconds(10);
            digitalWrite(PIN_TRIG, LOW);
            long duration = pulseIn(PIN_ECHO, HIGH, 30000); // Timeout 30ms
            pinMode(PIN_TRIG, INPUT); // Reset for ADC usage
            xSemaphoreGive(mutexSharedPin);
            return duration * 0.034 / 2;
        }

        void setAutoWalk(bool enable) { autoWalk = enable; }

        /**
         * @brief Autonomous obstacle avoidance task.
         * @details If enabled, checks distance and sends movement commands to avoid obstacles (<30cm).
         */
        void taskAutoWalk(void *pvParameters) {
            if (autoWalk) {
                float dist = getDistance();
                // Send status to App
                mqTx.enterForced(String(ACTION_ULTRASONIC) + "#" + String(dist) + "#\n");
                
                if (dist > 0 && dist < 30) {
                    // Obstacle: Move back/turn
                    enterMessageQueue(String(ACTION_MOVE_ANY) + "#180#10#0#5#"); // 180 deg direction
                } else {
                    // Clear: Move forward
                    enterMessageQueue(String(ACTION_MOVE_ANY) + "#0#20#0#20#"); // 0 deg direction
                }
            }
        }
    }

    // ===================================================================================
    //  LEDS
    // ===================================================================================
    namespace LEDs {
        Adafruit_NeoPixel strip(4, PIN_RGBLED, NEO_GRB + NEO_KHZ800);
        struct Config { uint8_t mode; uint8_t r; uint8_t g; uint8_t b; } cfg;

        void begin() {
            strip.begin();
            strip.setBrightness(255);
            pinMode(PIN_LED_BUILTIN, OUTPUT);
        }

        // Updates the configuration for the LED task
        void setRGB(uint8_t mode, uint8_t r, uint8_t g, uint8_t b) {
            cfg = {mode, r, g, b};
        }

        void loadConfig() {
            if (NVS::prefs.getBytesLength("LED") == sizeof(cfg)) {
                NVS::prefs.getBytes("LED", &cfg, sizeof(cfg));
            } else {
                cfg = {1, 0, 255, 0}; // Default Green
            }
        }

        void saveConfig() {
            NVS::prefs.putBytes("LED", &cfg, sizeof(cfg));
        }

        /**
         * @brief Handles RGB LED animations.
         * @details Implements state machines for Blink, Breathing, Rainbow, and Following modes.
         */
        void taskRGB(void *pvParameters) {
            static uint32_t lastStripUpdateTime = 0;
            static int followingColorStep = 0;
            static int breathingStep = 2;
            static bool breathingStateUp = true;
            static int rainbowStep = 2;
            static bool blinkStateUp = true;

            // LED_MODE_OFF 0, RGB 1, FOLLOWING 2, BLINK 3, BREATHING 4, RAINBOW 5
            
            switch (cfg.mode) {
                case 0: // OFF
                    if (millis() - lastStripUpdateTime > 500) {
                        strip.setBrightness(255);
                        for(int i=0; i<4; i++) strip.setPixelColor(i, 0, 0, 0);
                        strip.show();
                        lastStripUpdateTime = millis();
                    }
                    break;
                    
                case 1: // RGB (Solid)
                    if (millis() - lastStripUpdateTime > 50) {
                        strip.setBrightness(255);
                        for(int i=0; i<4; i++) strip.setPixelColor(i, cfg.r, cfg.g, cfg.b);
                        strip.show();
                        lastStripUpdateTime = millis();
                    }
                    break;

                case 2: // FOLLOWING
                    if (millis() - lastStripUpdateTime > 100) {
                        strip.setBrightness(255);
                        followingColorStep++;
                        int j = followingColorStep;
                        strip.clear();
                        // Cycle colors based on step
                        uint32_t color = strip.ColorHSV((j * 2000) & 65535);
                        strip.setPixelColor(j % 4, color);
                        strip.show();
                        lastStripUpdateTime = millis();
                    }
                    break;

                case 3: // BLINK
                    if (millis() - lastStripUpdateTime > 500) {
                        strip.setBrightness(255);
                        if (blinkStateUp) {
                            for(int i=0; i<4; i++) strip.setPixelColor(i, cfg.r, cfg.g, cfg.b);
                        } else {
                            for(int i=0; i<4; i++) strip.setPixelColor(i, 0, 0, 0);
                        }
                        strip.show();
                        blinkStateUp = !blinkStateUp;
                        lastStripUpdateTime = millis();
                    }
                    break;

                case 4: // BREATHING
                    if (millis() - lastStripUpdateTime > 10) {
                        // Step size increased to 10 for visibility at 50ms loop rate
                        if (breathingStateUp) {
                            breathingStep += 10;
                            if (breathingStep >= 255) {
                                breathingStep = 255;
                                breathingStateUp = false;
                            }
                        } else {
                            breathingStep -= 10;
                            if (breathingStep <= 0) {
                                breathingStep = 0;
                                breathingStateUp = true;
                            }
                        }
                        strip.setBrightness(breathingStep);
                        for(int i=0; i<4; i++) strip.setPixelColor(i, cfg.r, cfg.g, cfg.b);
                        strip.show();
                        lastStripUpdateTime = millis();
                    }
                    break;

                case 5: // RAINBOW
                    if (millis() - lastStripUpdateTime > 15) {
                        strip.setBrightness(255);
                        rainbowStep += 5; // Faster cycle
                        for(int i=0; i<4; i++) {
                            // Hue is 0-65535. Map step to hue shift.
                            int hue = (i * 65536 / 4) + (rainbowStep * 256);
                            strip.setPixelColor(i, strip.ColorHSV(hue & 65535));
                        }
                        strip.show();
                        lastStripUpdateTime = millis();
                    }
                    break;

                default: // Fallback to Solid
                    strip.setBrightness(255);
                    for(int i=0; i<4; i++) strip.setPixelColor(i, cfg.r, cfg.g, cfg.b);
                    strip.show();
                    break;
            }
        }

        void taskBuiltIn(void *pvParameters) {
            static uint32_t lastT = 0;
            static bool state = false;
            if (millis() - lastT > 500) {
                state = !state;
                digitalWrite(PIN_LED_BUILTIN, state);
                lastT = millis();
            }
        }
    }

    // ===================================================================================
    //  TOUCH
    // ===================================================================================
    namespace Touch {
        bool touched = false;
        
        void IRAM_ATTR isr(void *arg) { 
            uint32_t status = touch_pad_get_status();
            if (status & (1 << TOUCH_PAD_NUM3)) {
                touched = true; 
            }
            touch_pad_clear_status();
        }

        void begin() {
            touch_pad_init();
            touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
            touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
            touch_pad_config(TOUCH_PAD_NUM3, 0);
            
            // Enable filter and set dynamic threshold
            touch_pad_filter_start(10);
            delay(500); // Wait for filter to stabilize
            uint16_t touchVal;
            touch_pad_read_filtered(TOUCH_PAD_NUM3, &touchVal);
            Serial.printf("Touch Baseline: %d\n", touchVal);
            
            // Set threshold to 80% of baseline (matches original firmware)
            // If baseline is invalid (too low), use safe default
            if (touchVal > 100) {
                touch_pad_set_thresh(TOUCH_PAD_NUM3, touchVal * 4 / 5);
                touch_pad_isr_register(isr, NULL);
                touch_pad_intr_enable();
            } else {
                Serial.println("Touch Sensor Error: Baseline too low (Grounded?). Disabling Touch.");
            }
        }

        /**
         * @brief Monitors the touch sensor state.
         * @details Implements a state machine to distinguish between Short Press (Dance) and Long Press (Rest).
         */
        void taskMonitor(void *pvParameters) {
            static int state = 0;
            static uint32_t tStart = 0;
            static uint8_t danceIndex = 0;

            switch (state) {
                case 0: // Idle
                    if (touched) {
                        Buzzer::play(MELODY_BEEP_2); // Immediate beep on detection
                        tStart = millis();
                        touched = false;
                        state = 1;
                    }
                    break;
                case 1: { // Pressed, checking duration
                    bool isTouching = touched;
                    if (isTouching) { 
                        touched = false; 
                    }
                    
                    if (millis() - tStart > 700) { // Long Press (>700ms)
                        Buzzer::play(MELODY_BEEP_1);
                        enterMessageQueue(String(ACTION_UP_DOWN) + "#0#"); // Toggle Stand/Rest
                        state = 2; // Wait for release
                    } else if (millis() - tStart > 50 && !isTouching) { 
                        // Released quickly (Short Press)
                        // Only trigger if NOT touching anymore
                        Buzzer::play(MELODY_BEEP_2);
                        enterMessageQueue(String(ACTION_DANCING) + "#" + String(danceIndex) + "#");
                        danceIndex = (danceIndex + 1) % 5;
                        state = 0;
                    }
                    break;
                }
                case 2: // Wait for release (Long press cooldown)
                    if (!touched) state = 0;
                    touched = false;
                    break;
            }
        }
    }
}