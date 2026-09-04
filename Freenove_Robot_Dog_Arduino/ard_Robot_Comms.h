#ifndef ARD_ROBOT_COMMS_H
#define ARD_ROBOT_COMMS_H

#include "ard_Robot_Global.h"
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
        static void startCmdServer();
        static void taskServer(void *pvParameters);
        static void taskCmdServer(void *pvParameters);
    };

    // --- Radio State Manager ---
    namespace Radio {
        extern bool isRemoteOnline;
        void begin();
        void touchRemote();
    }

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