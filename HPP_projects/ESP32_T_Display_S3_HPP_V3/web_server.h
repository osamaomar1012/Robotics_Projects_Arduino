/**

 * MODULE: Web Server Dashboard

 * DESCRIPTION: Provides a web interface in both AP and STA modes for 

 *              monitoring and controlling the HPP system.

 * Created by Eng.Osama Omar

 */

#ifndef WEB_SERVER_H

#define WEB_SERVER_H



#include <WiFi.h>

#include <WebServer.h>

#include <Preferences.h>

#include <FS.h>

#include <ArduinoJson.h>





// WiFi Credentials - Update these for your network

const char* ssid_sta = "";

const char* pass_sta = "";

const char* ssid_ap  = "HPP_CONTROL_V1";

const char* pass_ap  = "12345678";



WebServer server(80);



// Externs to link with main project globals

extern int16_t motorSpeed;

extern float batteryVoltage, batteryCurrent, batteryPower, maxCurrentThreshold, minVoltageThreshold, baselineCurrent, internalVoltage, baselineRPM, speedDipThresholdPercent;

extern float penetrationOffset, penetrationHysteresis;

extern bool calibrationNeeded;

extern uint32_t estimatedRPM, penetrationCount;

extern uint8_t batteryPercent, webBaseSpeed, motorVersion, currentMenuItem;

extern void setMotorVersion(uint8_t version);

enum OperationState { IDLE, RUNNING, PAUSED };

enum AnimationTrigger { ANIM_NONE, ANIM_START, ANIM_STOP, ANIM_REPORT };

extern volatile AnimationTrigger pendingAnimation;

enum ScanState { SCAN_IDLE, SCAN_REQUESTED, SCANNING, SCAN_COMPLETE }; extern volatile ScanState scanState;

extern String initialScanResultsJson;

extern OperationState operationState;

extern String patientMobile;

extern uint32_t screenSwitchTime;

extern uint32_t deepSleepTimeoutMs;

extern unsigned long lastInteractionTime;

extern bool systemEnabled, forwardDirection, safetyTripped, lowBatteryTripped, debugMode, isPenetrating, wifiConnecting, oscillatingMode, isCharging, inMenuMode;

  extern String patientName, patientAge, patientNationality, doctorName;

extern unsigned long operationTimeAccumulator, lastTimeCapture; extern uint32_t pauseCount;

extern uint32_t oscillationDuration;

extern uint32_t oscillationDurationCW, oscillationDurationCCW; extern Preferences preferences;

extern uint8_t ledBrightness;

extern void setLEDBrightness(uint8_t brightness);

extern void displayWebConfirmation(String msg); // Function to display confirmation on OLED

extern String getFormattedDate(); // Function to get date from NTP



// WiFi Config Page HTML

const char WIFI_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - WiFi</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 400px; margin: auto; padding: 20px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); box-shadow: 0 10px 25px rgba(0,0,0,0.3); }

        input, select { width: 90%; padding: 12px; margin: 10px 0; background: #334155; color: white; border: 1px solid #475569; border-radius: 8px; outline: none; }

        input:focus { border-color: #38bdf8; }

        .btn { padding: 12px 20px; font-size: 15px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; color: white; transition: 0.3s; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }

        .btn-blue { background: #0284c7; box-shadow: 0 4px 12px rgba(2, 132, 199, 0.3); }

        .btn-green { background: #10b981; box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3); width: 100%; margin-top: 15px; }

        .btn:active { transform: scale(0.95); }

        .link-footer { margin-top: 25px; display: block; color: #94a3b8; text-decoration: none; font-size: 0.85em; }

        .status-box { padding: 12px; border-radius: 8px; margin-bottom: 20px; font-size: 0.9em; font-weight: 600; display: flex; align-items: center; justify-content: center; gap: 8px; border: 1px solid rgba(255,255,255,0.1); }

        .status-connected { background: rgba(16, 185, 129, 0.1); color: #34d399; border-color: rgba(16, 185, 129, 0.3); }

        .status-disconnected { background: rgba(239, 68, 68, 0.1); color: #f87171; border-color: rgba(239, 68, 68, 0.3); }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2>WIFI SETUP</h2>

        </div>



        ##WIFI_STATUS_BOX##



        <div class="card">

            <button class="btn btn-blue" onclick="scan()">SCAN NETWORKS</button>

            <button class="btn btn-blue" style="background: #7f8c8d;" onclick="manual()">HIDDEN SSID</button>

            <div id="networks"></div>

            <form action="/savewifi" method="POST">

                <div style="text-align: left; padding-left: 5%;">SSID:</div>

                <input type="text" name="ssid" id="ssid" placeholder="Enter SSID" required>

                <div style="text-align: left; padding-left: 5%;">Password:</div>

                <input type="password" name="pass" id="pass" placeholder="Enter Password">

                <div style="text-align: left; padding-left: 5%; font-size: 0.8em; margin-bottom: 10px;">

                    <input type="checkbox" style="width: auto; margin: 0; vertical-align: middle;" onclick="togglePass()"> Show Password

                </div>

                <input type="submit" class="btn btn-green" value="SAVE & RESTART">

            </form>

        </div>

        <a href="/" class="link-footer">&larr; Return to Dashboard</a>

    </div>

    <script>

        window.onload = function() { loadInitialScan(); }; // Load cached results on page load

        function scan() {

            document.getElementById('networks').innerHTML = "<p style='color:#f39c12; font-weight:bold;'>Scanning...<br><span style='font-size:0.8em; font-weight:normal;'>Your phone will disconnect for a moment. Please wait up to 10 seconds for results. If the page freezes, reconnect to the 'HPP_CONTROL_V1' WiFi network.</span></p>";

            // Use a two-step scan process for reliability

            fetch('/scan').then(() => {

                // Wait for the scan to complete and the AP to restart

                setTimeout(() => {

                    fetch('/scanresults').then(r => r.json()).then(data => {

                        if (data.length === 0) {

                            document.getElementById('networks').innerHTML = "<p>No networks found. Please try again.</p>";

                            return;

                        }

                        let html = '<select onchange="document.getElementById(\'ssid\').value=this.value">';

                        html += '<option value="">-- Select Network --</option>';

                        data.forEach(n => { html += `<option value="${n.ssid}">${n.ssid} (${n.rssi}dBm)</option>`; });

                        html += '</select>';

                        document.getElementById('networks').innerHTML = html;

                    }).catch(err => { document.getElementById('networks').innerHTML = "<p>Scan failed to retrieve results.</p>"; });

                }, 10000); // Increased timeout to 10s for reliability

            });

        }

        function loadInitialScan() {

            document.getElementById('networks').innerHTML = "<p>Loading initial results...</p>";

            fetch('/initialscanresults').then(r => r.json()).then(data => {

                if (data.length === 0) {

                    document.getElementById('networks').innerHTML = "<p>No networks found on startup. Use Scan button.</p>";

                    return;

                }

                let html = '<select onchange="document.getElementById(\'ssid\').value=this.value">';

                html += '<option value="">-- Select Network --</option>';

                data.forEach(n => { html += `<option value="${n.ssid}">${n.ssid} (${n.rssi}dBm)</option>`; });

                html += '</select>';

                document.getElementById('networks').innerHTML = html;

            }).catch(err => { document.getElementById('networks').innerHTML = "<p>Could not load initial results.</p>"; });

        }

        function manual() {

            document.getElementById('networks').innerHTML = "<p style='font-size:0.8em;color:#aaa;'>Enter the Hidden SSID manually below.</p>";

            document.getElementById('ssid').value = "";

            document.getElementById('ssid').focus();

        }

        function togglePass() {

            var x = document.getElementById("pass");

            x.type = x.type === "password" ? "text" : "password";

        }

    </script>

</body>

</html>

)=====";



// Detailed Readings Page

const char READINGS_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Readings</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 400px; margin: auto; padding: 20px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); display: flex; justify-content: space-around; }

        .btn { padding: 8px 15px; font-size: 12px; cursor: pointer; border: none; border-radius: 6px; color: white; transition: 0.3s; font-weight: 600; text-transform: uppercase; }

        .value { color: #facc15; font-weight: 600; font-size: 1.3em; font-family: 'Courier New', Courier, monospace; }

        .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px; }

        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }

        .status-dot { height: 10px; width: 10px; background-color: #10b981; border-radius: 50%; display: inline-block; margin-right: 5px; box-shadow: 0 0 8px #10b981; }

        .compare-row { display: flex; justify-content: space-between; align-items: center; width: 100%; margin: 4px 0; }

        .compare-label { font-size: 0.8em; color: #94a3b8; text-align: left; }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,11.67 4.53,11.34 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2>READINGS</h2>

        </div>

        <div class="card" style="flex-direction: column; padding: 18px 25px; gap: 8px;">

            <div style="font-weight: bold; font-size: 0.8em; color: #38bdf8; text-transform: uppercase; letter-spacing: 1.5px; border-bottom: 1px solid rgba(255,255,255,0.08); padding-bottom: 8px; width:100%; text-align: left;">Voltage Comparison</div>

            <div class="compare-row">

                <span class="compare-label">INA219 Sensor (Battery Pack)</span>

                <span id="v" class="value">0.00V</span>

            </div>

            <div class="compare-row">

                <span class="compare-label">TTGO Internal ADC (Board Pin)</span>

                <span id="v_int" class="value">0.00V</span>

            </div>

            <div class="compare-row" style="border-top: 1px solid rgba(255,255,255,0.05); padding-top: 8px; margin-top: 5px;">

                <span class="compare-label" style="font-weight:600;">Power State</span>

                <span id="pwr_status" class="value" style="font-size: 1.1em; font-weight: bold;">Checking...</span>

            </div>

        </div>



        <div class="card" style="flex-direction: column; padding: 18px 25px; gap: 8px;">

            <div style="font-weight: bold; font-size: 0.8em; color: #38bdf8; text-transform: uppercase; letter-spacing: 1.5px; border-bottom: 1px solid rgba(255,255,255,0.08); padding-bottom: 8px; width:100%; text-align: left; margin-bottom: 10px;">Live Telemetry Plot</div>

            <canvas id="telemetryGraph" width="340" height="150" style="width: 100%; border-radius: 8px; background: #070b13; border: 1px solid rgba(255,255,255,0.05);"></canvas>

            <div style="display: flex; justify-content: space-around; font-size: 0.7em; margin-top: 5px; width: 100%;">

                <span style="color: #38bdf8; font-weight: bold;">● Volts (x30)</span>

                <span style="color: #facc15; font-weight: bold;">● Amps (x120)</span>

                <span style="color: #10b981; font-weight: bold;">● RPM (x0.15)</span>

            </div>

        </div>



        <div class="card">

            <div><div class="label">CURRENT</div><div id="i" class="value">0.0A</div></div>

            <div><div class="label">POWER</div><div id="p" class="value">0.0W</div></div>

        </div>

        <div class="card">

            <div><div class="label">BATTERY</div><div id="pct" class="value">0%</div></div>

            <div><div class="label">ACTUAL RPM</div><div id="rpm" class="value">0</div></div>

        </div>

        <div class="card" style="flex-direction: column; align-items: center;">

            <div><div class="label">BASELINE I</div><div id="baseI" class="value">0.0A</div></div>

            <button id="calBtn" class="btn" style="background: #475569; margin-top: 10px;" onclick="sendCmd('calibrate', 'now')">RE-CALIBRATE</button>

            <div id="calMsg" style="color: #f39c12; display:none; margin-top: 10px;">CALIBRATING...</div>

        </div>

        <div class="card">

            <div><div class="label">LED ILLUMINATION (PIN 18)</div><div id="ledDiag" class="value">0%</div></div>

            <div><div class="label">LED PWM DUTY</div><div id="ledPwm" class="value">0 / 255</div></div>

        </div>

        <div class="card">

            <div><div class="label">WiFi MODE</div><div id="wifi" class="value">-</div></div>

            <div><div class="label">IP ADDR</div><div id="ip" class="value">-</div></div>

        </div>

        <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>

    </div>

    <script>

        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }

        function updateLed(val) {

            let pwm = Math.round(val * 255 / 100);

            let ledValElem = document.getElementById('ledVal');

            let ledSliderElem = document.getElementById('ledSlider');

            let ledStatusElem = document.getElementById('ledStatus');

            if (ledValElem) ledValElem.innerText = val + '%';

            if (ledSliderElem) ledSliderElem.value = val;

            if (ledStatusElem) {

                ledStatusElem.innerText = (val > 0) ? 'ON' : 'OFF';

                ledStatusElem.style.color = (val > 0) ? '#38bdf8' : '#94a3b8';

            }

            sendCmd('led', pwm);

        }

        function sendLedPower(state) {

            updateLed(state === 'on' ? 100 : 0);

        }



        // --- Real-time Rolling Graph Logic ---

        const canvas = document.getElementById('telemetryGraph');

        const ctx = canvas.getContext('2d');

        const maxPoints = 50;

        const vData = [];

        const iData = [];

        const rpmData = [];



        function drawGraph() {

            ctx.clearRect(0, 0, canvas.width, canvas.height);

            

            // Draw visual gridlines

            ctx.strokeStyle = 'rgba(255, 255, 255, 0.04)';

            ctx.lineWidth = 1;

            for (let y = 30; y < canvas.height; y += 30) {

                ctx.beginPath();

                ctx.moveTo(0, y);

                ctx.lineTo(canvas.width, y);

                ctx.stroke();

            }



            // Render scaled data series

            plotSeries(vData, '#38bdf8', 30);    // Scale Voltage (e.g. 4V -> 120px)

            plotSeries(iData, '#facc15', 120);   // Scale Current (e.g. 0.5A -> 60px)

            plotSeries(rpmData, '#10b981', 0.15); // Scale RPM (e.g. 800 RPM -> 120px)

        }



        function plotSeries(data, color, scale) {

            if (data.length < 2) return;

            ctx.strokeStyle = color;

            ctx.lineWidth = 2;

            ctx.beginPath();

            

            const dx = canvas.width / (maxPoints - 1);

            for (let i = 0; i < data.length; i++) {

                const x = i * dx;

                const scaledVal = data[i] * scale;

                const y = canvas.height - Math.min(scaledVal, canvas.height - 5);

                

                if (i === 0) ctx.moveTo(x, y);

                else ctx.lineTo(x, y);

            }

            ctx.stroke();

        }



        setInterval(() => {

            fetch('/telemetry').then(r => r.json()).then(data => {

                document.getElementById('v').innerText = data.v.toFixed(2) + 'V';

                document.getElementById('v_int').innerText = data.v_int.toFixed(2) + 'V';

                

                let statusText = "DISCHARGING (Battery)";

                let statusColor = "#f59e0b"; // amber

                if (data.chg === 1) {

                    statusText = "CHARGING via USB ⚡";

                    statusColor = "#10b981"; // green

                }

                document.getElementById('pwr_status').innerText = statusText;

                document.getElementById('pwr_status').style.color = statusColor;



                document.getElementById('i').innerText = data.i + 'A';

                document.getElementById('p').innerText = data.p + 'W';

                document.getElementById('pct').innerText = data.pct + '%';

                document.getElementById('rpm').innerText = data.rpm;

                document.getElementById('baseI').innerText = data.baseI + 'A';

                document.getElementById('wifi').innerText = data.wifi_mode;

                document.getElementById('ip').innerText = data.ip;

                document.getElementById('calMsg').style.display = data.calib ? 'block' : 'none';

                document.getElementById('calBtn').style.display = data.calib ? 'none' : 'inline-block';

                if (document.getElementById('ledDiag')) {

                    document.getElementById('ledDiag').innerText = (data.led_pct !== undefined ? data.led_pct : Math.round(data.led * 100 / 255)) + '%';

                    document.getElementById('ledPwm').innerText = data.led + ' / 255';

                }



                // Append and roll the telemetry arrays

                vData.push(data.v);

                if (vData.length > maxPoints) vData.shift();



                iData.push(data.i);

                if (iData.length > maxPoints) iData.shift();



                rpmData.push(data.rpm);

                if (rpmData.length > maxPoints) rpmData.shift();



                drawGraph();

            });

        }, 500); // Polling every 500ms for fast telemetry plotting responsiveness

    </script>

</body>

</html>

)=====";



// Configuration & Safety Page

const char CONFIG_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Config</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 400px; margin: auto; padding: 20px; box-sizing: border-box; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); text-align: center; }

        .btn { padding: 12px 20px; font-size: 13px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; color: white; width: 90%; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; transition: 0.3s; }

        .btn:hover { opacity: 0.9; transform: translateY(-1px); }

        .btn:active { transform: translateY(0); }

        input, select { background: #334155; color: white; border: 1px solid #475569; padding: 8px; text-align: center; border-radius: 6px; outline: none; }

        input[type="number"] { width: 70px; }

        select { width: 90%; padding: 10px; border-radius: 8px; cursor: pointer; }

        .label { font-size: 0.7em; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1.5px; }

        .flex-row { display: flex; justify-content: space-around; align-items: center; margin: 10px 0; }

        .debug-on { background: #f59e0b !important; color: #000 !important; }

        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }

        .link-footer:hover { text-decoration: underline; }



        /* Submenu Navigation Styles */

        .menu-btn {

            display: flex;

            align-items: center;

            justify-content: flex-start;

            width: 100%;

            padding: 16px 20px;

            background: rgba(30, 41, 59, 0.7);

            border: 1px solid rgba(255, 255, 255, 0.05);

            border-radius: 12px;

            color: white;

            font-size: 14px;

            font-weight: 600;

            margin: 12px 0;

            cursor: pointer;

            transition: all 0.25s ease;

            text-align: left;

            box-sizing: border-box;

        }

        .menu-btn:hover {

            background: rgba(56, 189, 248, 0.15);

            border-color: #38bdf8;

            transform: translateY(-2px);

            box-shadow: 0 4px 12px rgba(56, 189, 248, 0.15);

        }

        .menu-btn-danger {

            border-color: rgba(239, 68, 68, 0.2);

        }

        .menu-btn-danger:hover {

            background: rgba(239, 68, 68, 0.15);

            border-color: #ef4444;

            box-shadow: 0 4px 12px rgba(239, 68, 68, 0.15);

        }

        .menu-icon {

            font-size: 18px;

            margin-right: 15px;

            display: inline-block;

            width: 24px;

            text-align: center;

        }

        .back-btn {

            background: rgba(255, 255, 255, 0.03);

            border: 1px solid rgba(255, 255, 255, 0.05);

            color: #38bdf8;

            font-size: 12px;

            font-weight: 700;

            cursor: pointer;

            margin-bottom: 15px;

            display: inline-flex;

            align-items: center;

            justify-content: center;

            gap: 8px;

            padding: 8px 16px;

            border-radius: 8px;

            transition: 0.2s;

            text-transform: uppercase;

            letter-spacing: 0.5px;

            align-self: flex-start;

        }

        .back-btn:hover {

            background: rgba(56, 189, 248, 0.1);

            border-color: rgba(56, 189, 248, 0.2);

        }

        .submenu-view {

            display: none;

            flex-direction: column;

            animation: fadeIn 0.3s ease-out;

        }

        @keyframes fadeIn {

            from { opacity: 0; transform: translateY(8px); }

            to { opacity: 1; transform: translateY(0); }

        }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95C4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2 id="configHeader">SETTINGS</h2>

        </div>



        <!-- Main Settings Menu -->

        <div id="mainMenu">

            <button class="menu-btn" onclick="showSubMenu('screenConfig', 'Screen Config')">

                <span class="menu-icon">🖥️</span> Power & Screen

            </button>

            <button class="menu-btn" onclick="showSubMenu('motorConfig', 'Motor Config')">

                <span class="menu-icon">⚙️</span> Motor Config

            </button>

            <button class="menu-btn" onclick="showSubMenu('safetyConfig', 'Safety Config')">

                <span class="menu-icon">🛡️</span> Safety Config

            </button>

            <button class="menu-btn" onclick="showSubMenu('motorHardConfig', 'Motor Hardware')">

                <span class="menu-icon">🔌</span> Motor Hardware Version

            </button>

            <button class="menu-btn" onclick="showSubMenu('reportConfig', 'Report Config')">

                <span class="menu-icon">📄</span> Report Config

            </button>

            <button class="menu-btn" onclick="showSubMenu('securityConfig', 'Security Config')">

                <span class="menu-icon">🔒</span> Security Config

            </button>

            <button class="menu-btn menu-btn-danger" onclick="showSubMenu('factoryResetConfig', 'Factory Reset')">

                <span class="menu-icon">⚠️</span> Reset to Factory Settings

            </button>

            <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>

        </div>



        <!-- Screen & Power Config Submenu -->

        <div id="screenConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Screen Rotation (s)</div>

                <input type="number" id="st" min="1" max="60" style="margin-right: 10px;">

                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('scrTime', document.getElementById('st').value)">SET</button>

            </div>

            <div class="card">

                <div class="label">Auto-Sleep Timeout (mins)</div>

                <input type="number" id="slp" min="10" max="15" style="margin-right: 10px;">

                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('sleepTime', document.getElementById('slp').value)">SET</button>

                <div style="font-size: 0.75em; color: #94a3b8; margin-top: 5px;">Range: 10 to 15 minutes.</div>

            </div>

            <div class="card">

                <div class="label">Surgical LED Brightness (Pin 18)</div>

                <div class="flex-row">

                    <input type="range" min="0" max="100" value="0" class="slider" id="ledCfgSlider" oninput="document.getElementById('ledCfgVal').innerText=this.value+'%'">

                    <span id="ledCfgVal" class="value" style="margin-left: 10px; font-size: 1.1em;">0%</span>

                </div>

                <button class="btn" style="background: #7c3aed; width: auto; margin-top: 10px;" onclick="set('led_pct', document.getElementById('ledCfgSlider').value)">APPLY BRIGHTNESS</button>

            </div>

        </div>



        <!-- Motor Config Submenu -->

        <div id="motorConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Oscillation Time (ms)</div>

                <input type="number" id="ot" min="500" max="10000" step="500" style="margin-right: 10px;">

                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('oscDur', document.getElementById('ot').value)">SAVE</button>

            </div>

            <div class="card">

                <div class="label">Manual Oscillation Durations (ms)</div>

                <div class="flex-row">

                    <div><div class="label">CW</div><input type="number" id="oscCW" step="100"></div>

                    <div><div class="label">CCW</div><input type="number" id="oscCCW" step="100"></div>

                </div>

                <button class="btn" style="background: #7c3aed;" onclick="saveOsc()">SAVE OSCILLATION</button>

            </div>

            <div class="card">

                <div class="label">Graft Sensitivity</div>

                <div class="flex-row">

                    <div><div class="label">Spike (A)</div><input type="number" id="po" step="0.01"></div>

                    <div><div class="label">Hyst (A)</div><input type="number" id="ph" step="0.01"></div>

                </div>

                <div id="speedDipRow" class="flex-row" style="margin-top: 15px; border-top: 1px solid rgba(255,255,255,0.05); padding-top: 15px;">

                    <div><div class="label">Speed Dip (%)</div><input type="number" id="sd" step="0.5" min="1" max="50"></div>

                </div>

                <button class="btn" style="background: #0284c7; margin-top: 10px;" onclick="savePen()">SAVE SENSITIVITY</button>

            </div>

        </div>



        <!-- Safety Config Submenu -->

        <div id="safetyConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Safety Thresholds</div>

                <div class="flex-row">

                    <div><div class="label">Max I (A)</div><input type="number" id="mc" step="0.1"></div>

                    <div><div class="label">Min V (V)</div><input type="number" id="mv" step="0.1"></div>

                </div>

                <button class="btn" style="background: #ea580c;" onclick="saveSafety()">SAVE SAFETY</button>

            </div>

            <div class="card">

                <div class="label">Engineering Mode</div>

                <button id="db" class="btn" style="background: #475569;" onclick="set('debug', '0')">TOGGLE DEBUG</button>

            </div>

        </div>



        <!-- Motor Hardware Version Submenu -->

        <div id="motorHardConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Motor Hardware Version</div>

                <select id="motVer" onchange="set('motVer', this.value)">

                    <option value="1">Version 1: Motor + Encoder</option>

                    <option value="2">Version 2: Motor without Encoder</option>

                </select>

            </div>

        </div>



        <!-- Security Config Submenu -->

        <div id="securityConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Pen WiFi Password (AP Mode)</div>

                <input type="text" id="apPass" minlength="8" placeholder="Enter new password" style="width: 80%; margin-bottom: 10px;">

                <button class="btn" style="background: #0ea5e9;" onclick="saveApPass()">SAVE & RESTART</button>

                <div style="font-size: 0.75em; color: #94a3b8; margin-top: 10px;">Min 8 chars. Device will restart.</div>

            </div>

        </div>



        <!-- Report Config Submenu -->

        <div id="reportConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card">

                <div class="label">Report HTML Template</div>

                <a href="/editor" class="btn" style="text-decoration:none; background: #334155; display: inline-block; width: 85%;">EDIT REPORT TEMPLATE</a>

            </div>

        </div>



        <!-- Reset to Factory Settings Submenu -->

        <div id="factoryResetConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>

            <div class="card" style="border: 1px dashed #ef4444;">

                <div class="label" style="color: #ef4444;">DANGER ZONE</div>

                <div style="font-size: 0.85em; color: #94a3b8; margin: 15px 0; line-height: 1.4;">Warning: This will clear all stored configurations and wireless profiles, reverting the system to original settings.</div>

                <button class="btn" style="background: #ef4444;" onclick="if(confirm('Reset all settings and WiFi to defaults?')) set('resetDefaults', '1')">RESET TO FACTORY</button>

            </div>

        </div>

    </div>

    <script>

        function set(c, v) { fetch(`/control?cmd=${c}&val=${v}`); }

        function saveSafety() { set('maxCurr', document.getElementById('mc').value); set('minVolt', document.getElementById('mv').value); }

        function saveOsc() { set('oscDurCw', document.getElementById('oscCW').value); set('oscDurCcw', document.getElementById('oscCCW').value); }

        function saveApPass() { 

            let p = document.getElementById('apPass').value; 

            if(p.length < 8) { alert('Password must be at least 8 characters'); return; }

            if(confirm('Change pen WiFi password to: ' + p + ' and restart?')) {

                set('apPass', encodeURIComponent(p));

            }

        }

        function savePen() { 

            set('penOff', document.getElementById('po').value); 

            set('penHyst', document.getElementById('ph').value); 

            set('spdDip', document.getElementById('sd').value); 

        }



        function showSubMenu(id, title) {

            document.getElementById('mainMenu').style.display = 'none';

            document.querySelectorAll('.submenu-view').forEach(el => el.style.display = 'none');

            document.getElementById(id).style.display = 'flex';

            document.getElementById('configHeader').innerText = title.toUpperCase();

        }

        function goBack() {

            document.querySelectorAll('.submenu-view').forEach(el => el.style.display = 'none');

            document.getElementById('mainMenu').style.display = 'block';

            document.getElementById('configHeader').innerText = 'SETTINGS';

        }

        

        setInterval(() => {

            fetch('/telemetry').then(r => r.json()).then(data => {

                if (document.activeElement.tagName !== 'INPUT' && document.activeElement.tagName !== 'SELECT') {

                    document.getElementById('st').value = data.st;

                    document.getElementById('slp').value = data.slp;

                    document.getElementById('oscCW').value = data.oscDurCw;

                    document.getElementById('oscCCW').value = data.oscDurCcw;

                    document.getElementById('ot').value = data.oscDur;

                    document.getElementById('mc').value = data.mc;

                    document.getElementById('mv').value = data.mv;

                    document.getElementById('po').value = data.po;

                    document.getElementById('ph').value = data.ph;

                    document.getElementById('sd').value = data.spdDip;

                    document.getElementById('motVer').value = data.motVer;

                    if (document.getElementById('ledCfgSlider')) {

                        let lval = (data.led_pct !== undefined ? data.led_pct : Math.round(data.led * 100 / 255));

                        document.getElementById('ledCfgSlider').value = lval;

                        document.getElementById('ledCfgVal').innerText = lval + '%';

                    }

                }

                

                document.getElementById('speedDipRow').style.display = (data.motVer === 1) ? 'flex' : 'none';



                let b = document.getElementById('db');

                if (data.debug) {

                    b.innerText = "SAFETY DISABLED (ON)";

                    b.classList.add('debug-on');

                } else {

                    b.innerText = "SAFETY ACTIVE (OFF)";

                    b.classList.remove('debug-on');

                }

            });

        }, 1000);

    </script>

</body>

</html>

)=====";



// Template Editor Page

const char EDITOR_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Template Editor</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 800px; margin: auto; padding: 20px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); }

        textarea { width: 95%; height: 400px; background: #0f172a; color: #e2e8f0; border: 1px solid #334155; border-radius: 8px; padding: 10px; font-family: 'Courier New', monospace; font-size: 14px; }

        .btn { padding: 12px 20px; font-size: 15px; margin: 10px 5px; cursor: pointer; border: none; border-radius: 8px; color: white; transition: 0.3s; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; }

        .btn-green { background: #10b981; }

        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }

        input[type="file"] { color: #94a3b8; margin: 10px auto; }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2>TEMPLATE EDITOR</h2>

        </div>

        <div class="card">

            <h3>Report Logo</h3>

            <form method='POST' action='/upload_logo' enctype='multipart/form-data'>

                <input type='file' name='logo'>

                <input type='submit' class="btn btn-green" value='UPLOAD LOGO'>

            </form>

        </div>

        <div class="card">

            <h3>Report HTML Template</h3>

            <form id="templateForm" method='POST' action='/save_template'>

                <textarea id="templateContent" name="content"></textarea>

                <input type='submit' class="btn btn-green" value='SAVE TEMPLATE'>

            </form>

        </div>

        <a href="/config" class="link-footer">&larr; Back to Settings</a>

    </div>

    <script>

        fetch('/template/report.html')

            .then(response => response.text())

            .then(data => {

                document.getElementById('templateContent').value = data;

            });

    </script>

</body>

</html>

)=====";



// Report Page HTML

const char REPORT_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <title>Post-Operative Surgical Report</title>

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <style>

        body { font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif; background-color: #f1f5f9; color: #1e293b; margin: 0; padding: 20px 0; -webkit-print-color-adjust: exact; print-color-adjust: exact; }

        .report-container { max-width: 780px; margin: 10px auto; background: white; padding: 40px; border-radius: 12px; border: 1px solid #e2e8f0; box-shadow: 0 10px 25px rgba(0,0,0,0.05); box-sizing: border-box; position: relative; }

        

        .report-header { display: flex; justify-content: space-between; align-items: center; border-bottom: 3px solid #0f172a; padding-bottom: 20px; margin-bottom: 30px; }

        .header-left { text-align: left; }

        .header-left h1 { margin: 0; font-size: 22px; font-weight: 800; letter-spacing: 1px; color: #0f172a; text-transform: uppercase; }

        .header-left p { margin: 5px 0 0; font-size: 13px; font-weight: 500; color: #64748b; letter-spacing: 0.5px; }

        .header-right { text-align: right; }

        .header-right img { max-width: 140px; max-height: 60px; object-fit: contain; }

        

        .section { margin-bottom: 25px; }

        .section h2 { font-size: 14px; font-weight: 700; color: #0284c7; text-transform: uppercase; letter-spacing: 1.5px; border-bottom: 2px solid #f1f5f9; padding-bottom: 6px; margin-bottom: 15px; text-align: left; }

        

        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }

        .grid-item { background-color: #f8fafc; padding: 12px 18px; border-radius: 8px; border: 1px solid #edf2f7; text-align: left; }

        .grid-item .label { font-weight: 600; color: #64748b; display: block; margin-bottom: 3px; font-size: 11px; text-transform: uppercase; letter-spacing: 0.5px; }

        .grid-item .value { font-size: 14px; font-weight: 700; color: #0f172a; }

        

        .metrics-card { background: linear-gradient(135deg, #0f172a, #1e293b); color: white; padding: 25px; border-radius: 12px; display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin-bottom: 30px; border: 1px solid rgba(255,255,255,0.05); }

        .metric-item { text-align: center; border-right: 1px solid rgba(255,255,255,0.1); }

        .metric-item:last-child { border-right: none; }

        .metric-item .label { font-size: 10px; font-weight: 600; color: #94a3b8; text-transform: uppercase; letter-spacing: 1px; display: block; margin-bottom: 5px; }

        .metric-item .value { font-size: 24px; font-weight: 800; color: #38bdf8; font-family: 'Courier New', Courier, monospace; }

        

        .remarks-section { border: 1px solid #e2e8f0; border-radius: 8px; padding: 15px; background: #fff; margin-bottom: 30px; text-align: left; }

        .remarks-title { font-size: 12px; font-weight: 700; color: #64748b; margin-bottom: 15px; text-transform: uppercase; letter-spacing: 0.5px; }

        .remarks-line { height: 1px; background-color: #cbd5e1; margin-top: 25px; margin-bottom: 5px; width: 100%; }

        

        .signature-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 100px; margin-top: 40px; }

        .signature-box { border-top: 1px solid #cbd5e1; padding-top: 8px; text-align: center; font-size: 12px; font-weight: 600; color: #64748b; text-transform: uppercase; letter-spacing: 0.5px; }

        

        .footer { text-align: center; margin-top: 35px; font-size: 11px; color: #94a3b8; border-top: 1px solid #e2e8f0; padding-top: 15px; }

        

        .print-btn { position: fixed; top: 20px; right: 20px; padding: 12px 24px; background: #0284c7; color: white; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; cursor: pointer; box-shadow: 0 4px 15px rgba(2, 132, 199, 0.4); font-family: sans-serif; transition: 0.3s; z-index: 1000; text-transform: uppercase; letter-spacing: 1px; }

        .print-btn:hover { background: #0369a1; transform: scale(1.02); }

        .print-btn:active { transform: scale(0.98); }

        

        @media print {

            .print-btn { display: none !important; }

            body { background: white; color: black; padding: 0; }

            .report-container { border: none; box-shadow: none; margin: 0; padding: 0; width: 100%; }

            .grid-item { background-color: #f8fafc !important; -webkit-print-color-adjust: exact; print-color-adjust: exact; }

            .metrics-card { background: #0f172a !important; color: white !important; -webkit-print-color-adjust: exact; print-color-adjust: exact; }

            .metrics-card .metric-item .value { color: #38bdf8 !important; }

            .remarks-section { page-break-before: always; }

        }

        

        @page { size: A4; margin: 20mm; }

        

        @media (max-width: 768px) {

            .report-container { margin: 10px; padding: 20px; }

            .grid { grid-template-columns: 1fr; }

            .metrics-card { grid-template-columns: 1fr; gap: 20px; }

            .metric-item { border-right: none; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 15px; }

            .metric-item:last-child { border-bottom: none; padding-bottom: 0; }

            .print-btn { top: auto; bottom: 20px; right: 20px; width: calc(100% - 40px); left: 20px; text-align: center; }

        }

    </style>

</head>

<body>

    <button class="print-btn" onclick="window.print()">&#128438; Print Clinical Report</button>

    

    <div class="report-container">

    <div class="report-header">

        <div class="header-left">

            <h1>Surgical Operative Report</h1>

            <p>EL-BASEET HAIR PEN-V2 &bull; Medical Integration</p>

        </div>

        <div class="header-right">

            <img src="/img/logo.jpg" alt="Clinic Logo">

        </div>

    </div>



        <div class="section">

            <h2>Patient Demographics</h2>

            <div class="grid">

                <div class="grid-item"><span class="label">Patient Name:</span><span class="value">##PATIENT_NAME##</span></div>

                <div class="grid-item"><span class="label">Age / Gender:</span><span class="value">##PATIENT_AGE##</span></div>

                <div class="grid-item"><span class="label">Mobile Number:</span><span class="value">##PATIENT_MOBILE##</span></div>

                <div class="grid-item"><span class="label">Nationality:</span><span class="value">##PATIENT_NATIONALITY##</span></div>

                <div class="grid-item" style="grid-column: 1 / span 2;"><span class="label">Date of Procedure:</span><span class="value">##DATE##</span></div>

            </div>

        </div>



        <div class="section">

            <h2>Surgical Team</h2>

            <div class="grid">

                <div class="grid-item" style="grid-column: 1 / span 2;"><span class="label">Lead Implanting Surgeon:</span><span class="value">##DOCTOR_NAME##</span></div>

            </div>

        </div>



        <div class="section">

            <h2>Procedure Operative Metrics</h2>

            <div class="metrics-card">

                <div class="metric-item">

                    <span class="label">Total Grafts</span>

                    <span id="graftsCount" class="value">##GRAFT_COUNT##</span>

                </div>

                <div class="metric-item">

                    <span class="label">Operative Duration</span>

                    <span id="opDuration" class="value">##OP_TIME##</span>

                </div>

                <div class="metric-item">

                    <span class="label">Total Pauses</span>

                    <span class="value">##PAUSE_COUNT##</span>

                </div>

            </div>

            <div class="grid" style="margin-top: -15px;">

                <div class="grid-item" style="grid-column: 1 / span 2; display: flex; justify-content: space-between; align-items: center; border-left: 4px solid #0284c7;">

                    <span class="label" style="margin: 0; font-size: 11px;">Average Operational Flow Rate:</span>

                    <span id="graftRate" class="value" style="color: #0284c7; font-size: 15px;">Calculating...</span>

                </div>

            </div>

        </div>



        <div class="remarks-section">

            <div class="remarks-title">Clinical Notes & Surgeon Remarks</div>

            <div style="font-size: 12px; color: #64748b; line-height: 1.5; margin-bottom: 25px;">Enter custom observations, post-op instructions, or graft survival notes below:</div>

            <div class="remarks-line"></div>

            <div class="remarks-line"></div>

            <div class="remarks-line"></div>

        </div>



        <div class="signature-grid">

            <div class="signature-box">Surgeon Signature</div>

            <div class="signature-box">Clinical Stamp / Date</div>

        </div>



        <div class="footer">

            <p>This post-operative surgical sheet was automatically generated by the EL-BASEET Hair Pen-V2 integration system.</p>

            <p>&copy; 2026 EL-BASEET Industrial & Medical Solutions</p>

        </div>

    </div>



    <script>

        window.onload = function() {

            try {

                // Parse graft count

                let grafts = parseInt(document.getElementById('graftsCount').innerText) || 0;

                

                // Parse operation duration (e.g. "0h 15m 30s")

                let timeText = document.getElementById('opDuration').innerText;

                let hours = 0, minutes = 0, seconds = 0;

                

                let hMatch = timeText.match(/(\d+)\s*h/);

                let mMatch = timeText.match(/(\d+)\s*m/);

                let sMatch = timeText.match(/(\d+)\s*s/);

                

                if (hMatch) hours = parseInt(hMatch[1]);

                if (mMatch) minutes = parseInt(mMatch[1]);

                if (sMatch) seconds = parseInt(sMatch[1]);

                

                let totalHours = hours + (minutes / 60) + (seconds / 3600);

                

                let rate = 0;

                if (totalHours > 0) {

                    rate = Math.round(grafts / totalHours);

                }

                

                document.getElementById('graftRate').innerText = rate + " grafts / hour";

            } catch (e) {

                document.getElementById('graftRate').innerText = "N/A";

                console.error("Error calculating average graft rate:", e);

            }

        };

    </script>

</body>

</html>

)=====";



// Report Archive Page

const char ARCHIVE_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Report Archive</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 600px; margin: auto; padding: 20px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); }

        ul { list-style: none; padding: 0; }

        li { background: #334155; margin: 8px 0; padding: 10px; border-radius: 8px; display: flex; justify-content: space-between; align-items: center; }

        a { color: #f0f9ff; text-decoration: none; font-size: 1.1em; }

        .del-btn { background: #ef4444; color: white; border: none; padding: 8px 12px; border-radius: 5px; cursor: pointer; }

        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2>REPORT ARCHIVE</h2>

        </div>

        <div class="card">

            <ul>##REPORT_LIST##</ul>

        </div>

        <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>

    </div>

    <script>

        function del(file) {

            if (confirm('Delete report ' + file + '?')) {

                fetch('/control?cmd=delReport&val=' + file).then(() => {

                    window.location.reload();

                });

            }

        }

    </script>

</body>

</html>

)=====";



// New Add Patient Page

const char ADD_PATIENT_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Add Patient</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; }

        .container { max-width: 500px; margin: auto; padding: 20px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); }

        .btn-on { background: linear-gradient(135deg, #10b981, #059669); box-shadow: 0 4px 15px rgba(16, 185, 129, 0.3); padding: 12px 24px; font-size: 14px; margin: 6px; cursor: pointer; border: none; border-radius: 12px; color: white; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; }

        input { width: 90%; padding: 12px; margin: 5px 0 15px 0; background: #334155; color: white; border: 1px solid #475569; border-radius: 8px; outline: none; }

        .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 1.5px; text-align: left; padding-left: 5%;}

        .flex-row { display: flex; justify-content: space-between; gap: 10px; }

        .link-footer { margin-top: 25px; display: block; color: #64748b; text-decoration: none; font-size: 0.85em; }

    </style>

</head>

<body>

    <div class="container">

        <div class="card">

            <h3>Add New Patient</h3>

            <form action="/savepatient" method="POST">

                <div class="label">Patient Name</div><input type="text" name="pName" placeholder="Enter patient's full name" required>

                <div class="flex-row">

                    <div style="width: 48%;"><div class="label">Age</div><input type="text" name="pAge" placeholder="e.g., 35" style="width: 90%;"></div>

                    <div style="width: 48%;"><div class="label">Nationality</div><input type="text" name="pNat" placeholder="e.g., Egyptian" style="width: 90%;"></div>

                </div>

                <div class="label">Patient Mobile</div><input type="text" name="pMob" placeholder="e.g., +201xxxxxxxxx (Optional)">

                <div class="label">Doctor Name</div><input type="text" name="pDoc" placeholder="Enter doctor's name" required>

                <input type="submit" class="btn-on" style="width: 100%; margin-top: 15px;" value="SAVE & SELECT PATIENT">

            </form>

        </div>

        <a href="/patientinfo" class="link-footer">&larr; Cancel and Return to Patient Management</a>

    </div>

</body>

</html>

)=====";



// New Patient Info Page

const char PATIENT_INFO_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - Patient Management</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 400px; margin: auto; padding: 20px; box-sizing: border-box; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }

        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; font-size: 1.3em;}

        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); text-align: center; }

        .btn { padding: 12px 20px; font-size: 13px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; color: white; width: 90%; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; transition: 0.3s; }

        .btn:hover { opacity: 0.9; transform: translateY(-1px); }

        .btn:active { transform: translateY(0); }

        .btn-red { background: #ef4444; }

        input, select { background: #334155; color: white; border: 1px solid #475569; padding: 12px; text-align: center; border-radius: 8px; outline: none; margin-bottom: 15px; width: 90%; box-sizing: border-box;}

        .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1.5px; }

        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }

        .link-footer:hover { text-decoration: underline; }



        /* Submenu Navigation Styles */

        .menu-btn {

            display: flex;

            align-items: center;

            justify-content: flex-start;

            width: 100%;

            padding: 16px 20px;

            background: rgba(30, 41, 59, 0.7);

            border: 1px solid rgba(255, 255, 255, 0.05);

            border-radius: 12px;

            color: white;

            font-size: 14px;

            font-weight: 600;

            margin: 12px 0;

            cursor: pointer;

            transition: all 0.25s ease;

            text-align: left;

            box-sizing: border-box;

        }

        .menu-btn:hover {

            background: rgba(56, 189, 248, 0.15);

            border-color: #38bdf8;

            transform: translateY(-2px);

            box-shadow: 0 4px 12px rgba(56, 189, 248, 0.15);

        }

        .menu-btn-danger {

            border-color: rgba(239, 68, 68, 0.2);

        }

        .menu-btn-danger:hover {

            background: rgba(239, 68, 68, 0.15);

            border-color: #ef4444;

            box-shadow: 0 4px 12px rgba(239, 68, 68, 0.15);

        }

        .menu-icon {

            font-size: 18px;

            margin-right: 15px;

            display: inline-block;

            width: 24px;

            text-align: center;

        }

        .back-btn {

            background: rgba(255, 255, 255, 0.03);

            border: 1px solid rgba(255, 255, 255, 0.05);

            color: #38bdf8;

            font-size: 12px;

            font-weight: 700;

            cursor: pointer;

            margin-bottom: 15px;

            display: inline-flex;

            align-items: center;

            justify-content: center;

            gap: 8px;

            padding: 8px 16px;

            border-radius: 8px;

            transition: 0.2s;

            text-transform: uppercase;

            letter-spacing: 0.5px;

            align-self: flex-start;

        }

        .back-btn:hover {

            background: rgba(56, 189, 248, 0.1);

            border-color: rgba(56, 189, 248, 0.2);

        }

        .submenu-view {

            display: none;

            flex-direction: column;

            animation: fadeIn 0.3s ease-out;

        }

        @keyframes fadeIn {

            from { opacity: 0; transform: translateY(8px); }

            to { opacity: 1; transform: translateY(0); }

        }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2 id="patientHeader">PATIENT MANAGEMENT</h2>

        </div>



        <!-- Main Patient Menu -->

        <div id="mainMenu">

            <a href="/addpatient" class="menu-btn" style="text-decoration: none;">

                <span class="menu-icon">➕</span> Add New Patient

            </a>

            <button class="menu-btn" onclick="showSubMenu('selectPatientConfig', 'Select Patient')">

                <span class="menu-icon">📂</span> Select Existing Patient

            </button>

            <button class="menu-btn menu-btn-danger" onclick="showSubMenu('removePatientConfig', 'Remove Patient')">

                <span class="menu-icon">🗑️</span> Remove Patient

            </button>

            <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>

        </div>



        <!-- Select Patient Submenu -->

        <div id="selectPatientConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Patient Menu</button>

            <div class="card">

                <div class="label">Choose from previously saved patients</div>

                <select id="patientListSelect" name="patientListSelect">##PATIENT_LIST##</select>

                <button class="btn" style="background:#0ea5e9;" onclick="selectPatient()">Select This Patient</button>

            </div>

        </div>



        <!-- Remove Patient Submenu -->

        <div id="removePatientConfig" class="submenu-view">

            <button class="back-btn" onclick="goBack()">&larr; Back to Patient Menu</button>

            <div class="card" style="border: 1px dashed #ef4444;">

                <div class="label" style="color: #ef4444;">Remove Patient</div>

                <div style="font-size: 0.85em; color: #94a3b8; margin: 10px 0 15px 0;">This will permanently delete a patient and all their reports.</div>

                <select id="patientListRemove" name="patientListRemove">##PATIENT_LIST##</select>

                <button class="btn btn-red" onclick="removePatient()">Permanently Remove</button>

            </div>

        </div>



    </div>

    <script>

        function showSubMenu(id, title) {

            document.getElementById('mainMenu').style.display = 'none';

            document.querySelectorAll('.submenu-view').forEach(el => el.style.display = 'none');

            document.getElementById(id).style.display = 'flex';

            document.getElementById('patientHeader').innerText = title.toUpperCase();

        }

        function goBack() {

            document.querySelectorAll('.submenu-view').forEach(el => el.style.display = 'none');

            document.getElementById('mainMenu').style.display = 'block';

            document.getElementById('patientHeader').innerText = 'PATIENT MANAGEMENT';

        }



        function selectPatient() {

            let patientFile = document.getElementById('patientListSelect').value;

            if (!patientFile) return;

            fetch('/selectpatient?file=' + patientFile).then(() => {

                window.location.href = '/';

            });

        }

        function removePatient() {

            let list = document.getElementById('patientListRemove');

            if (!list.value) {

                alert("Please select a patient to remove.");

                return;

            }

            let patientName = list.options[list.selectedIndex].text;



            if (confirm('Are you sure you want to remove ' + patientName + ' and all their associated reports? This cannot be undone.')) {

                fetch('/removepatient?name=' + patientName).then(() => {

                    window.location.reload();

                });

            }

        }

    </script>

</body>

</html>

)=====";



// New Operation Page - Contains all the in-procedure controls

const char OPERATION_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET - In Operation</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 420px; margin: auto; padding: 25px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 30px; }

        .logo-svg { width: 45px; height: 45px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 10px rgba(56, 189, 248, 0.5)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 3px; background: linear-gradient(to right, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.6); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.05); box-shadow: 0 15px 35px rgba(0,0,0,0.4); }

        .btn { padding: 12px 24px; font-size: 14px; margin: 6px; cursor: pointer; border: none; border-radius: 12px; color: white; transition: all 0.3s; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; }

        .btn-on { background: linear-gradient(135deg, #10b981, #059669); box-shadow: 0 4px 15px rgba(16, 185, 129, 0.3); }

        .btn-off { background: linear-gradient(135deg, #ef4444, #dc2626); box-shadow: 0 4px 15px rgba(239, 68, 68, 0.3); }

        .btn-dir { background: linear-gradient(135deg, #0ea5e9, #2563eb); box-shadow: 0 4px 15px rgba(14, 165, 233, 0.3); }

        .btn:active { transform: scale(0.95); }

        .slider { -webkit-appearance: none; width: 100%; height: 8px; border-radius: 5px; background: #334155; outline: none; margin: 25px 0; }

        .slider::-webkit-slider-thumb { -webkit-appearance: none; width: 22px; height: 22px; border-radius: 50%; background: #38bdf8; cursor: pointer; border: 3px solid #0f172a; box-shadow: 0 0 10px rgba(56, 189, 248, 0.5); }

        .flex-row { display: flex; justify-content: space-around; align-items: center; }

        .value { color: #facc15; font-weight: 700; font-size: 1.4em; font-family: 'Courier New', Courier, monospace; }

        .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 1.5px; }

        .alarm { background: rgba(239, 68, 68, 0.2) !important; color: #f87171; border: 1px solid #ef4444 !important; font-weight: bold; display: none; animation: pulse 1s infinite; }

        @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }

    </style>

</head>

<body>

    <div class="container">

        <div id="alarmBox" class="card alarm">SYSTEM HALTED: OVERLOAD</div>

        <div class="card">

            <div class="label">OPERATION CONTROL</div>

            <div id="opControls" class="flex-row">

                <button id="btnPause" class="btn" style="background: #f59e0b;" onclick="sendCmd('pause_op', '1')">PAUSE</button>

                <button id="btnResume" class="btn btn-on" style="display:none;" onclick="sendCmd('resume_op', '1')">RESUME</button>

                <button id="btnStop" class="btn btn-off" onclick="stopOperation()">STOP OPERATION</button>

            </div>

        </div>

        <div class="card">

            <div class="flex-row">

                <div><div class="label">GRAFT COUNTER</div><div id="penCount" class="value" style="font-size: 2.8em; color: #10b981;">0</div></div>

            </div>

            <button class="btn btn-off" style="width: 50%; font-size: 0.7em; padding: 8px; margin-top: 15px; opacity: 0.8;" onclick="sendCmd('resetCnt', '0')">RESET COUNTER</button>

        </div>

        <div class="card">

            <div class="flex-row">

                <div><div class="label">SETPOINT</div><div id="speedVal" class="value">0</div></div>

                <div><div class="label">ACTUAL RPM</div><div id="rpm" class="value">0</div></div>

            </div>

            <input type="range" min="0" max="255" value="200" class="slider" id="speedSlider" oninput="updateSpeed(this.value)">

            <button class="btn btn-on" onclick="sendCmd('power', 'on')">ON</button>

            <button class="btn btn-off" onclick="sendCmd('power', 'off')">OFF</button>

            <button class="btn btn-dir" onclick="sendCmd('dir', 'toggle')">REVERSE</button>

        </div>

        <div class="card">

            <div class="label">OPERATING MODE</div>

            <div class="flex-row">

                <button id="modeNormal" class="btn" style="background: #475569;" onclick="sendCmd('mode', 'normal')">NORMAL</button>

                <button id="modeOsc" class="btn" style="background: #475569;" onclick="sendCmd('mode', 'osc')">OSCILLATE</button>

            </div>

        </div>

        <div class="card">

            <div class="label">SURGICAL ILLUMINATION (PIN 18)</div>

            <div class="flex-row" style="margin-bottom: 8px;">

                <div><div class="label">BRIGHTNESS</div><div id="ledVal" class="value">0%</div></div>

                <div><div class="label">STATUS</div><div id="ledStatus" class="value" style="color: #94a3b8;">OFF</div></div>

            </div>

            <input type="range" min="0" max="100" value="0" class="slider" id="ledSlider" oninput="updateLed(this.value)">

            <div class="flex-row">

                <button class="btn btn-on" style="flex:1; padding: 10px;" onclick="sendLedPower('on')">ON</button>

                <button class="btn btn-off" style="flex:1; padding: 10px;" onclick="sendLedPower('off')">OFF</button>

                <button class="btn" style="flex:1; padding: 10px; background: #f59e0b;" onclick="updateLed(50)">50%</button>

                <button class="btn" style="flex:1; padding: 10px; background: #0ea5e9;" onclick="updateLed(100)">100%</button>

            </div>

        </div>

    </div>

    <script>

        function updateSpeed(val) { document.getElementById('speedVal').innerText = val; sendCmd('speed', val); }

        function updateLed(val) {

            let pwm = Math.round(val * 255 / 100);

            let ledValElem = document.getElementById('ledVal');

            let ledSliderElem = document.getElementById('ledSlider');

            let ledStatusElem = document.getElementById('ledStatus');

            if (ledValElem) ledValElem.innerText = val + '%';

            if (ledSliderElem) ledSliderElem.value = val;

            if (ledStatusElem) {

                ledStatusElem.innerText = (val > 0) ? 'ON' : 'OFF';

                ledStatusElem.style.color = (val > 0) ? '#38bdf8' : '#94a3b8';

            }

            sendCmd('led', pwm);

        }

        function sendLedPower(state) {

            updateLed(state === 'on' ? 100 : 0);

        }

        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }

        function stopOperation() {

            if(confirm('This will stop the operation. Proceed?')) {

                sendCmd('stop_op', '1');

                setTimeout(() => { window.location.href = '/'; }, 500); // Redirect back to main dashboard

            }

        }

        // The rest of the script (telemetry polling) will be added by the compiler from the main page script.

        // This is just a placeholder for the logic. The full script from INDEX_HTML will be used.

    </script>

</body>

</html>

)=====";



// HTML Content with CSS and JS

const char INDEX_HTML[] PROGMEM = R"=====(

<!DOCTYPE html>

<html>

<head>

    <meta charset="UTF-8">

    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>EL-BASEET HAIR PEN-V2</title>

    <style>

        body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }

        .container { max-width: 420px; margin: auto; padding: 25px; }

        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 30px; }

        .logo-svg { width: 45px; height: 45px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 10px rgba(56, 189, 248, 0.5)); }

        @keyframes spin { 100% { transform: rotate(360deg); } }

        h2 { font-weight: 300; letter-spacing: 3px; background: linear-gradient(to right, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin: 0; }

        .card { background: rgba(30, 41, 59, 0.6); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.05); box-shadow: 0 15px 35px rgba(0,0,0,0.4); }

        .btn { padding: 12px 24px; font-size: 14px; margin: 6px; cursor: pointer; border: none; border-radius: 12px; color: white; transition: all 0.3s; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; }

        .btn-on { background: linear-gradient(135deg, #10b981, #059669); box-shadow: 0 4px 15px rgba(16, 185, 129, 0.3); }

        .btn-off { background: linear-gradient(135deg, #ef4444, #dc2626); box-shadow: 0 4px 15px rgba(239, 68, 68, 0.3); }

        .btn-dir { background: linear-gradient(135deg, #0ea5e9, #2563eb); box-shadow: 0 4px 15px rgba(14, 165, 233, 0.3); }

        .btn:active { transform: scale(0.95); }

        .slider { -webkit-appearance: none; width: 100%; height: 8px; border-radius: 5px; background: #334155; outline: none; margin: 25px 0; }

        .slider::-webkit-slider-thumb { -webkit-appearance: none; width: 22px; height: 22px; border-radius: 50%; background: #38bdf8; cursor: pointer; border: 3px solid #0f172a; box-shadow: 0 0 10px rgba(56, 189, 248, 0.5); }

        .telemetry { display: flex; justify-content: space-around; flex-wrap: wrap; }

        .flex-row { display: flex; justify-content: space-around; align-items: center; }

        .value { color: #facc15; font-weight: 700; font-size: 1.4em; font-family: 'Courier New', Courier, monospace; }

        .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 1.5px; }

        .link-footer { margin-top: 25px; display: block; color: #64748b; text-decoration: none; font-size: 0.85em; transition: 0.3s; }

        .link-footer:hover { color: #38bdf8; }

        .nav-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-top: 15px; }

        .alarm { background: rgba(239, 68, 68, 0.2) !important; color: #f87171; border: 1px solid #ef4444 !important; font-weight: bold; display: none; animation: pulse 1s infinite; }

        @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }

    </style>

</head>

<body>

    <div class="container">

        <div class="header-wrap">

            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>

            <h2>EL-BASEET<br><span style="font-size: 0.65em; letter-spacing: 4px; color: #38bdf8;">HAIR PEN-V2</span><br><span style="font-size: 0.35em; letter-spacing: 2px; color: #94a3b8;">SURGICAL ASSISTANT</span></h2>

        </div>

        <div id="alarmBox" class="card alarm">SYSTEM HALTED: OVERLOAD</div>

        <div id="patientCard" class="card" style="display:none;">

            <div class="label">CURRENT PATIENT</div>

            <div id="currentPatient" class="value" style="font-size: 1.2em; color: #38bdf8;"></div>

        </div>

        

        <div class="card">

            <div class="label">OPERATION CONTROL</div>

            <div id="opControls" class="flex-row">

                <button id="btnStart" class="btn btn-on" onclick="startOperation()">START OPERATION</button>

                <button id="btnPause" class="btn" style="background: #f59e0b; display:none;" onclick="sendCmd('pause_op', '1')">PAUSE</button>

                <button id="btnResume" class="btn btn-on" style="display:none;" onclick="sendCmd('resume_op', '1')">RESUME</button>

                <button id="btnStop" class="btn btn-off" style="display:none;" onclick="if(confirm('This will stop the operation. Proceed?')) sendCmd('stop_op', '1')">STOP</button>

                <button id="btnGenReport" class="btn" style="background: #059669; text-decoration: none; display:none;" onclick="generateAndSave()">GENERATE REPORT</button>

            </div>

        </div>

        <div class="card">

            <div class="label">SURGICAL ILLUMINATION (PIN 18)</div>

            <div class="flex-row" style="margin-bottom: 8px;">

                <div><div class="label">BRIGHTNESS</div><div id="ledVal" class="value">0%</div></div>

                <div><div class="label">STATUS</div><div id="ledStatus" class="value" style="color: #94a3b8;">OFF</div></div>

            </div>

            <input type="range" min="0" max="100" value="0" class="slider" id="ledSlider" oninput="updateLed(this.value)" style="margin: 15px 0;">

            <div class="flex-row">

                <button class="btn btn-on" style="flex:1; padding: 10px;" onclick="sendLedPower('on')">ON</button>

                <button class="btn btn-off" style="flex:1; padding: 10px;" onclick="sendLedPower('off')">OFF</button>

                <button class="btn" style="flex:1; padding: 10px; background: #f59e0b;" onclick="updateLed(50)">50%</button>

                <button class="btn" style="flex:1; padding: 10px; background: #0ea5e9;" onclick="updateLed(100)">100%</button>

            </div>

        </div>



        <div class="nav-grid">

            <a href="/patientinfo" class="btn btn-dir" style="text-decoration:none; background: #0ea5e9; grid-column: 1 / span 2;">Patient Management</a>

            <a href="/config" class="btn btn-dir" style="text-decoration:none; background: #334155; box-shadow: none; grid-column: 1 / span 2;">Settings</a>

            <a href="/archive" class="btn btn-dir" style="grid-column: 1 / span 2; text-decoration:none; background: #047857;">REPORT ARCHIVE</a>

            <a href="/readings" class="btn btn-dir" style="grid-column: 1 / span 2; text-decoration:none; background: #7c3aed; box-shadow: 0 4px 15px rgba(124, 58, 237, 0.3);">Diagnostics</a>

        </div>

        <a href="/wifi" class="link-footer">WIFI CONNECTION SETTINGS</a>

        <a href="/help" target="_blank" rel="noopener noreferrer" class="link-footer" style="margin-top: 15px; color: #0ea5e9;">Help & Guidelines</a>

    </div>



    <script>

        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }

        function startOperation() {

            sendCmd('start_op', '1');

            setTimeout(() => { window.location.href = '/operation'; }, 500); // Redirect to the new operation page

        }

        function generateAndSave() {

            fetch('/control?cmd=saveReport&val=1').then(r => r.text()).then(filename => {

                console.log('Report saved as:', filename);

                window.open('/viewreport?file=' + filename, '_blank');

            });

        }

        function resetReport() {

            if (confirm('Are you sure you want to clear all report data? This includes patient info, graft count, time, and pauses.')) sendCmd('resetReport', '1');

        }



        setInterval(() => {

            fetch('/telemetry').then(r => r.json()).then(data => {

                // Disable start button if no patient is selected

                document.getElementById('btnStart').disabled = (data.pName === 'N/A' || data.opState !== 0);



                document.getElementById('alarmBox').style.display = (data.trip) ? 'block' : 'none';                

                

                // Control Logo Spinning Animation based on real-time motor speed

                let logo = document.querySelector('.logo-svg');

                if (data.spd > 10) {

                    let duration = (255 / data.spd) * 1.5; // Speed proportional rotation duration

                    logo.style.animation = `spin ${duration}s linear infinite`;

                } else {

                    logo.style.animation = 'none';

                }



                // Patient Card

                if (data.pName && data.pName !== 'N/A') {

                    document.getElementById('patientCard').style.display = 'block';

                    document.getElementById('currentPatient').innerText = data.pName;

                } else {

                    document.getElementById('patientCard').style.display = 'none';

                }

                // Surgical LED Slider Telemetry Sync
                let ledSlider = document.getElementById('ledSlider');

                if (ledSlider && document.activeElement !== ledSlider) {

                    let ledPct = (data.led_pct !== undefined ? data.led_pct : Math.round(data.led * 100 / 255));

                    ledSlider.value = ledPct;

                    document.getElementById('ledVal').innerText = ledPct + '%';

                    let st = document.getElementById('ledStatus');

                    if (st) {

                        st.innerText = (data.led > 0) ? 'ON' : 'OFF';

                        st.style.color = (data.led > 0) ? '#38bdf8' : '#94a3b8';

                    }

                }



                // Operation State Buttons

                let btnStart = document.getElementById('btnStart');

                let btnPause = document.getElementById('btnPause');

                let btnResume = document.getElementById('btnResume');

                let btnStop = document.getElementById('btnStop');

                let btnGenReport = document.getElementById('btnGenReport');

                if (data.opState === 0) { // IDLE

                    btnStart.style.display = 'inline-block';

                    btnPause.style.display = btnResume.style.display = btnStop.style.display = 'none';

                    // Show Generate Report button only if an operation has been completed (grafts > 0)

                    if (data.cnt > 0) {

                        btnGenReport.style.display = 'inline-block';

                    } else {

                        btnGenReport.style.display = 'none';

                    }

                } else if (data.opState === 1) { // RUNNING

                    btnPause.style.display = 'inline-block';

                    btnStop.style.display = 'inline-block';

                    btnStart.style.display = btnResume.style.display = btnGenReport.style.display = 'none';

                } else { // PAUSED

                    btnResume.style.display = 'inline-block';

                    btnStop.style.display = 'inline-block';

                    btnStart.style.display = btnPause.style.display = btnGenReport.style.display = 'none';

                }

            });

        }, 500);

    </script>

</body>

</html>

)=====";



void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

void handleOperation() { 

    // Before showing the operation page, ensure an operation is actually running.

    // If not, redirect back to the main page.

    if (operationState != RUNNING && operationState != PAUSED) {

        server.sendHeader("Location", "/");

        server.send(302, "text/plain", "No active operation.");

        return;

    }



    // The new operation page needs the same dynamic JS as the main page.

    // We can build the full page here by combining the HTML and the script.

    String page = OPERATION_HTML;

    String script = R"=====(

        setInterval(() => {

            fetch('/telemetry').then(r => r.json()).then(data => {

                document.getElementById('rpm').innerText = data.rpm;

                document.getElementById('penCount').innerText = data.cnt;

                if (document.activeElement.id !== 'speedSlider') {

                    document.getElementById('speedSlider').value = data.set;

                    document.getElementById('speedVal').innerText = data.set;

                }

                let ledSlider = document.getElementById('ledSlider');

                if (ledSlider && document.activeElement !== ledSlider) {

                    let ledPct = (data.led_pct !== undefined ? data.led_pct : Math.round(data.led * 100 / 255));

                    ledSlider.value = ledPct;

                    document.getElementById('ledVal').innerText = ledPct + '%';

                    let st = document.getElementById('ledStatus');

                    if (st) {

                        st.innerText = (data.led > 0) ? 'ON' : 'OFF';

                        st.style.color = (data.led > 0) ? '#38bdf8' : '#94a3b8';

                    }

                }

            });

        }, 200);

    )=====";

    page.replace("</script>", script + "</script>");

    server.send(200, "text/html", page);

}

void handleConfig() { server.send(200, "text/html", CONFIG_HTML); }

void handleReadings() { server.send(200, "text/html", READINGS_HTML); }

void handleSelectPatient();

void handleGetPatientData();

void handlePatientInfo();

void handleAddNewPatient();

void handleEditor();

void handleSaveTemplate();

void handleInitialScanResults();

void handleUpload();

void handleHelp();

void handleArchive();

void handleViewReport();

void handleOperation();



String reportProcessor(const String& var) {

    if (var == "PATIENT_NAME") return patientName;

    if (var == "PATIENT_AGE") return patientAge;

    if (var == "PATIENT_NATIONALITY") return patientNationality;

    if (var == "PATIENT_MOBILE") return patientMobile;

    if (var == "DOCTOR_NAME") return doctorName;

    if (var == "GRAFT_COUNT") return String(penetrationCount);

    if (var == "PAUSE_COUNT") return String(pauseCount);

    if (var == "OP_TIME") {

        unsigned long totalSeconds = operationTimeAccumulator / 1000;

        if (systemEnabled) { // Add current running time if motor is on

            totalSeconds += (millis() - lastTimeCapture) / 1000;

        }

        int hours = totalSeconds / 3600;

        int minutes = (totalSeconds % 3600) / 60;

        int seconds = totalSeconds % 60;

        return String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";

    }

    if (var == "DATE") {

        // Use the date from the server arguments if available (for viewing old reports)

        if (server.hasArg("date")) return server.arg("date");

        // Otherwise, get the current date

        return getFormattedDate();

    }

    return String();

}



String archiveProcessor(const String& var) {

    if (var == "REPORT_LIST") return server.arg("list");

    return String();

}



void handleWifi() {

    String pageContent = WIFI_HTML;

    String statusHtml = "";

    

    if (WiFi.status() == WL_CONNECTED) {

        String currentSSID = WiFi.SSID();

        String currentIP = WiFi.localIP().toString();

        statusHtml = "<div class=\"status-box status-connected\">"

                     "<span>🟢 Connected to: <b>" + currentSSID + "</b><br><span style=\"font-size:0.8em;font-weight:normal;color:#94a3b8;\">IP: " + currentIP + "</span></span>"

                     "</div>";

    } else {

        statusHtml = "<div class=\"status-box status-disconnected\">"

                     "<span>🔴 Not connected to any network (AP Mode)</span>"

                     "</div>";

    }

    

    pageContent.replace("##WIFI_STATUS_BOX##", statusHtml);

    server.send(200, "text/html", pageContent);

}



void handleInitialScanResults() {

    server.send(200, "application/json", initialScanResultsJson);

}



void handleScan() {

    // Trigger the scan state machine in the main loop

    scanState = SCAN_REQUESTED;

    server.send(200, "text/plain", "OK");

}



void handleScanResults() {

    if (scanState != SCAN_COMPLETE) {

        server.send(200, "application/json", "[]"); // Scan not ready

        return;

    }



    int n = WiFi.scanComplete(); // Get the number of networks found

    String json = "[";

    for (int i = 0; i < n; ++i) {

        if (i > 0) json += ",";

        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";

    }

    json += "]";

    server.send(200, "application/json", json);

    WiFi.scanDelete(); // Clear results from memory

    scanState = SCAN_IDLE; // Reset state machine

}



void handleSaveWifi() {

    if (server.hasArg("ssid")) {

        String s = server.arg("ssid");

        String p = server.arg("pass");

        preferences.putString("sta_ssid", s);

        preferences.putString("sta_pass", p);

        

        server.send(200, "text/html", "<html><body style='background:#1a1a1a;color:white;text-align:center;'><h2>Settings Saved!</h2><p>Restarting to connect to: " + s + "</p></body></html>");

        delay(2000);

        ESP.restart();

    } else {

        server.send(400, "text/plain", "Bad Request");

    }

}



void handleTelemetry() {

    String json = "{";

    json += "\"v\":" + String(batteryVoltage, 2) + ",";

    json += "\"v_int\":" + String(internalVoltage, 2) + ",";

    json += "\"chg\":" + String(isCharging ? 1 : 0) + ",";

    json += "\"i\":" + String(batteryCurrent, 2) + ",";

    json += "\"p\":" + String(batteryPower, 2) + ",";

    json += "\"rpm\":" + String(estimatedRPM) + ",";

    json += "\"cnt\":" + String(penetrationCount) + ",";

    json += "\"pct\":" + String(batteryPercent) + ",";

    json += "\"spd\":" + String(abs(motorSpeed)) + ","; // Actual real-time speed

    json += "\"set\":" + String(webBaseSpeed) + ",";    // User-defined setpoint

    json += "\"mc\":" + String(maxCurrentThreshold, 1) + ",";

    json += "\"mv\":" + String(minVoltageThreshold, 1) + ",";

    json += "\"motVer\":" + String(motorVersion) + ",";

    json += "\"st\":" + String(screenSwitchTime / 1000) + ",";

    json += "\"trip\":" + String((safetyTripped || lowBatteryTripped) && !debugMode ? 1 : 0) + ",";

    json += "\"debug\":" + String(debugMode ? 1 : 0) + ",";

    json += "\"oscMode\":" + String(oscillatingMode ? 1 : 0) + ",";

    json += "\"oscDur\":" + String(oscillationDuration) + ",";

    json += "\"oscDurCw\":" + String(oscillationDurationCW) + ",";

    json += "\"oscDurCcw\":" + String(oscillationDurationCCW) + ",";

    json += "\"po\":" + String(penetrationOffset, 2) + ",";

    json += "\"ph\":" + String(penetrationHysteresis, 2) + ",";

    json += "\"spdDip\":" + String(speedDipThresholdPercent, 1) + ",";

    json += "\"baseRPM\":" + String(baselineRPM, 1) + ",";

    json += "\"calib\":" + String(calibrationNeeded ? 1 : 0) + ",";

    json += "\"spikeT\":" + String(baselineCurrent + penetrationOffset, 2) + ",";

    json += "\"exitT\":" + String((baselineCurrent + penetrationOffset) - penetrationHysteresis, 2) + ",";

    json += "\"baseI\":" + String(baselineCurrent, 2) + ",";

    json += "\"led\":" + String(ledBrightness) + ",";

    json += "\"led_pct\":" + String((int)round(ledBrightness * 100.0f / 255.0f)) + ",";

    json += "\"wifi_mode\":\"" + String(WiFi.status() == WL_CONNECTED ? "STA" : "AP") + "\",";

    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";

    json += ",\"opTime\":" + String(operationTimeAccumulator);

    json += ",\"opState\":" + String(operationState);

    json += ",\"pName\":\"" + patientName + "\"";

    json += ",\"pAge\":\"" + patientAge + "\"";

    json += ",\"pNat\":\"" + patientNationality + "\"";

    json += ",\"pMob\":\"" + patientMobile + "\"";

    json += ",\"pDoc\":\"" + doctorName + "\"";

    json += "}";

    server.send(200, "application/json", json);

}



void handleControl() {

    lastInteractionTime = millis();

    String cmd = server.arg("cmd");

    String val = server.arg("val");



    if (cmd == "start_op") {

        if (operationState == IDLE) {

            if (patientName == "N/A") return; // Do not start if no patient is selected

            operationState = RUNNING;

            operationTimeAccumulator = 0;

            pauseCount = 0;

            lastTimeCapture = millis();

            displayWebConfirmation("Operation Started");

            pendingAnimation = ANIM_START;

        }

    }

    if (cmd == "pause_op") {

        if (operationState == RUNNING) {

            operationState = PAUSED;

            operationTimeAccumulator += millis() - lastTimeCapture;

            pauseCount++;

            systemEnabled = false; // Ensure motor is off

            displayWebConfirmation("Operation Paused");

        }

    }

    if (cmd == "resume_op") {

        if (operationState == PAUSED) {

            operationState = RUNNING;

            lastTimeCapture = millis();

            displayWebConfirmation("Operation Resumed");

        }

    }

    if (cmd == "power") {

        if (val == "on") {

            systemEnabled = true;

            if (webBaseSpeed > 0) {

                isPenetrating = false;

                calibrationNeeded = true;

            }

        } else {

            systemEnabled = false;

            baselineCurrent = 0.0; // Reset baseline when motor is turned OFF

            calibrationNeeded = false; // Stop any ongoing calibration

        }

        isPenetrating = false;

        displayWebConfirmation(systemEnabled ? "Motor ON" : "Motor OFF");

    }

    if (cmd == "dir") {

        forwardDirection = !forwardDirection;

        displayWebConfirmation(forwardDirection ? "Direction FWD" : "Direction REV");

    }

    if (cmd == "speed") {

        webBaseSpeed = val.toInt();

        preferences.putUChar("motSpd", webBaseSpeed); // Save to NVM

        isPenetrating = false;

        calibrationNeeded = true; // Trigger calibration after speed change

        displayWebConfirmation("Speed: " + val);

    }

    if (cmd == "maxCurr") {

        maxCurrentThreshold = val.toFloat();

        preferences.putFloat("maxCurr", maxCurrentThreshold);

        displayWebConfirmation("Max I: " + val + "A");

    }

    if (cmd == "minVolt") {

        minVoltageThreshold = val.toFloat();

        preferences.putFloat("minVolt", minVoltageThreshold);

        displayWebConfirmation("Min V: " + val + "V");

    }

    if (cmd == "penOff") {

        penetrationOffset = val.toFloat();

        preferences.putFloat("penOff", penetrationOffset);

        displayWebConfirmation("Offset: " + val + "A");

    }

    if (cmd == "penHyst") {

        penetrationHysteresis = val.toFloat();

        preferences.putFloat("penHyst", penetrationHysteresis);

        displayWebConfirmation("Hyst: " + val + "A");

    }

    if (cmd == "spdDip") {

        speedDipThresholdPercent = val.toFloat();

        preferences.putFloat("spdDip", speedDipThresholdPercent);

        displayWebConfirmation("Speed Dip: " + val + "%");

    }

    if (cmd == "led" || cmd == "led_bri") {

        int b = constrain(val.toInt(), 0, 255);

        setLEDBrightness((uint8_t)b);

        uint8_t pct = round((float)ledBrightness * 100.0f / 255.0f);

        displayWebConfirmation(ledBrightness > 0 ? ("LED: " + String(pct) + "%") : "LED: OFF");

    }

    if (cmd == "led_pct") {

        int p = constrain(val.toInt(), 0, 100);

        uint8_t b = (uint8_t)round(p * 255.0f / 100.0f);

        setLEDBrightness(b);

        displayWebConfirmation(ledBrightness > 0 ? ("LED: " + String(p) + "%") : "LED: OFF");

    }

    if (cmd == "led_power") {

        if (val == "on") {

            setLEDBrightness(255);

            displayWebConfirmation("LED: 100%");

        } else {

            setLEDBrightness(0);

            displayWebConfirmation("LED: OFF");

        }

    }

    if (cmd == "motVer") {

        uint8_t ver = val.toInt();

        setMotorVersion(ver);

        displayWebConfirmation(ver == 1 ? "Premium N20" : "Std Motor");

    }

        if (cmd == "sleepTime") {

        uint32_t mins = val.toInt();

        if (mins >= 10 && mins <= 15) {

            deepSleepTimeoutMs = mins * 60000;

            preferences.putUInt("sleepTime", deepSleepTimeoutMs);

            displayWebConfirmation("Sleep Time Saved");

        }

    }

    if (cmd == "scrTime") {

        screenSwitchTime = val.toInt() * 1000;

        preferences.putUInt("scrTime", screenSwitchTime); // Save to NVM

        displayWebConfirmation("Rotate: " + val + "s");

    }

    if (cmd == "apPass") {

        if (val.length() >= 8) {

            preferences.putString("ap_pass", val);

            displayWebConfirmation("AP Pass Changed!");

            delay(1500);

            ESP.restart();

        }

    }

    if (cmd == "debug") {

        debugMode = !debugMode;

        if (debugMode) {

            safetyTripped = false; // Reset overcurrent latch when enabling debug

        }

        preferences.putBool("debug", debugMode); // Save debug mode state

        displayWebConfirmation(debugMode ? "DEBUG: SAFETY OFF" : "DEBUG: SAFETY ON");

    }

    if (cmd == "resetDefaults") {

        // Reset operational variables

        maxCurrentThreshold = 2.5;

        minVoltageThreshold = 3.2;

        webBaseSpeed = 200;

        screenSwitchTime = 5000;

        penetrationOffset = 0.04;

        penetrationHysteresis = 0.02;

        penetrationCount = 0;

        debugMode = false;

        setLEDBrightness(0);



        // Save defaults to NVM

        preferences.putFloat("maxCurr", 2.5);

        preferences.putFloat("minVolt", 3.2);

        preferences.putUChar("motSpd", 200);

        preferences.putUInt("scrTime", 5000);

        preferences.putFloat("penOff", 0.04);

        preferences.putFloat("penHyst", 0.02);

        preferences.putUInt("pCnt", 0);

        preferences.putBool("debug", false);

        preferences.putUChar("ledBri", 0);

        preferences.putString("sta_ssid", ssid_sta);

        preferences.putString("sta_pass", pass_sta);

        preferences.remove("ap_pass"); // Reset AP password to default



        displayWebConfirmation("Factory Reset...");

        delay(2000);

        ESP.restart();

    }

    if (cmd == "resetCnt") {

        penetrationCount = 0;

        preferences.putUInt("pCnt", 0); // Clear in NVM

        displayWebConfirmation("Counter Reset");

    }

    if (cmd == "stop_op") {

        if (operationState == RUNNING) {

            operationTimeAccumulator += millis() - lastTimeCapture;

        }

        operationState = IDLE;

        systemEnabled = false;

        preferences.putUInt("pCnt", penetrationCount); // Save final count

        displayWebConfirmation("Operation Stopped");

        pendingAnimation = ANIM_STOP;



        // --- Automatically save the report ---

        StaticJsonDocument<512> doc;

        doc["pName"] = patientName;

        doc["pAge"] = patientAge;

        doc["pNat"] = patientNationality;

        doc["pMob"] = patientMobile;

        doc["pDoc"] = doctorName;

        doc["grafts"] = penetrationCount;

        doc["pauses"] = pauseCount;

        doc["opTime"] = operationTimeAccumulator / 1000; // Stored as seconds

        doc["date"] = getFormattedDate();



        String pNameSanitized = patientName;

        pNameSanitized.replace(" ", "_");

        if (pNameSanitized.length() > 15) pNameSanitized = pNameSanitized.substring(0, 15);

        String filename = "/reports/" + pNameSanitized + "_" + String(millis()) + ".json";



        fs::File file = SPIFFS.open(filename, FILE_WRITE);

        if (file) {

            serializeJson(doc, file);

            file.close();

            Serial.println("Report auto-saved on stop: " + filename);

        }

    }

    if (cmd == "saveReport") {

        // 1. Create a JSON document with the report data

        StaticJsonDocument<512> doc;

        doc["pName"] = patientName;

        doc["pAge"] = patientAge;

        doc["pNat"] = patientNationality;

        doc["pMob"] = patientMobile;

        doc["pDoc"] = doctorName;

        doc["grafts"] = penetrationCount;

        doc["pauses"] = pauseCount;



        unsigned long totalSeconds = operationTimeAccumulator / 1000;

        // No need to add current time, stop_op handles the final accumulation

        doc["opTime"] = totalSeconds;



        doc["date"] = getFormattedDate();



        // 2. Generate a unique filename in the /reports directory

        String pNameSanitized = patientName;

        pNameSanitized.replace(" ", "_"); // Sanitize spaces

        if (pNameSanitized.length() > 15) pNameSanitized = pNameSanitized.substring(0, 15);

        String filename = "/reports/" + pNameSanitized + "_" + String(millis()) + ".json";



        // 3. Write the JSON file to SPIFFS

        fs::File file = SPIFFS.open(filename, FILE_WRITE);

        if (!file) {

            displayWebConfirmation("Save Failed!");

            Serial.println("Failed to create report file: " + filename);

            server.send(500, "text/plain", "Save Failed");

        } else {

            serializeJson(doc, file);

            file.close();

            server.send(200, "text/plain", filename.substring(filename.lastIndexOf('/') + 1));

            Serial.println("Report saved to: " + filename);

            pendingAnimation = ANIM_REPORT;

        }

    }

    if (cmd == "delReport") {

        String filename = "/reports/" + val;

        if (SPIFFS.remove(filename)) {

            Serial.println("Deleted report: " + filename);

        } else {

            Serial.println("Failed to delete report: " + filename);

        }

    }

    if (cmd == "mode") {

        oscillatingMode = (val == "osc");

        preferences.putBool("oscMode", oscillatingMode);

        displayWebConfirmation(oscillatingMode ? "Mode: OSC" : "Mode: NORMAL");

    }

    if (cmd == "oscDur") {

        uint32_t newDur = val.toInt();

        if (newDur >= 500 && newDur <= 10000) {

            oscillationDuration = newDur;

            preferences.putUInt("oscDur", oscillationDuration);

            displayWebConfirmation("OSC Time: " + val + "ms");

        }

    }

    if (cmd == "oscDurCw") {

        uint32_t newDur = val.toInt();

        if (newDur >= 500 && newDur <= 10000) {

            oscillationDurationCW = newDur;

            preferences.putUInt("oscDurCw", oscillationDurationCW);

        }

    }

    if (cmd == "oscDurCcw") {

        uint32_t newDur = val.toInt();

        if (newDur >= 500 && newDur <= 10000) {

            oscillationDurationCCW = newDur;

            preferences.putUInt("oscDurCcw", oscillationDurationCCW);

        }

    }

    if (cmd == "calibrate") {

        isPenetrating = false;

        calibrationNeeded = true;

        displayWebConfirmation("Calibrating...");

    }

    if (cmd == "pName") { patientName = val; preferences.putString("pName", val); }

    if (cmd == "pAge") { patientAge = val; preferences.putString("pAge", val); }

    if (cmd == "pNat") { patientNationality = val; preferences.putString("pNat", val); }

    if (cmd == "pDoc") { doctorName = val; preferences.putString("pDoc", val); }

    if (cmd == "resetReport") {

        operationTimeAccumulator = 0;

        pauseCount = 0;

        penetrationCount = 0;

        patientName = "N/A";

        patientAge = "N/A";

        patientNationality = "N/A";

        patientMobile = "N/A";

        doctorName = "N/A";

        preferences.putUInt("pauseCnt", 0);

        preferences.putUInt("pCnt", 0);

        preferences.putString("pName", "N/A");

        preferences.putString("pAge", "N/A");

        // preferences.putString("pMob", "N/A");

        preferences.putString("pDoc", "N/A");

        preferences.putString("pMob", "N/A");

        displayWebConfirmation("Report Data Cleared");

    }



    server.send(200, "text/plain", "OK");

}



void handleArchive() {

    String list = "";

    fs::File root = SPIFFS.open("/reports");

    fs::File file = root.openNextFile();

    while(file){

        String name = file.name();

        if (name.endsWith(".json")) {

            String displayName = name.substring(name.lastIndexOf('/') + 1);

            list += "<li><a href='/viewreport?file=" + displayName + "' target='_blank'>" + displayName + "</a><button class='del-btn' onclick='del(\"" + displayName + "\")'>X</button></li>";

        }

        file = root.openNextFile();

    }

    if (list == "") list = "<p>No reports saved yet.</p>";

    

    String pageContent = ARCHIVE_HTML;

    pageContent.replace("##REPORT_LIST##", list);

    server.send(200, "text/html", pageContent);

} 



// Self-contained Base64 encoding function

String b64_encode(const uint8_t* data, size_t length) {

    const char* b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    String ret;

    ret.reserve((length + 2) / 3 * 4);

    int i = 0;

    int j = 0;

    uint8_t char_array_3[3];

    uint8_t char_array_4[4];



    while (length--) {

        char_array_3[i++] = *(data++);

        if (i == 3) {

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;

            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);

            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++) ret += b64_chars[char_array_4[i]];

            i = 0;

        }

    }

    if (i) {

        for(j = i; j < 3; j++) char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;

        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);

        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++) ret += b64_chars[char_array_4[j]];

        while((i++ < 3)) ret += '=';

    }

    return ret;

}



void handleViewReport() {

    if (!server.hasArg("file")) {

        server.send(400, "text/plain", "Bad Request: Missing file argument.");

        return;

    }

    String filename = "/reports/" + server.arg("file");

    fs::File file = SPIFFS.open(filename, FILE_READ);

    if (!file) {

        server.send(404, "text/plain", "File Not Found.");

        return;

    }



    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();



    if (error) {

        server.send(500, "text/plain", "Failed to parse report data.");

        return;

    }



    // Temporarily populate global vars for the reportProcessor

    String temp_pName = patientName, temp_pAge = patientAge, temp_pNat = patientNationality, temp_pDoc = doctorName, temp_pMob = patientMobile;

    uint32_t temp_pCount = penetrationCount, temp_pauseCount = pauseCount;

    unsigned long temp_opTime = operationTimeAccumulator;



    patientName = doc["pName"].as<String>();

    patientMobile = doc["pMob"].as<String>();

    patientAge = doc["pAge"].as<String>();

    patientNationality = doc["pNat"].as<String>();

    doctorName = doc["pDoc"].as<String>();

    penetrationCount = doc["grafts"].as<uint32_t>();

    pauseCount = doc["pauses"].as<uint32_t>();

    operationTimeAccumulator = doc["opTime"].as<unsigned long>() * 1000; // Stored as seconds

    server.arg("date") = doc["date"].as<String>();

    pendingAnimation = ANIM_REPORT;



    // Read template from SPIFFS

    fs::File templateFile = SPIFFS.open("/template/report.html", "r");

    if (!templateFile) {

        server.send(500, "text/plain", "Report template not found!");

        // Restore live global vars on failure

        patientName = temp_pName; patientAge = temp_pAge; patientNationality = temp_pNat; doctorName = temp_pDoc;

        penetrationCount = temp_pCount; pauseCount = temp_pauseCount; operationTimeAccumulator = temp_opTime;

        return;

    }

    

    // Stream the template file directly to the client, using the reportProcessor

    // to replace placeholders on-the-fly. This is memory efficient.

    // server.streamFile(templateFile, "text/html"); // This does not process placeholders.



    // Read the template into a string, replace placeholders, and send.

    String templateContent = templateFile.readString();

    templateFile.close();



    // Always use the embedded default SVG logo for reliability.

    String defaultLogo = "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxMDAgMTAwIiB3aWR0aD0iMTAwIiBoZWlnaHQ9IjEwMCI+CiAgPCEtLSBCYWNrZ3JvdW5kIGNpcmNsZSBmb3IgaWNvbiBzdHlsZSAtLT4KICA8Y2lyY2xlIGN4PSI1MCIgY3k9IjUwIiByPSI0NiIgZmlsbD0iI2Y0ZjlmOSIgc3Ryb2tlPSIjMGVhNWU5IiBzdHJva2Utd2lkdGg9IjIiLz4KICAKICA8IS0tIEhhaXIgRm9sbGljbGUvU3RyYW5kcyBiZWluZyBwbGFudGVkIC0tPgogIDxwYXRoIGQ9Ik0gMzIsMzIgQyAyOCwyNSAzMiwxNSA0MCwxMiIgZmlsbD0ibm9uZSIgc3Ryb2tlPSIjMWUyOTNiIiBzdHJva2Utd2lkdGg9IjIuNSIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+CiAgPHBhdGggZD0iTSAzNSwzNSBDIDMyLDI4IDM4LDIwIDQ4LDE4IiBmaWxsPSJub25lIiBzdHJva2U9IiMxZTI5M2IiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+CiAgCiAgPCEtLSBQZW4gQm9keSAoQW5nbGVkIGZyb20gdG9wIHJpZ2h0IHRvIGJvdHRvbSBsZWZ0KSAtLT4KICA8IS0tIE1haW4gU2hhZnQgLS0+CiAgPHBhdGggZD0iTSA3MiwyMCBMIDc4LDI2IEwgNDYsNTggTCA0MCw1MiBaIiBmaWxsPSIjY2JkNWUxIiBzdHJva2U9IiM0NzU1NjkiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgogIDwhLS0gTWV0YWxsaWMgUmlkZ2VzIC8gR3JpcCBzZWN0aW9uIC0tPgogIDxwYXRoIGQ9Ik0gNDYsNTggTCA0MCw1MiBMIDM3LDU1IEwgNDMsNjEgWiIgZmlsbD0iIzk0YTNiOCIgc3Ryb2tlPSIjNDc1NTY5IiBzdHJva2Utd2lkdGg9IjIiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KICAKICA8IS0tIFBlbiBUaXAgLyBOZWVkbGUgKENob2kgSW1wbGFudGVyIHN0eWxlIGJldmVsKSAtLT4KICA8Y2lyY2xlIGN4PSI1MCIgY3k9IjUwIiByPSI0NiIgZmlsbD0ibm9uZSIvPgogIDxwYXRoIGQ9Ik0gMzcsNTUgTCAyOSw2MyBMIDMyLDY0IEwgNDMsNjEgWiIgZmlsbD0iI2UyZThmMCIgc3Ryb2tlPSIjNDc1NTY5IiBzdHJva2Utd2lkdGg9IjEuNSIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgogIDwhLS0gVWx0cmEtZmluZSBuZWVkbGUgcG9pbnQgLS0+CiAgPGxpbmUgeDE9IjI5IiB5MT0iNjMiIHgyPSIyNCIgeTI9IjY4IiBzdHJva2U9IiM2NDc0OGIiIHN0cm9rZS13aWR0aD0iMS41IiBzdHJva2UtbGluZWNhcD0icm91bmQiLz4KICAKICA8IS0tIFBsdW5nZXIvQnV0dG9uIGF0IHRoZSB0b3AgLS0+CiAgPHBhdGggZD0iTSA3MiwyMCBMIDc4LDI2IEwgODIsMjIgTCA3NiwxNiBaIiBmaWxsPSIjMGVhNWU5IiBzdHJva2U9IiMwMjg0YzciIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgogIAogIDwhLS0gU2tpbiBMaW5lIC8gVGFyZ2V0IEFyZWEgLS0+CiAgPHBhdGggZD0iTSAxNSw3NSBRIGQ1MCw3MiA4NSw3NSIgZmlsbD0ibm9uZSIgc3Ryb2tlPSIjY2JkNWUxIiBzdHJva2Utd2lkdGg9IjMiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgogIAogIDwhLS0gTmV3bHkgUGxhbnRlZCBIYWlyIG9uIFNraW4gLS0+CiAgPHBhdGggZD0iTSAyMiw3NCBDIDIwLDY4IDI0LDYyIDI2LDU4IiBmaWxsPSJub25lIiBzdHJva2U9IiM0NzU1NjkiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+CiAgPHBhdGggZD0iTSA2MCw3MyBDIDU4LDY2IDY0LDYwIDY4LDU3IiBmaWxsPSJub25lIiBzdHJva2U9IiM2NDc0OGIiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+Cjwvc3ZnPg==";

    templateContent.replace("src=\"/img/logo.jpg\"", "src=\"" + defaultLogo + "\"");



    // Manually replace all placeholders using the processor logic

    templateContent.replace("##PATIENT_NAME##", reportProcessor("PATIENT_NAME"));

    templateContent.replace("##PATIENT_AGE##", reportProcessor("PATIENT_AGE"));

    templateContent.replace("##PATIENT_MOBILE##", reportProcessor("PATIENT_MOBILE"));

    templateContent.replace("##PATIENT_NATIONALITY##", reportProcessor("PATIENT_NATIONALITY"));

    templateContent.replace("##DOCTOR_NAME##", reportProcessor("DOCTOR_NAME"));

    templateContent.replace("##GRAFT_COUNT##", reportProcessor("GRAFT_COUNT"));

    templateContent.replace("##PAUSE_COUNT##", reportProcessor("PAUSE_COUNT"));

    templateContent.replace("##OP_TIME##", reportProcessor("OP_TIME"));

    templateContent.replace("##DATE##", reportProcessor("DATE"));



    server.send(200, "text/html", templateContent);

    // Restore live global vars

    patientName = temp_pName; patientAge = temp_pAge; patientNationality = temp_pNat; doctorName = temp_pDoc; patientMobile = temp_pMob;

    penetrationCount = temp_pCount; pauseCount = temp_pauseCount; operationTimeAccumulator = temp_opTime;

}



void handlePatientInfo() {

    // 1. Scan for unique patient names

    String patientListOptions = "<option value=''>-- Select a Patient --</option>";

    String uniquePatientFiles[50]; // Array to hold one file for each unique patient

    int uniqueCount = 0;



    fs::File root = SPIFFS.open("/reports");

    fs::File file = root.openNextFile();

    while(file && uniqueCount < 50){

        String filename = file.name();

        if (filename.endsWith(".json")) {

            String currentPatientName = filename.substring(0, filename.lastIndexOf('_'));

            bool found = false;

            for (int i = 0; i < uniqueCount; i++) {

                if (uniquePatientFiles[i].startsWith(currentPatientName)) {

                    found = true;

                    break;

                }

            }

            if (!found) {

                uniquePatientFiles[uniqueCount++] = filename;

            }

        }

        file = root.openNextFile();

    }



    // Now, build the options from the unique list

    for (int i = 0; i < uniqueCount; i++) {

        String filename = uniquePatientFiles[i];

        String displayName = filename.substring(filename.lastIndexOf('/') + 1);

        String patientNameForDisplay = displayName.substring(0, displayName.lastIndexOf('_'));

        patientNameForDisplay.replace("_", " ");

        patientListOptions += "<option value='" + displayName + "'>" + patientNameForDisplay + "</option>";

    }



    // 2. Inject the list into the HTML and send

    String pageContent = PATIENT_INFO_HTML;

    pageContent.replace("##PATIENT_LIST##", patientListOptions);

    server.send(200, "text/html", pageContent);

}



void handleAddNewPatient() {

    server.send(200, "text/html", ADD_PATIENT_HTML);

}



void handleSelectPatient() {

    if (!server.hasArg("file")) {

        server.send(400, "text/plain", "Bad Request");

        return;

    }

    String filename = "/reports/" + server.arg("file");

    fs::File file = SPIFFS.open(filename, "r");

    if (!file) { server.send(404, "text/plain", "File Not Found"); return; }



    StaticJsonDocument<512> doc;

    deserializeJson(doc, file);

    file.close();



    patientName = doc["pName"].as<String>();

    patientAge = doc["pAge"].as<String>();

    patientNationality = doc["pNat"].as<String>();

    patientMobile = doc["pMob"].as<String>();

    doctorName = doc["pDoc"].as<String>();

    server.send(200, "text/plain", "OK");

}



void handleEditor() {

    server.send_P(200, "text/html", EDITOR_HTML);

}



void handleSaveTemplate() {

    if (server.hasArg("content")) {

        fs::File templateFile = SPIFFS.open("/template/report.html", "w");

        if (templateFile) {

            templateFile.print(server.arg("content"));

            templateFile.close();

            displayWebConfirmation("Template Saved!");

            server.sendHeader("Location", "/editor");

            server.send(302, "text/plain", "Updated");

        } else {

            server.send(500, "text/plain", "Failed to save template.");

        }

    } else {

        server.send(400, "text/plain", "Bad Request");

    }

}



fs::File uploadFile;



void handleUpload() {

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {

        String filename = "/img/logo.jpg"; // Always overwrite the logo

        uploadFile = SPIFFS.open(filename, "w");

        Serial.println("Upload Start: " + filename + " Size: " + String(upload.totalSize));

    } else if (upload.status == UPLOAD_FILE_WRITE) {

        if (uploadFile) {

            uploadFile.write(upload.buf, upload.currentSize);

            Serial.print(".");

        }

    } else if (upload.status == UPLOAD_FILE_END) {

        if (uploadFile) {

            uploadFile.close();

            Serial.println("\nUpload End. Size: " + String(upload.totalSize));

            displayWebConfirmation("Logo Uploaded!");

            server.sendHeader("Location", "/editor");

            server.send(302, "text/plain", "Uploaded");

        }

    }

}



#include "guideline.h"



void handleHelp() {

    // Serve the guideline book directly from PROGMEM flash memory

    // This avoids missing file issues if the SPIFFS data upload step was skipped

    server.send_P(200, "text/html", GUIDELINE_HTML);

}



void createDefaultTemplate() {

    fs::File templateFile = SPIFFS.open("/template/report.html", "w");

    templateFile.print(REPORT_HTML);

    templateFile.close();

    Serial.println("Default report template created.");

}



void setupWeb() {

    // 1. Setup AP Mode

    String current_ap_pass = preferences.getString("ap_pass", pass_ap);

    WiFi.softAP(ssid_ap, current_ap_pass.c_str());

    Serial.println("AP Mode Active: " + WiFi.softAPIP().toString());



    // 2. Setup STA Mode using saved credentials or defaults

    String s = preferences.getString("sta_ssid", ssid_sta);

    String p = preferences.getString("sta_pass", pass_sta);



    if (s.length() > 0) {

        Serial.println("Connecting to Station: " + s);

        WiFi.begin(s.c_str(), p.c_str());

    }



    if (!SPIFFS.exists("/reports")) {

        SPIFFS.mkdir("/reports");

        Serial.println("Created /reports directory.");

    }

    if (!SPIFFS.exists("/template")) {

        SPIFFS.mkdir("/template");

    }

    if (!SPIFFS.exists("/img")) {

        SPIFFS.mkdir("/img");

    }

    

    // Graceful Template Upgrade: Overwrite old Times New Roman template with the new modern one, preserving non-default modifications

    if (SPIFFS.exists("/template/report.html")) {

        fs::File f = SPIFFS.open("/template/report.html", "r");

        if (f) {

            String content = f.readString();

            f.close();

            if (content.indexOf("Times New Roman") != -1 || content.indexOf("charset=") == -1 || content.indexOf("page-break-before") == -1) {

                createDefaultTemplate();

                Serial.println("Upgraded legacy report template in SPIFFS to modern clinical template with UTF-8.");

            }

        }

    } else {

        createDefaultTemplate();

    }



    // 3. Server Routes

    server.on("/", handleRoot);

    server.on("/config", handleConfig);

    

    // Serve static files for editor

    server.serveStatic("/template", SPIFFS, "/template");

    server.serveStatic("/img", SPIFFS, "/img");

    server.on("/operation", handleOperation);

    server.on("/readings", handleReadings);

    server.on("/selectpatient", handleSelectPatient);

    server.on("/getpatientdata", handleGetPatientData);

    server.on("/patientinfo", handlePatientInfo);

    server.on("/addpatient", handleAddNewPatient);

    server.on("/archive", handleArchive);

    server.on("/editor", handleEditor);

    server.on("/viewreport", handleViewReport);

    server.on("/help", handleHelp);

    server.on("/telemetry", handleTelemetry);

    server.on("/control", handleControl);

    server.on("/wifi", handleWifi);

    server.on("/scan", handleScan);

    server.on("/initialscanresults", handleInitialScanResults);

    server.on("/scanresults", handleScanResults);

    server.on("/savewifi", HTTP_POST, handleSaveWifi);

    server.on("/savepatient", HTTP_POST, []() {

        patientName = server.arg("pName");

        patientAge = server.arg("pAge");

        patientNationality = server.arg("pNat");

        patientMobile = server.arg("pMob");

        doctorName = server.arg("pDoc");

        preferences.putString("pName", patientName);

        preferences.putString("pAge", patientAge);

        preferences.putString("pNat", patientNationality);

        preferences.putString("pMob", patientMobile);

        preferences.putString("pDoc", doctorName);

        

        server.sendHeader("Location", "/");

        server.send(302, "text/plain", "Patient Selected. Returning to Dashboard...");

    });

    server.on("/removepatient", []() {

        if (!server.hasArg("name")) { server.send(400, "text/plain", "Bad Request"); return; } 

        String nameToRemove = server.arg("name");

        nameToRemove.replace(" ", "_");



        fs::File root = SPIFFS.open("/reports");

        fs::File file = root.openNextFile();

        while(file){

            String filename = file.name();

            if (filename.startsWith("/reports/" + nameToRemove + "_")) {

                SPIFFS.remove(filename);

                Serial.println("Removed: " + filename);

            }

            file = root.openNextFile();

        }

        server.send(200, "text/plain", "OK");

    });

    server.on("/save_template", HTTP_POST, handleSaveTemplate);

    // The upload handler needs two arguments: a reply function on success, and the upload handler function

    server.on("/upload_logo", HTTP_POST, []() { server.send(200, "text/plain", "OK"); }, handleUpload); 

    server.begin();

}



void handleGetPatientData() {

    if (!server.hasArg("file")) {

        server.send(400, "text/plain", "Bad Request");

        return;

    }

    String filename = "/reports/" + server.arg("file");

    fs::File file = SPIFFS.open(filename, "r");

    if (!file) {

        server.send(404, "text/plain", "File Not Found");

        return;

    }



    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();



    if (error) {

        server.send(500, "text/plain", "Failed to parse data");

        return;

    }

    String output;

    serializeJson(doc, output);

    server.send(200, "application/json", output);

}



#endif