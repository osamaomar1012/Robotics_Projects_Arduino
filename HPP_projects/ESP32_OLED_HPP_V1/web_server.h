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

// WiFi Credentials - Update these for your network
const char* ssid_sta = "";
const char* pass_sta = "";
const char* ssid_ap  = "HPP_CONTROL_V1";
const char* pass_ap  = "12345678";

WebServer server(80);

// Externs to link with main project globals
extern int16_t motorSpeed;
extern float batteryVoltage, batteryCurrent, batteryPower, maxCurrentThreshold, minVoltageThreshold, baselineCurrent;
extern float penetrationOffset, penetrationHysteresis;
extern bool calibrationNeeded;
extern uint32_t estimatedRPM, penetrationCount;
extern uint8_t batteryPercent, webBaseSpeed; 
extern uint32_t screenSwitchTime;
extern bool systemEnabled, forwardDirection, safetyTripped, lowBatteryTripped, debugMode, isPenetrating, wifiConnecting;
extern Preferences preferences;
extern void displayWebConfirmation(String msg); // Function to display confirmation on OLED

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
    </style>
</head>
<body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>WIFI SETUP</h2>
        </div>
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
        function scan() {
            document.getElementById('networks').innerHTML = "<p>Scanning... please wait.</p>";
            fetch('/scan').then(r => r.json()).then(data => {
                if (data.length === 0) {
                    document.getElementById('networks').innerHTML = "<p>No networks found.</p>";
                    return;
                }
                let html = '<select onchange="document.getElementById(\'ssid\').value=this.value">';
                html += '<option value="">-- Select Network --</option>';
                data.forEach(n => { html += `<option value="${n.ssid}">${n.ssid} (${n.rssi}dBm)</option>`; });
                html += '</select>';
                document.getElementById('networks').innerHTML = html;
            }).catch(err => { document.getElementById('networks').innerHTML = "<p>Scan failed.</p>"; });
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
    </style>
</head>
<body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,11.67 4.53,11.34 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>READINGS</h2>
        </div>
        <div class="card">
            <div><div class="label">VOLTAGE</div><div id="v" class="value">0.0V</div></div>
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
            <div><div class="label">WiFi MODE</div><div id="wifi" class="value">-</div></div>
            <div><div class="label">IP ADDR</div><div id="ip" class="value">-</div></div>
        </div>
        <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>
    </div>
    <script>
        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }
        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                document.getElementById('v').innerText = data.v + 'V';
                document.getElementById('i').innerText = data.i + 'A';
                document.getElementById('p').innerText = data.p + 'W';
                document.getElementById('pct').innerText = data.pct + '%';
                document.getElementById('rpm').innerText = data.rpm;
                document.getElementById('baseI').innerText = data.baseI + 'A';
                document.getElementById('wifi').innerText = data.wifi_mode;
                document.getElementById('ip').innerText = data.ip;
                document.getElementById('calMsg').style.display = data.calib ? 'block' : 'none';
                document.getElementById('calBtn').style.display = data.calib ? 'none' : 'inline-block';
            });
        }, 1000);
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
        .container { max-width: 400px; margin: auto; padding: 20px; }
        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }
        .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }
        @keyframes spin { 100% { transform: rotate(360deg); } }
        h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }
        .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); }
        .btn { padding: 12px 20px; font-size: 13px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; color: white; width: 90%; font-weight: 600; text-transform: uppercase; letter-spacing: 1px; transition: 0.3s; }
        input { width: 70px; background: #334155; color: white; border: 1px solid #475569; padding: 8px; text-align: center; border-radius: 6px; }
        .label { font-size: 0.7em; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1.5px; }
        .flex-row { display: flex; justify-content: space-around; align-items: center; margin: 10px 0; }
        .debug-on { background: #f59e0b !important; color: #000 !important; }
        .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>CONFIG</h2>
        </div>
        <div class="card">
            <div class="label">Screen Rotation (s)</div>
            <input type="number" id="st" min="1" max="60" style="margin-right: 10px;">
            <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('scrTime', document.getElementById('st').value)">SAVE</button>
        </div>
        <div class="card">
            <div class="label">Safety Thresholds</div>
            <div class="flex-row">
                <div><div class="label">Max I (A)</div><input type="number" id="mc" step="0.1"></div>
                <div><div class="label">Min V (V)</div><input type="number" id="mv" step="0.1"></div>
            </div>
            <button class="btn" style="background: #ea580c;" onclick="saveSafety()">SAVE SAFETY</button>
        </div>
        <div class="card">
            <div class="label">Graft Sensitivity</div>
            <div class="flex-row">
                <div><div class="label">Spike (A)</div><input type="number" id="po" step="0.01"></div>
                <div><div class="label">Hyst (A)</div><input type="number" id="ph" step="0.01"></div>
            </div>
            <button class="btn" style="background: #0284c7;" onclick="savePen()">SAVE FACTORS</button>
        </div>
        <div class="card">
            <div class="label">Engineering Mode</div>
            <button id="db" class="btn" style="background: #475569;" onclick="set('debug', '0')">TOGGLE DEBUG</button>
        </div>
        <div class="card" style="border: 1px dashed #ef4444;">
            <div class="label" style="color: #ef4444;">DANGER ZONE</div>
            <button class="btn" style="background: #ef4444;" onclick="if(confirm('Reset all settings and WiFi to defaults?')) set('resetDefaults', '1')">RESET TO FACTORY</button>
        </div>
        <a href="/" class="link-footer">&larr; BACK TO DASHBOARD</a>
    </div>
    <script>
        function set(c, v) { fetch(`/control?cmd=${c}&val=${v}`); }
        function saveSafety() { set('maxCurr', document.getElementById('mc').value); set('minVolt', document.getElementById('mv').value); }
        function savePen() { set('penOff', document.getElementById('po').value); set('penHyst', document.getElementById('ph').value); }
        
        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                if (document.activeElement.tagName !== 'INPUT') {
                    document.getElementById('st').value = data.st;
                    document.getElementById('mc').value = data.mc;
                    document.getElementById('mv').value = data.mv;
                    document.getElementById('po').value = data.po;
                    document.getElementById('ph').value = data.ph;
                }
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

// HTML Content with CSS and JS
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>EL-BASEET HAIR PEN-V1</title>
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
            <h2>EL-BASEET<br><span style="font-size: 0.65em; letter-spacing: 4px; color: #38bdf8;">HAIR PEN-V1</span><br><span style="font-size: 0.35em; letter-spacing: 2px; color: #94a3b8;">SURGICAL ASSISTANT</span></h2>
        </div>
        <div id="alarmBox" class="card alarm">SYSTEM HALTED: OVERLOAD</div>
        
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
        
        <div class="nav-grid">
            <a href="/config" class="btn btn-dir" style="text-decoration:none; background: #334155; box-shadow: none;">SETTINGS</a>
            <a href="/readings" class="btn btn-dir" style="text-decoration:none; background: #7c3aed; box-shadow: 0 4px 15px rgba(124, 58, 237, 0.3);">READINGS</a>
        </div>
        <a href="/wifi" class="link-footer">WIFI CONNECTION SETTINGS</a>
    </div>

    <script>
        function updateSpeed(val) { 
            document.getElementById('speedVal').innerText = val; 
            sendCmd('speed', val); 
        }
        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }

        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                document.getElementById('rpm').innerText = data.rpm;
                document.getElementById('penCount').innerText = data.cnt;
                if (document.activeElement.id !== 'speedSlider') {
                    document.getElementById('speedSlider').value = data.set;
                    document.getElementById('speedVal').innerText = data.set;
                }
                document.getElementById('alarmBox').style.display = (data.trip) ? 'block' : 'none';
            });
        }, 200); // Reduced polling interval to 200ms for faster updates
    </script>
</body>
</html>
)=====";

void handleRoot() { server.send(200, "text/html", INDEX_HTML); }
void handleConfig() { server.send(200, "text/html", CONFIG_HTML); }
void handleReadings() { server.send(200, "text/html", READINGS_HTML); }

void handleWifi() { server.send(200, "text/html", WIFI_HTML); }

void handleScan() {
    // Ensure we are in a mode that allows scanning while AP is active
    WiFi.mode(WIFI_AP_STA);
    WiFi.scanDelete(); // Clear previous scan results from memory
    
    int n = WiFi.scanNetworks(false, false, false, 300); // Synchronous scan
    String json = "[";
    for (int i = 0; i < n && i < 20; ++i) { // Limit to 20 networks for stability
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
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
    json += "\"i\":" + String(batteryCurrent, 2) + ",";
    json += "\"p\":" + String(batteryPower, 2) + ",";
    json += "\"rpm\":" + String(estimatedRPM) + ",";
    json += "\"cnt\":" + String(penetrationCount) + ",";
    json += "\"pct\":" + String(batteryPercent) + ",";
    json += "\"spd\":" + String(abs(motorSpeed)) + ","; // Actual real-time speed
    json += "\"set\":" + String(webBaseSpeed) + ",";    // User-defined setpoint
    json += "\"mc\":" + String(maxCurrentThreshold, 1) + ",";
    json += "\"mv\":" + String(minVoltageThreshold, 1) + ",";
    json += "\"st\":" + String(screenSwitchTime / 1000) + ",";
    json += "\"trip\":" + String((safetyTripped || lowBatteryTripped) && !debugMode ? 1 : 0) + ",";
    json += "\"debug\":" + String(debugMode ? 1 : 0) + ",";
    json += "\"po\":" + String(penetrationOffset, 2) + ",";
    json += "\"ph\":" + String(penetrationHysteresis, 2) + ",";
    json += "\"calib\":" + String(calibrationNeeded ? 1 : 0) + ",";
    json += "\"spikeT\":" + String(baselineCurrent + penetrationOffset, 2) + ",";
    json += "\"exitT\":" + String((baselineCurrent + penetrationOffset) - penetrationHysteresis, 2) + ",";
    json += "\"baseI\":" + String(baselineCurrent, 2) + ",";
    json += "\"wifi_mode\":\"" + String(WiFi.status() == WL_CONNECTED ? "STA" : "AP") + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleControl() {
    String cmd = server.arg("cmd");
    String val = server.arg("val");

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
    if (cmd == "scrTime") {
        screenSwitchTime = val.toInt() * 1000;
        preferences.putUInt("scrTime", screenSwitchTime); // Save to NVM
        displayWebConfirmation("Rotate: " + val + "s");
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

        // Save defaults to NVM
        preferences.putFloat("maxCurr", 2.5);
        preferences.putFloat("minVolt", 3.2);
        preferences.putUChar("motSpd", 200);
        preferences.putUInt("scrTime", 5000);
        preferences.putFloat("penOff", 0.04);
        preferences.putFloat("penHyst", 0.02);
        preferences.putUInt("pCnt", 0);
        preferences.putBool("debug", false);
        preferences.putString("sta_ssid", ssid_sta);
        preferences.putString("sta_pass", pass_sta);

        displayWebConfirmation("Factory Reset...");
        delay(2000);
        ESP.restart();
    }
    if (cmd == "resetCnt") {
        penetrationCount = 0;
        preferences.putUInt("pCnt", 0); // Clear in NVM
        displayWebConfirmation("Counter Reset");
    }
    if (cmd == "calibrate") {
        isPenetrating = false;
        calibrationNeeded = true;
        displayWebConfirmation("Calibrating...");
    }

    server.send(200, "text/plain", "OK");
}

void setupWeb() {
    // 1. Setup AP Mode
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_ap, pass_ap);
    Serial.println("AP Mode Active: " + WiFi.softAPIP().toString());

    // 2. Setup STA Mode using saved credentials or defaults
    String s = preferences.getString("sta_ssid", ssid_sta);
    String p = preferences.getString("sta_pass", pass_sta);

    if (s.length() > 0) {
        Serial.println("Connecting to Station: " + s);
        wifiConnecting = true; // Set flag before attempting connection
        WiFi.begin(s.c_str(), p.c_str());
    }

    // 3. Server Routes
    server.on("/", handleRoot);
    server.on("/config", handleConfig);
    server.on("/readings", handleReadings);
    server.on("/telemetry", handleTelemetry);
    server.on("/control", handleControl);
    server.on("/wifi", handleWifi);
    server.on("/scan", handleScan);
    server.on("/savewifi", HTTP_POST, handleSaveWifi);
    server.begin();
}

#endif