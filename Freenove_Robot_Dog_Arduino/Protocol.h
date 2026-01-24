#ifndef PROTOCOL_H
#define PROTOCOL_H

// --- Action Codes ---
// These characters correspond to the command protocol used by the Freenove App.
#define ACTION_UP_DOWN      'A' // Toggle Stand/Rest or specific posture
#define ACTION_BODY_HEIGHT  'B' // Adjust body height (Y-axis)
#define ACTION_RGB          'C' // Control LED colors and modes
#define ACTION_BUZZER       'D' // Play specific frequency or melody
#define ACTION_TWIST        'E' // Rotate body (Pitch/Roll/Yaw) without moving feet
#define ACTION_MOVE_ANY     'F' // Omnidirectional movement (Trot/Crawl)
#define ACTION_CAMERA       'G' // Camera control (Resolution/Quality)
#define ACTION_ULTRASONIC   'H' // Ultrasonic sensor data request
#define ACTION_GET_VOLTAGE  'I' // Battery voltage request
#define ACTION_CALIBRATE    'J' // Calibration commands (Test/Save/Reset)
#define ACTION_SET_NVS      'K' // Save settings to Non-Volatile Storage
#define ACTION_INSTALLATION 'L' // Installation mode (90-degree servo alignment)
#define ACTION_AUTO_WALKING 'M' // Enable/Disable autonomous obstacle avoidance
#define ACTION_NETWORK      'N' // WiFi scanning and connection commands
#define ACTION_DANCING      'O' // Trigger pre-programmed dance routines
#define ACTION_SET_ROBOT    'R' // Set global robot parameters (Speed, etc.)
#define ACTION_TEST         'T' // Test commands
#define ID_CHECK            'W' // Handshake/Identity verification

// --- Melody Codes ---
#define MELODY_POWER_UP             0
#define MELODY_LOW_POWER            1
#define MELODY_NO_POWER             2
#define MELODY_WIFI_CONNECT_SUCCESS 3
#define MELODY_WIFI_CONNECT_FAILED  4
#define MELODY_WIFI_DISCONNECT      5
#define MELODY_BLE_CONNECT_SUCCESS  6
#define MELODY_BLE_DISCONNECT       7
#define MELODY_CAM_CONNECT_SUCCESS  8
#define MELODY_CAM_DISCONNECT       9
#define MELODY_CAM_FAILURE          10
#define MELODY_BEEP_1               11
#define MELODY_BEEP_2               12
#define MELODY_DHTG                 13
#define MELODY_BB_CLEAR_1           14
#define MELODY_BB_CLEAR_2           15

#endif