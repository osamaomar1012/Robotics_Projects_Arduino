/**
 * MODULE: ESP32-WROVER Video Streamer
 * DESCRIPTION: Handles initialization and frame capture for the OV2640 camera.
 *              Targeted for WROVER-E Dev Boards.
 */
#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "esp_camera.h"
#include "BoardConfig.h"
#include "soc/soc.h"           // Required for brownout disable
#include "soc/rtc_cntl_reg.h"  // Required for brownout disable

void setSurgicalLightingMode(bool lightOn) {
    sensor_t * s = esp_camera_sensor_get();
    if (!s) return;

    if (lightOn) {
        // HIGH LIGHT MODE: Maximize Clarity, Minimize Noise
        s->set_gain_ctrl(s, 1);                  
        s->set_gainceiling(s, GAINCEILING_2X);   // Force low gain (removes grain/noise)
        s->set_ae_level(s, 0);                   // Normal exposure
        s->set_brightness(s, -1);                // Slightly darker to prevent blowout
        s->set_contrast(s, 1);                   // Sharper contrast for surgical detail
        s->set_saturation(s, 1);                 // Vivid colors for tissue differentiation
        s->set_sharpness(s, 2);                  // Maximize edge clarity for follicles
        s->set_denoise(s, 1);                    // Remove electronic noise
        Serial.println(F("CAM: Optimized for High Light (Low Noise)"));
    } else {
        // LOW LIGHT MODE: Maximize Visibility
        s->set_gain_ctrl(s, 1);
        s->set_gainceiling(s, GAINCEILING_32X);  // Allow high gain boost
        s->set_ae_level(s, 2);                   // Boost exposure target
        s->set_brightness(s, 1);                 // Brighter image
        s->set_contrast(s, 0);                   // Neutral contrast
        s->set_saturation(s, 0);                 // Neutral saturation
        s->set_sharpness(s, 0);                  // Lower sharpness to reduce gain-noise artifacts
        s->set_denoise(s, 1);                    // Active noise reduction
        Serial.println(F("CAM: Optimized for Low Light (High Sensitivity)"));
    }
}

/**
 * @brief Configures the OV2640 camera sensor.
 */
bool setupCamera() {
    // Disable brownout detector for debugging on weak power sources (USB)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    delay(200); // Give power rail time to stabilize before high-current camera init

    Serial.println(F("--- PSRAM Diagnostic ---"));
    if (psramFound()) {
        Serial.printf("PSRAM: Found. Size: %u bytes\n", ESP.getPsramSize());
        Serial.printf("PSRAM: Free: %u bytes\n", ESP.getFreePsram());
    } else {
        Serial.println(F("PSRAM: Not Found! Check Tools -> Board and PSRAM settings."));
    }

    if (PIN_LAMP_FLASH != -1) {
        pinMode(PIN_LAMP_FLASH, OUTPUT);
        digitalWrite(PIN_LAMP_FLASH, LOW); // Start with light OFF
    }

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_2; // Move to Channel 2 to avoid conflict with Motor PWM (Channels 0/1)
    config.ledc_timer = LEDC_TIMER_1;     // Use a dedicated timer for the camera clock
    config.pin_d0 = CAM_PIN_D0;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d7 = CAM_PIN_D7;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_pclk = CAM_PIN_PCLK;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_sscb_sda = CAM_PIN_SIOD;
    config.pin_sscb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000; // 20MHz is the limit for stable parallel bus
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST; 
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // Optimized for streaming: low resolution to save CPU for motor logic
    if (psramFound()) {
        Serial.printf("ULTRA-STREAM: Allocating 8 PSRAM Buffers. Free: %u\n", ESP.getFreePsram());
        config.frame_size = FRAMESIZE_VGA;  // 640x480: Higher resolution for surgical precision
        config.jpeg_quality = 14;           // Priority: Speed over sharpness to eliminate lag
        config.fb_count = 8;                // Maximum buffer depth for 8MB PSRAM
    } else {
        Serial.println(F("Configuring Camera for Internal RAM (Limited)"));
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    Serial.println("Initializing Camera...");
    esp_err_t err = esp_camera_init(&config); 
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    // Sensor adjustments for better visual stability
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);     // Brighter for medical use
        s->set_contrast(s, 2);       // High contrast for follicle differentiation
        s->set_saturation(s, 0);     // -2 to 2
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable

        // Low-light surgical environment optimizations
        s->set_gain_ctrl(s, 1);                  // Enable Auto Gain Control
        s->set_exposure_ctrl(s, 1);              // Enable Auto Exposure Control
        s->set_gainceiling(s, GAINCEILING_8X);   // Strict gain ceiling for ultra-clean image
        s->set_ae_level(s, 1);                   // Slight exposure boost
        s->set_bpc(s, 1);                        // Black Point Compensation
        s->set_wpc(s, 1);                        // White Point Compensation
    }
    return true;
}

#endif // CAMERA_HANDLER_H