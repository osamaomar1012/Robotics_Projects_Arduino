#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "esp_camera.h"
#include "BoardConfig.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

bool setupCamera() {
    // Disable brownout for stability during WiFi bursts
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // 1. Force De-init to clear previous state (Soft Reset fix)
    esp_camera_deinit();

    // 2. Manually Reset I2C Bus (Clear stuck lines)
    pinMode(CAM_PIN_SIOD, OUTPUT); digitalWrite(CAM_PIN_SIOD, HIGH); 
    pinMode(CAM_PIN_SIOC, OUTPUT); digitalWrite(CAM_PIN_SIOC, HIGH);
    delay(10);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_4; // Match Robot_Comms configuration
    config.ledc_timer = LEDC_TIMER_2;
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
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000; // 20MHz is standard for high-speed DMA transfer
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; // Low-latency strategy
    config.fb_location = CAMERA_FB_IN_PSRAM;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_QVGA; // 320x240 - Perfect fit for CrowPanel 2.8
        config.jpeg_quality = 14; // Matched to Robot_Comms
        config.fb_count = 1; // Minimize buffers to eliminate lag
    } else {
        return false; // Advance version requires PSRAM
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) return false;

    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);
        s->set_contrast(s, 1);
        s->set_hmirror(s, 1); // Match Robot_Comms orientation
        s->set_vflip(s, 1);   // Match Robot_Comms orientation
        // Optimize for surgical lighting
        s->set_gain_ctrl(s, 1);
        s->set_exposure_ctrl(s, 1);
        s->set_gainceiling(s, GAINCEILING_8X);
    }

    return true;
}

#endif