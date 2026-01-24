#ifndef ROBOT_WIRELESS_COMMUNICATION_H
#define ROBOT_WIRELESS_COMMUNICATION_H

#include "Robot_Global_Definitions.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

namespace Comms {

    // --- Command Parser ---
    class Parser {
    public:
        char cmdChar;
        float params[10];
        int paramCount;
        String rawParams[10];
        
        void parse(String msg);
    };

    // --- Camera & WiFi Service ---
    class Camera {
    public:
        static bool isCameraNormal;
        static void begin();
        static void setupWiFi();
        static void startServer();
        static void startCmdServer();
        static void taskServer(void *pvParameters);
        static void taskCmdServer(void *pvParameters);
    };

    // --- BLE Service ---
    namespace BLE {
        void begin();
        void send(String msg);
        void taskUpload(void *pvParameters);
    }

    // --- Main Command Task ---
    void taskCommand(void *pvParameters);
}

#endif