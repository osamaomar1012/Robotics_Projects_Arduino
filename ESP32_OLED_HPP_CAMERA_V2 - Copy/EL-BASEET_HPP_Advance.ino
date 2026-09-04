#include "BoardConfig.h"
#include "camera_handler.h"
#include "web_server.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- EL-BASEET HPP_Advance Booting ---");

    if (setupCamera()) {
        Serial.println("Camera Engine: OK");
        cameraActive = true;
    } else {
        Serial.println("Camera Engine: FAILED");
    }

    setupWeb();
    Serial.println("System Ready. Connect to WiFi HPP_ADVANCE_SURGERY");
}

void loop() {
    // Core 1 handles the Web Server client stack
    server.handleClient();
    
    // Minimal delay to prevent Watchdog Triggers
    delay(1);
}
