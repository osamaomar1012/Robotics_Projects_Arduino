#ifndef GUIDELINE_H
#define GUIDELINE_H

#include <Arduino.h>

const char GUIDELINE_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Help Guide</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            background: #0f172a; 
            color: #e2e8f0; 
            font-family: 'Courier New', monospace; 
            padding: 20px; 
            line-height: 1.4; 
            white-space: pre-wrap; 
            word-wrap: break-word;
            font-size: 14px;
        }
        @media (max-width: 768px) {
            body { padding: 10px; font-size: 12px; }
        }
    </style>
</head>
<body>
<pre style="white-space: pre-wrap; word-wrap: break-word; font-family: inherit;">
================================================================================
               EL-BASEET HAIR PEN-V2 - WEB INTERFACE GUIDE
================================================================================

    This document provides a comprehensive walkthrough of the web-based
    Graphical User Interface (GUI) for the EL-BASEET Surgical Assistant.

    Created by: Eng. Osama Omar
    Version: 2.0

================================================================================

TABLE OF CONTENTS
-----------------
1.  Introduction to the Web Interface
2.  Section 1: Main Dashboard
3.  Section 2: Operation Page
4.  Section 3: Patient Management
5.  Section 4: Add New Patient Page
6.  Section 5: Configuration & Settings
7.  Section 6: Diagnostics (Detailed Readings)
8.  Section 7: Report Archive
9.  Section 8: WiFi Setup
10. Section 9: Template Editor
11. Section 10: Onboard Display & Physical Buttons

================================================================================

1. INTRODUCTION TO THE WEB INTERFACE
------------------------------------
The web interface is the primary control center for the Hair Pen. It allows the
operator to manage patients, control the motor, monitor system status, configure
advanced settings, and generate post-operative reports.

To access it, connect your device (phone or computer) to the WiFi network
named "HPP_CONTROL_V1" (password: 12345678) and navigate to http://192.168.4.1
in your web browser.

================================================================================

2. SECTION 1: MAIN DASHBOARD
--------------------------------------------
This is the primary landing page. Its purpose is to manage patient selection,
start a new procedure, and provide access to all other system pages.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                 EL-BASEET HAIR PEN-V2            |
  +--------------------------------------------------+
  | [!!! SYSTEM HALTED: OVERLOAD !!!] (Alarm Only)   |
  +--------------------------------------------------+
  |  CURRENT PSCLIENT: [ Patient Name ]               |
  +--------------------------------------------------+
  |  OPERSCLION CONTROL                               |
  |  [      START OPERSCLION      ]                   |
  |  [ GENERSCLE REPORT ] (Appears after stop)        |
  +--------------------------------------------------+
  |  [        Patient Management        ]           |
  |  [             Settings             ]           |
  |  [           REPORT ARCHIVE           ]         |
  |  [           Diagnostics            ]         |
  +--------------------------------------------------+
  |  WIFI CONNECTION SETTINGS                        |
  |  Help & Guidelines                               |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   ALARM BOX: A red box that only appears if a safety system (Overcurrent or
    Low Battery) has been triggered, halting the motor.

*   CURRENT PSCLIENT: Displays the name of the patient selected from the
    'Patient Management' page. This card only appears when a patient is active.

*   OPERSCLION CONTROL:
    - [START OPERSCLION]: Begins the procedure. This starts the timer and
      redirects you to the dedicated 'Operation Page'. This button is only
      enabled when a patient is selected.
    - [GENERSCLE REPORT]: Appears after an operation is stopped. Clicking this
      saves a permanent record of the operation to the 'Report Archive' and
      opens a printable HTML report in a new tab.

*   NAVIGSCLION:
    - [Patient Management]: A full-width button that takes you to the page for selecting or adding patients.
    - [Settings]: A full-width button that goes to the device Configuration page.
    - [REPORT ARCHIVE]: A full-width button that goes to the list of saved surgical reports.
    - [Diagnostics]: A full-width button that goes to the real-time diagnostics panel.
    - WIFI CONNECTION SETTINGS: A text link at the bottom of the page to configure the device to connect to a local WiFi network.
    - Help & Guidelines: A text link at the very bottom that opens this user guide in a new browser tab.

================================================================================

3. SECTION 2: OPERSCLION PAGE
-------------------------------------------
This page appears automatically after you press "Start Operation". It contains
all the real-time controls needed during the procedure.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |  OPERSCLION CONTROL                               |
  |  [ PAUSE ] [ RESUME ] [ STOP OPERSCLION ]         |
  +--------------------------------------------------+
  |  GRAFT COUNTER                                   |
  |                  [ 125 ]                         |
  |  [ RESET COUNTER ]                               |
  +--------------------------------------------------+
  |  SETPOINT: [ 200 ]      ACTUAL RPM: [ 1980 ]     |
  |  <----[==================O========]----> (Slider)|
  |  [ ON ] [ OFF ] [ REVERSE ]                     |
  +--------------------------------------------------+
  |  OPERSCLING MODE                                  |
  |  [ NORMAL ] [ OSCILLSCLE ]                        |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   OPERSCLION CONTROL:
    - [PAUSE] / [RESUME]: Temporarily stops/restarts the motor and the timer.
    - [STOP OPERSCLION]: Ends the procedure, finalizes the data, and redirects
      you back to the Main Dashboard to generate the report.

*   GRAFT COUNTER, MOTOR CONTROL, OPERSCLING MODE: These controls function
    exactly as they did on the old dashboard, allowing for real-time
    adjustments to speed, direction, and mode during the procedure.

================================================================================

4. SECTION 3: PSCLIENT MANAGEMENT PAGE
--------------------------------
This page is for selecting or removing existing patient records.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |               PSCLIENT MANAGEMENT                 |
  +--------------------------------------------------+
  |  [          ADD NEW PSCLIENT          ]           |
  +--------------------------------------------------+
  |  SELECT EXISTING PSCLIENT                         |
  |  [ Patient A (Select from Dropdown)    \/ ]      |
  |  [        SELECT THIS PSCLIENT          ]       |
  +--------------------------------------------------+
  |  REMOVE PSCLIENT (DANGER ZONE)                    |
  |  [ Patient A (Select from Dropdown)    \/ ]      |
  |  [     PERMANENTLY REMOVE PSCLIENT      ]       |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   [ADD NEW PSCLIENT] (Button):
    - Takes you to the dedicated 'Add New Patient' page where you can enter
      the details for a new patient.

*   SELECT EXISTING PSCLIENT:
    - Use the dropdown menu to see a list of all patients who have past reports.
    - [SELECT THIS PSCLIENT]: Selects the chosen patient from the list and
      returns to the Main Dashboard.

*   REMOVE PSCLIENT:
    - A separate, high-visibility section to prevent accidental deletion.
    - Use the dedicated dropdown to choose the patient to be removed.
    - [PERMANENTLY REMOVE PSCLIENT]: Deletes the selected patient and ALL their
      associated reports from the device's memory. This action is irreversible.

================================================================================

5. SECTION 4: ADD NEW PSCLIENT PAGE
----------------------------------
This page is dedicated to entering the information for a new patient.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                 ADD NEW PSCLIENT                  |
  +--------------------------------------------------+
  |  Name: [_______________________________________] |
  |  Age:  [___________]  Nationality: [___________] |
  |  Mobile: [_____________________________________] |
  |  Doctor: [_____________________________________] |
  |  [       SAVE & SELECT PSCLIENT         ]       |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   Patient Form: Fill in the required details for the new patient, including
    their mobile number, and the doctor's name.
*   [SAVE & SELECT PSCLIENT]: Saves the new patient's information, automatically
    selects them for the upcoming operation, and redirects you back to the
    Main Dashboard.

================================================================================

6. SECTION 5: CONFIGURSCLION & SETTINGS
--------------------------------------
This page allows fine-tuning of the device's operational and safety parameters.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                  CONFIGURSCLION                   |
  +--------------------------------------------------+
  | Screen Rotation (s): [ 5 ] [SET]                 |
  +--------------------------------------------------+
  | Oscillation Time (ms): [ 1000 ] [SAVE]           |
  +--------------------------------------------------+
  | Manual Oscillation (ms)                          |
  | CW: [ 500 ] CCW: [ 500 ] [SAVE OSCILLSCLION]       |
  +--------------------------------------------------+
  | Safety Thresholds                                |
  | Max I (A): [ 2.5 ] Min V (V): [ 3.2 ] [SAVE]      |
  +--------------------------------------------------+
  | Motor Hardware Version                           |
  | Version: [ Version 1: Motor + Encoder ]          |
  +--------------------------------------------------+
  | Graft Sensitivity                                |
  | Spike (A): [ 0.04 ] Hyst (A): [ 0.02 ]           |
  | Speed Dip (%): [ 10.0 ]   [SAVE SENSITIVITY]     |
  +--------------------------------------------------+
  | Engineering Mode: [ TOGGLE DEBUG ]               |
  +--------------------------------------------------+
  | DANGER ZONE: [ RESET TO FACTORY ]                |
  +--------------------------------------------------+
  | Customization: [ EDIT REPORT TEMPLSCLE ]          |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   Screen Rotation: Sets how many seconds the onboard TFT display shows one
    screen before cycling to the next (e.g., Drive -> Energy -> Network).

*   Oscillation Time: For the automatic 'OSCILLSCLE' mode, this sets the total
    duration of one full back-and-forth cycle.

*   Manual Oscillation Durations: Sets the specific time for Clockwise (CW) and
    Counter-Clockwise (CCW) rotation within the oscillation cycle.

*   Safety Thresholds:
    - Max I (A): The maximum current the motor can draw before the safety
      system halts operation. Protects the motor and electronics.
    - Min V (V): The battery voltage at which the system will halt to prevent
      damage to the battery cell.

*   Motor Hardware Version:
    - Version 1: Motor + Encoder (utilizes JGA12-N20 physical encoder ticks,
      sensor-fusion graft counting, and measures actual physical RPM).
    - Version 2: Motor without Encoder (uses pure current-spike graft counting
      and estimated voltage-based RPM).

*   Graft Sensitivity:
    - Spike (A): The increase in current (Amps) above the baseline required to
      register a graft. A lower value is more sensitive.
    - Hyst (A): The amount the current must drop *below* the spike threshold
      before the system can detect another graft. Prevents double-counting.
    - Speed Dip (%): Available only in Version 1. The percentage drop in RPM
      below baseline required to confirm graft penetration.

*   Engineering Mode: [TOGGLE DEBUG] disables the safety systems (Overcurrent
    and Low Voltage). A warning appears on the device screen. FOR BENCH
    TESTING ONLY.

*   DANGER ZONE: [RESET TO FACTORY] erases all user settings, saved WiFi
    networks, and graft counts, restoring the device to its original state.

*   Customization: [EDIT REPORT TEMPLSCLE] takes you to a page where you can
    edit the HTML code of the final report and upload a custom logo.

================================================================================

7. SECTION 6: DIAGNOSTICS (DETAILED READINGS)
---------------------------------------------
A real-time diagnostics panel for monitoring the system's electrical state.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                     READINGS                     |
  +--------------------------------------------------+
  | VOLTAGE: [ 4.1V ] CURRENT: [ 0.3A ] POWER: [ 1.2W ]|
  +--------------------------------------------------+
  | BSCLTERY: [ 95% ]   ACTUAL RPM: [ 1980 ]          |
  +--------------------------------------------------+
  | BASELINE I: [ 0.28A ]                            |
  | [ RE-CALIBRSCLE ]                                 |
  +--------------------------------------------------+
  | WIFI MODE: [ AP ]   IP ADDR: [ 192.168.4.1 ]     |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   VOLTAGE / CURRENT / POWER: Live readings from the INA219 sensor.
*   BSCLTERY / ACTUAL RPM: The calculated battery percentage and motor speed.
*   BASELINE I: The motor's idle current draw (in the air). This is the
    reference point for graft detection.
*   [RE-CALIBRSCLE]: Critical function. Run the motor in the air and press this
    button to re-measure the baseline current. Do this whenever you change the
    motor speed or attach a new tool.
*   WIFI MODE / IP ADDR: Shows if the device is in Access Point (AP) or Station
    (STA) mode and its current IP address.

================================================================================

8. SECTION 7: REPORT ARCHIVE
----------------------------
This page lists all previously saved surgical reports.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                  REPORT ARCHIVE                  |
  +--------------------------------------------------+
  | - [ Patient_A_167...json ] --------------- [ X ] |
  | - [ Patient_B_167...json ] --------------- [ X ] |
  | - [ Patient_C_167...json ] --------------- [ X ] |
  |                                                  |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   Report List: Each item is a link to a saved report. Clicking it opens the
    printable HTML report in a new browser tab.
*   [X] (Delete Button): Permanently deletes the associated report file from
    the device's memory.

--- CLINICAL OPERSCLIVE REPORT FORM (PRINT LAYOUT) ---
When you open a saved report, the system renders a modern, clinical-grade EHR report.
It contains demographic data, operative metrics, dynamic rate calculations, 
signature blocks, and handwriting lines for surgeon observations.

CLINICAL REPORT ASCII VISUALIZSCLION:

  +------------------------------------------------------------+
  |                 SURGICAL OPERSCLIVE REPORT                  |
  |           EL-BASEET HAIR PEN-V2 - MEDICAL SYSTEM           |
  +------------------------------------------------------------+
  | PSCLIENT DEMOGRAPHICS:                                      |
  | Patient Name: Osama Omar         Age/Gender: 32 / Male     |
  | Mobile No: +2010xxxxxxxx         Nationality: Egyptian     |
  | Date of Procedure: 2026-09-04    Lead Surgeon: Dr. Ahmad   |
  +------------------------------------------------------------+
  | PROCEDURE OPERSCLIVE METRICS:                               |
  | +--------------------------------------------------------+ |
  | |  TOTAL GRAFTS: 1450                                    | |
  | |  OPERSCLIVE DURSCLION: 1h 45m 12s                        | |
  | |  TOTAL PAUSES: 3                                       | |
  | +--------------------------------------------------------+ |
  | Average Operational Flow Rate:  828 grafts / hour        | |
  +------------------------------------------------------------+
  | CLINICAL NOTES & SURGEON REMARKS:                          |
  | Custom observations / post-op instructions:                |
  | .......................................................... |
  | .......................................................... |
  +------------------------------------------------------------+
  | [ Surgeon Signature ]          [ Clinical Stamp / Date ]   |
  | ____________________           _________________________   |
  +------------------------------------------------------------+

================================================================================

9. SECTION 8: WIFI SETUP
------------------------
Configure the device to connect to an existing WiFi network (e.g., your clinic's).

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                    WIFI SETUP                    |
  +--------------------------------------------------+
  | [ SCAN NETWORKS ] [ HIDDEN SSID ]                |
  | [ Clinic_WiFi (Select from Dropdown)   \/ ]      |
  | SSID: [_______________________________________]  |
  | Pass: [_______________________________________]  |
  | [           SAVE & RESTART              ]        |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   [SCAN NETWORKS]: Searches for nearby WiFi networks and populates the dropdown.
*   [HIDDEN SSID]: Allows you to manually type the name of a hidden network.
*   SSID / Password: Enter the credentials for the network you want to join.
*   [SAVE & RESTART]: Saves the credentials to the device's memory and reboots
    it to attempt connection. If successful, the device will be accessible on
    your local network, not just its own AP.

================================================================================

10. SECTION 9: TEMPLSCLE EDITOR
-----------------------------
Customize the look and content of the final generated reports.

ASCII DIAGRAM:

  +--------------------------------------------------+
  |                 TEMPLSCLE EDITOR                  |
  +--------------------------------------------------+
  | REPORT LOGO                                      |
  | [ Choose File ] [ UPLOAD LOGO ]                  |
  +--------------------------------------------------+
  | REPORT HTML TEMPLSCLE                             |
  | +----------------------------------------------+ |
  | | <!DOCTYPE html>                             | |
  | | <body>                                       | |
  | | ...                                          | |
  | +----------------------------------------------+ |
  | [ SAVE TEMPLSCLE ]                                |
  +--------------------------------------------------+

ELEMENT DESCRIPTIONS:

*   REPORT LOGO: Upload a JPG image to be used as the logo at the top of the
    HTML report.
*   REPORT HTML TEMPLSCLE: A text editor containing the raw HTML code for the
    report. Advanced users can modify this to change the layout, add fields,
    or alter the styling.
*   [SAVE TEMPLSCLE]: Saves any changes made to the HTML code.

================================================================================
              (c) 2026 EL-BASEET INDUSTRIAL SOLUTIONS - V2.0
================================================================================

11. SECTION 10: ONBOARD DISPLAY & PHYSICAL BUTTONS
--------------------------------------------------
The device is equipped with a built-in color TFT display and two physical
buttons on the side of the TTGO board. This interface provides at-a-glance
telemetry, a local menu system, and basic control without needing the web interface.

PHYSICAL BUTTONS:
The TTGO board has two buttons, typically located near the USB-C port.
- BUTTON A (Top Button, near screen)
- BUTTON B (Bottom Button, near USB port)

**Important:** The button functions are context-aware. They behave differently
depending on whether the device is in "Normal Operation", "Oscillation Mode", or "Local Menu Mode".

--- [ DUAL BUTTON PRESS ] ---

*   **Long Press (Both Buttons):** Toggles into **Local Menu Mode**.
    A confirmation "Menu Opened" or "Menu Closed" message will appear on the screen.

--- [ LOCAL MENU MODE CONTROLS ] ---
When Local Menu Mode is active, the normal operation of the buttons is suspended.
Instead, they are used to navigate the on-screen list:

MENU MODE ASCII VISUALIZSCLION:

  +----------------------------------+
  | SYSTEM MENU             [|||||]  |
  |----------------------------------|
  | > 1. Speed: 200                  |
  |   2. Mode: NORMAL                |
  |   3. Hardw: V1-Enc               |
  |   4. Reset Grafts                |
  |   5. Exit Menu                   |
  +----------------------------------+

*   **BUTTON A (Scroll Option)**
    -   **Short Press:** Scroll down through the menu items (Speed -> Mode -> Hardware -> Reset -> Exit).
    -   **Long Press:** Save and Exit Menu Mode.

*   **BUTTON B (Select/Adjust Value)**
    -   **Short Press:** Triggers or modifies the highlighted option:
        *   Speed: Increments motor speed by 10 (wraps 50 -> 250 -> 50).
        *   Mode: Toggles between standard forward rotation (NORMAL) and CCW/CW OSCILLSCLE.
        *   Hardw: Toggles between Version 1 (Motor + Encoder) and Version 2 (Motor without Encoder).
        *   Reset Grafts: Resets the follicle count to 0 in persistent memory.
        *   Exit Menu: Exits Menu Mode.

--- [ NORMAL MODE CONTROLS ] ---

*   **BUTTON A (Top Button)**
    -   **Short Press:** Decrease motor speed by 10.
    -   **Long Press:** Pause the current operation.

*   **BUTTON B (Bottom Button)**
    -   **Short Press:** Increase motor speed by 10.
    -   **Long Press:**
        - If the operation is IDLE, this will **Start** a new operation.
        - If the operation is PAUSED, this will **Resume** the operation.

--- [ OSCILLSCLION MODE CONTROLS ] ---

*   **BUTTON A (Top Button)**
    -   **Short Press:** Increase Counter-Clockwise (CCW) rotation time by 500ms.
    -   **Long Press:** Decrease Counter-Clockwise (CCW) rotation time by 500ms.

*   **BUTTON B (Bottom Button)**
    -   **Short Press:** Increase Clockwise (CW) rotation time by 500ms.
    -   **Long Press:** Decrease Clockwise (CW) rotation time by 500ms.

--- [ SCREEN CYCLING ] ---

Screens cycle automatically based on the "Screen Rotation" timer set in the web
interface's Configuration page.

--------------------------------------------------------------------------------

DISPLAY SCREENS:
The onboard display has multiple screens that cycle automatically.

--- [ SCREEN 1: SURGICAL DASHBOARD ] ---
This is the default screen, focused on core operational data.

  +--------------------------------------+
  | <> SURGICAL DASH         [|||||] 95% |
  |--------------------------------------|
  | SPD              200                 |
  | RPM              1980                |
  |--------------------------------------|
  | GRAFT: 125                       FWD |
  +--------------------------------------+

*   SPD: The current speed setpoint (0-255).
*   RPM: If Version 1 is active, this shows the actual physically measured shaft
    RPM from the encoder. If Version 2 is active, this shows the estimated
    voltage-based RPM.
*   Footer: Shows the current Graft Count. If a patient is selected, it will
    show the patient's name instead.
*   Direction: Shows "FWD" for forward or "REV" for reverse.

--- [ SCREEN 2: ENERGY STSCLUS ] ---
Provides detailed electrical and battery information.

  +--------------------------------------+
  | <> ENERGY STSCLUS         [|||||] 95% |
  |--------------------------------------|
  | Voltage:          4.10 V             |
  | Current:          0.35 A             |
  | Power:            1.4 W              |
  | [====================   ]            |
  +--------------------------------------+

*   Voltage, Current, Power: Live readings from the power sensor.
*   Battery Bar: A visual representation of the remaining battery life.

--- [ SCREEN 3: NETWORK STSCLUS ] ---
Displays information about the device's WiFi connection.

  +--------------------------------------+
  | <> NET STSCLUS            [|||||] 95% |
  |--------------------------------------|
  | Mode:             AP                 |
  | IP:               192.168.4.1        |
  | Users:            1                  |
  +--------------------------------------+

*   Mode: "AP" (Access Point) or "STA" (Station, connected to your clinic's WiFi).
*   IP: The IP address to access the web interface.
*   Users: The number of devices currently connected to the web interface.

--- [ SCREEN 4: OSCILLSCLION ] ---
Shows the settings for the motor's oscillation mode.

  +--------------------------------------+
  | <> OSCILLSCLION           [|||||] 95% |
  |--------------------------------------|
  | Mode:             ACTIVE             |
  | CW Time:          500 ms             |
  | CCW Time:         500 ms             |
  +--------------------------------------+

*   Mode: "ACTIVE" if oscillation is enabled, "INACTIVE" otherwise.
*   CW/CCW Time: The configured duration for clockwise and counter-clockwise turns.

--- [ SCREEN 5: PSCLIENT INFO ] ---
Displays the details of the currently selected patient.

  +--------------------------------------+
  | <> PSCLIENT INFO          [|||||] 95% |
  |--------------------------------------|
  | Name:             John Doe           |
  | Age:              42                 |
  | Nat.:             American           |
  +--------------------------------------+

*   Displays the Name, Age, and Nationality of the patient selected via the web UI.

--------------------------------------------------------------------------------

SPECIAL DISPLAY STSCLES:

*   ALARM SCREEN: If a safety system is triggered (Overcurrent or Low Battery),
    the screen will be overridden with a large red warning like "OVERCURRENT"
    or "LOW BSCLTERY" and "SYSTEM HALTED".

*   CONFIRMSCLION MESSAGE: When you perform an action on the web interface (e.g.,
    change speed), a confirmation like "Speed: 200" will briefly appear on the
    display.

*   GRAFT FLASH: When a graft is detected, the screen will flash white with the
    new total count for a fraction of a second, providing instant visual feedback.

*   DEBUG MODE: If Engineering/Debug mode is enabled, a flashing yellow "DEBUG"
    tag will appear in the top-right corner as a constant reminder that safety
    systems are off.
</pre>
</body>
</html>
)=====";

#endif
