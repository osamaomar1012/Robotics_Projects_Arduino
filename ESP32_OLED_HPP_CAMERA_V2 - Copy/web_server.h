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
#include "esp_camera.h"
#include "BoardConfig.h"
#include "esp_camera.h"

// WiFi Credentials are now managed in BoardConfig.h
const char* ssid_sta = WIFI_SSID_STA;
const char* pass_sta = WIFI_PASS_STA;
const char* ssid_ap  = WIFI_SSID_AP;
const char* pass_ap  = WIFI_PASS_AP;
WebServer server(STREAM_PORT);
volatile float currentFPS = 0.0;
bool cameraActive = false;

WebServer server(80);

// Externs to link with main project globals
extern int16_t motorSpeed;
extern volatile float batteryVoltage, batteryCurrent, batteryPower;
extern volatile float currentFPS;
extern float maxCurrentThreshold, minVoltageThreshold, baselineCurrent; // baselineCurrent is now persisted
extern float penetrationOffset, penetrationHysteresis; 
extern volatile bool calibrationNeeded;
extern volatile uint32_t penetrationCount;
extern uint8_t batteryPercent, webBaseSpeed; 
extern uint32_t screenSwitchTime;
extern volatile bool systemEnabled, forwardDirection, safetyTripped, lowBatteryTripped, debugMode, isPenetrating, wifiConnecting, lampState;
extern unsigned long lampStartTime;
extern bool cameraAvailable;
extern uint32_t lampAutoOffDuration;
extern SemaphoreHandle_t telemetryMutex;
extern volatile bool sensorError; // New: Sensor error flag
extern Preferences preferences;
extern void displayWebConfirmation(String msg); // Function to display confirmation on OLED
extern void setMotorSpeed(int16_t speed);      // Added missing declaration
extern void setSurgicalLightingMode(bool lightOn);
extern void readMAX471(volatile float &voltage, volatile float &current);

volatile bool isStreaming = false; // Priority flag
bool hMirror = false;
bool vFlip = false;

// Centralized Logo Asset
const String EL_BASEET_LOGO_SVG = 
    "<svg class='logo-svg' id='mainLogo' viewBox='0 0 24 24'><path d='M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z'/></svg>";

// WiFi Config Page HTML
static const char WIFI_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>EL-BASEET - WiFi</title>
    <style>
        body { font-family: 'Inter', -apple-system, sans-serif; text-align: center; background: #020617; color: #f8fafc; margin: 0; padding: 15px; line-height: 1.5; }
        .container { max-width: 440px; margin: auto; }
        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 10px; margin: 20px 0; }
        .logo-svg { width: 24px; height: 24px; fill: #0ea5e9; }
        h2 { color: #f8fafc; letter-spacing: 1px; font-size: 1.25rem; font-weight: 600; margin: 0; text-transform: uppercase; }
        .card { background: rgba(30, 41, 59, 0.4); backdrop-filter: blur(12px); padding: 24px; margin: 16px 0; border-radius: 16px; border: 1px solid rgba(255, 255, 255, 0.1); box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1); }
        input, select { width: 100%; box-sizing: border-box; padding: 14px; margin: 12px 0; background: #1e293b; color: white; border: 1px solid #334155; border-radius: 12px; font-size: 16px; }
        .btn { width: 100%; padding: 14px; font-size: 14px; cursor: pointer; border: none; border-radius: 12px; color: white; transition: 0.2s; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; margin: 8px 0; }
        .btn-blue { background: linear-gradient(135deg, #0ea5e9, #0284c7); }
        .btn-green { background: linear-gradient(135deg, #10b981, #059669); }
        .nav-bar { display: flex; justify-content: space-around; background: rgba(15, 23, 42, 0.8); backdrop-filter: blur(10px); padding: 12px; position: sticky; top: 0; z-index: 100; border-bottom: 1px solid rgba(255,255,255,0.05); margin: -15px -15px 10px -15px; }
        .nav-bar a { color: #64748b; text-decoration: none; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; padding: 8px 12px; border-radius: 8px; transition: 0.2s; }
        .nav-bar a.active { color: #38bdf8; background: rgba(56, 189, 248, 0.1); }
        .form-label { text-align: left; display: block; font-size: 12px; color: #94a3b8; font-weight: 600; margin-left: 4px; }
        .link-footer { display: block; margin-top: 20px; color: #64748b; font-size: 12px; text-decoration: none; font-weight: 500; }
    </style>
</head>
<body>
    <div class="nav-bar">
        <a href="/">CAMERA</a>
        <a href="/readings">READINGS</a>
        <a href="/config">CONFIG</a>
        <a href="/wifi" class="active">WIFI</a>
    </div>
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
    body { background: #0f172a; color: #f8fafc; font-family: sans-serif; text-align: center; margin: 0; }
    .container { padding: 20px; }
    .stream-box { width: 100%; max-width: 640px; margin: 20px auto; border-radius: 12px; overflow: hidden; border: 2px solid #38bdf8; background: #000; }
    img { width: 100%; height: auto; display: block; }
    .stats { display: flex; justify-content: center; gap: 20px; font-weight: bold; color: #fbbf24; }
    h1 { font-size: 1.2rem; color: #38bdf8; margin-top: 20px; }
</style>
</head><body>
    <div class="container">
        <div class="header-wrap">
            %LOGO%
            <h2>CONNECTIVITY</h2>
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
        <h1>EL-BASEET HPP_Advance</h1>
        <div class="stream-box"><img src="/stream"></div>
        <div class="stats"><div>FPS: <span id="fps">0</span></div></div>
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
static const char READINGS_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>EL-BASEET - Readings</title>
    <style>
        body { font-family: 'Inter', sans-serif; text-align: center; background: #020617; color: #f8fafc; margin: 0; padding: 15px; }
        .container { max-width: 440px; margin: auto; }
        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 10px; margin: 20px 0; }
        .logo-svg { width: 24px; height: 24px; fill: #0ea5e9; }
        h2 { color: #f8fafc; font-size: 1.25rem; font-weight: 600; margin: 0; letter-spacing: 1px; text-transform: uppercase; }
        .card { background: rgba(30, 41, 59, 0.4); backdrop-filter: blur(12px); padding: 20px; margin: 12px 0; border-radius: 16px; border: 1px solid rgba(255, 255, 255, 0.1); display: flex; justify-content: space-around; align-items: center; }
        .btn { padding: 10px 16px; font-size: 11px; cursor: pointer; border: none; border-radius: 10px; color: white; transition: 0.2s; font-weight: 700; text-transform: uppercase; background: #334155; }
        .value { color: #fbbf24; font-weight: 700; font-size: 1.5rem; font-family: 'JetBrains Mono', monospace; }
        .label { font-size: 10px; color: #94a3b8; margin-bottom: 6px; text-transform: uppercase; letter-spacing: 1px; font-weight: 600; }
        .nav-bar { display: flex; justify-content: space-around; background: rgba(15, 23, 42, 0.8); backdrop-filter: blur(10px); padding: 12px; position: sticky; top: 0; z-index: 100; border-bottom: 1px solid rgba(255,255,255,0.05); margin: -15px -15px 10px -15px; }
        .nav-bar a { color: #64748b; text-decoration: none; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; padding: 8px 12px; border-radius: 8px; transition: 0.2s; }
        .nav-bar a.active { color: #38bdf8; background: rgba(56, 189, 248, 0.1); }
        .diag-row { font-size: 12px; display: flex; gap: 8px; align-items: center; justify-content: center; width: 100%; }
        .diag-val { color: #38bdf8; font-weight: 600; }
    </style>
</head>
<body>
    <div class="nav-bar">
        <a href="/">CAMERA</a>
        <a href="/readings" class="active">READINGS</a>
        <a href="/config">CONFIG</a>
        <a href="/wifi">WIFI</a>
    </div>
    <div class="container">
        <div class="header-wrap">
            %LOGO%
            <h2>READINGS</h2>
        </div>
        <div class="card">
            <div><div class="label">VOLTAGE</div><div id="v" class="value">0.0V</div></div>
            <div><div class="label">CURRENT</div><div id="i" class="value">0.0A</div></div>
            <div><div class="label">POWER</div><div id="p" class="value">0.0W</div></div>
        </div>
        <div class="card">
            <div><div class="label">BATTERY</div><div id="pct" class="value">0%</div></div>
        </div>
        <div class="card" style="flex-direction: column; align-items: center;">
            <div class="label">Memory Diagnostics (Internal / PSRAM)</div>
            <div style="font-size: 0.8em;">Free: <span id="mFree" class="value">0</span> / <span id="pFree" class="value">0</span></div>
            <div style="font-size: 0.8em; color: #ef4444;">Min: <span id="mMin" class="value">0</span> / <span id="pMin" class="value">0</span></div>
        </div>
        <div class="card" style="flex-direction: column; align-items: center;">
            <div><div class="label">BASELINE I</div><div id="baseI" class="value">0.0A</div></div>
            <button id="calBtn" class="btn" style="background: #475569; margin-top: 10px;" onclick="sendCmd('calibrate', 'now')">RE-CALIBRATE</button>
            <div id="calMsg" style="color: #f39c12; display:none; margin-top: 10px;">CALIBRATING...</div>
        </div>
        <div id="sensorErrorMsg" style="color: #ef4444; display:none; margin-top: 15px; font-weight: bold;">SENSOR ERROR: Readings Unreliable!</div>
        <div class="card">
            <div><div class="label">WiFi MODE</div><div id="wifi" class="value">-</div></div>
            <div><div class="label">IP ADDR</div><div id="ip" class="value">-</div></div>
        </div>
    </div>
    <script>
        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }
        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                document.getElementById('v').innerText = data.v + 'V';
                document.getElementById('i').innerText = data.i + 'A';
                document.getElementById('p').innerText = data.p + 'W';
                document.getElementById('pct').innerText = data.pct + '%';
                document.getElementById('baseI').innerText = data.baseI + 'A';
                document.getElementById('wifi').innerText = data.wifi_mode;
                document.getElementById('ip').innerText = data.ip;
                document.getElementById('mFree').innerText = (data.mFree / 1024).toFixed(1) + 'KB';
                document.getElementById('mMin').innerText = (data.mMin / 1024).toFixed(1) + 'KB';
                document.getElementById('pFree').innerText = (data.pFree / 1024).toFixed(1) + 'KB';
                document.getElementById('pMin').innerText = (data.pMin / 1024).toFixed(1) + 'KB';
                document.getElementById('calMsg').style.display = data.calib ? 'block' : 'none';
                document.getElementById('calBtn').style.display = data.calib ? 'none' : 'inline-block';
                document.getElementById('sensorErrorMsg').style.display = data.sensorErr ? 'block' : 'none';
            fetch('/telemetry').then(r => r.json()).then(d => {
                document.getElementById('fps').innerText = d.fps;
            });
        }, 1000);
    </script>
</body>
</html>
)=====";

// Configuration & Safety Page
static const char CONFIG_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>EL-BASEET - Config</title>
    <style>
        body { font-family: 'Inter', sans-serif; text-align: center; background: #020617; color: #f8fafc; margin: 0; padding: 15px; }
        .container { max-width: 440px; margin: auto; }
        .header-wrap { display: flex; align-items: center; justify-content: center; gap: 10px; margin: 20px 0; }
        .logo-svg { width: 24px; height: 24px; fill: #0ea5e9; }
        h2 { color: #f8fafc; font-size: 1.25rem; font-weight: 600; margin: 0; letter-spacing: 1px; text-transform: uppercase; }
        .card { background: rgba(30, 41, 59, 0.4); backdrop-filter: blur(12px); padding: 24px; margin: 12px 0; border-radius: 16px; border: 1px solid rgba(255, 255, 255, 0.1); }
        .btn { padding: 14px; font-size: 13px; cursor: pointer; border: none; border-radius: 12px; color: white; width: 100%; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; transition: 0.2s; margin-top: 10px; }
        input { width: 80px; background: #1e293b; color: #f8fafc; border: 1px solid #334155; padding: 10px; text-align: center; border-radius: 10px; font-weight: 600; }
        .label { font-size: 10px; color: #94a3b8; margin-bottom: 10px; text-transform: uppercase; letter-spacing: 1.5px; font-weight: 600; }
        .flex-row { display: flex; justify-content: space-between; align-items: center; margin: 12px 0; }
        .debug-on { background: linear-gradient(135deg, #f59e0b, #d97706) !important; color: #000 !important; }
        .nav-bar { display: flex; justify-content: space-around; background: rgba(15, 23, 42, 0.8); backdrop-filter: blur(10px); padding: 12px; position: sticky; top: 0; z-index: 100; border-bottom: 1px solid rgba(255,255,255,0.05); margin: -15px -15px 10px -15px; }
        .nav-bar a { color: #64748b; text-decoration: none; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; padding: 8px 12px; border-radius: 8px; transition: 0.2s; }
        .nav-bar a.active { color: #38bdf8; background: rgba(56, 189, 248, 0.1); }
        .section-title { font-size: 11px; color: #38bdf8; text-align: left; margin-bottom: 15px; font-weight: 800; border-bottom: 1px solid rgba(56, 189, 248, 0.2); padding-bottom: 5px; }
    </style>
</head>
<body>
    <div class="nav-bar">
        <a href="/">CAMERA</a>
        <a href="/readings">READINGS</a>
        <a href="/config" class="active">CONFIG</a>
        <a href="/wifi">WIFI</a>
    </div>
    <div class="container">
        <div class="header-wrap">
            %LOGO%
            <h2>CONFIG</h2>
        </div>
        <div class="card">
            <div class="label">Display & Light Timers</div>
            <div class="flex-row">
                <div><div class="label">Rotation (s)</div><input type="number" id="st" min="1" max="60"></div>
                <div id="lmoDiv" style="display:none;"><div class="label">Lamp (s)</div><input type="number" id="lmo" min="5" max="300"></div>
            </div>
            <button class="btn" style="background: #7c3aed;" onclick="saveTimers()">SAVE TIMERS</button>
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
            <div class="label" style="color: #ef4444;">DATA MANAGEMENT</div>
            <button class="btn" style="background: #64748b; margin-bottom:10px;" onclick="set('resetCnt', '0')">RESET GRAFT COUNTER</button>
            <br>
            <div class="label" style="color: #ef4444;">DANGER ZONE</div>
            <button class="btn" style="background: #ef4444;" onclick="if(confirm('Reset all settings and WiFi to defaults?')) set('resetDefaults', '1')">RESET TO FACTORY</button>
        </div>
    </div>
    <script>
        function set(c, v) { fetch(`/control?cmd=${c}&val=${v}`); }
        function saveTimers() { 
            set('scrTime', document.getElementById('st').value);
            if(document.getElementById('lmoDiv').style.display !== 'none') {
                set('lampTime', document.getElementById('lmo').value);
            }
        }
        function saveSafety() { set('maxCurr', document.getElementById('mc').value); set('minVolt', document.getElementById('mv').value); }
        function savePen() { set('penOff', document.getElementById('po').value); set('penHyst', document.getElementById('ph').value); }
        
        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                if (document.activeElement.tagName !== 'INPUT') {
                    document.getElementById('st').value = data.st;
                    if(data.hasCam) {
                        document.getElementById('lmoDiv').style.display = 'block';
                        document.getElementById('lmo').value = data.lmo;
                    }
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
static const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>EL-BASEET Surgical Dash V2.0</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
    body{background:#020617;color:#f8fafc;text-align:center;font-family:'Inter',sans-serif;margin:0;padding:15px;}
    .nav-bar { display: flex; justify-content: space-around; background: rgba(15, 23, 42, 0.8); backdrop-filter: blur(10px); padding: 12px; position: sticky; top: 0; z-index: 100; border-bottom: 1px solid rgba(255,255,255,0.05); margin: -15px -15px 10px -15px; }
    .nav-bar a { color: #64748b; text-decoration: none; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; padding: 8px 12px; border-radius: 8px; transition: 0.2s; }
    .nav-bar a.active { color: #38bdf8; background: rgba(56, 189, 248, 0.1); }
    .vid-clip { width:100%; max-width:640px; margin:10px auto 15px auto; overflow:hidden; border:1px solid rgba(56,189,248,0.4); border-radius:20px; position:relative; background:#000; box-shadow:0 25px 50px -12px rgba(0,0,0,0.5); }
    #videoFeed { width:100%; display:block; transition: transform 0.2s ease; transform-origin: center center; }
    .card { background: rgba(30, 41, 59, 0.4); backdrop-filter: blur(12px); padding: 20px; margin-bottom: 12px; border-radius: 16px; border: 1px solid rgba(255,255,255,0.1); }
    .zoom-label { color:#38bdf8; display:block; margin-bottom:8px; font-size:11px; font-weight:800; text-transform:uppercase; letter-spacing:1px; }
    .vol-label { color:#fbbf24; display:block; margin-bottom:8px; font-size:11px; font-weight:800; text-transform:uppercase; letter-spacing:1px; margin-top:10px; }
    .slider { width:100%; height:6px; border-radius:10px; background:#1e293b; outline:none; margin:15px 0; -webkit-appearance:none; }
    .slider::-webkit-slider-thumb { -webkit-appearance:none; width:22px; height:22px; border-radius:50%; background:#38bdf8; cursor:pointer; border: 4px solid #0f172a; box-shadow: 0 0 10px rgba(56,189,248,0.5); }
    .btn { padding:14px; font-size:11px; margin:4px; cursor:pointer; border:none; border-radius:12px; color:#fff; font-weight:800; text-transform:uppercase; transition:0.2s; letter-spacing:0.5px; }
    .btn-green { background: linear-gradient(135deg, #10b981, #059669); } .btn-red { background: linear-gradient(135deg, #ef4444, #dc2626); } .btn-blue { background: linear-gradient(135deg, #0ea5e9, #0284c7); } .btn-gray { background: #334155; }
    .value { color: #fbbf24; font-weight: 800; font-size: 1.8rem; font-family: 'JetBrains Mono', monospace; }
    .label { font-size: 10px; color: #94a3b8; text-transform: uppercase; margin-bottom:8px; font-weight: 600; letter-spacing: 1px; }
    .flex-row { display: flex; justify-content: space-between; align-items: center; }
    .header-wrap { display: flex; align-items: center; justify-content: center; gap: 10px; margin-bottom: 10px; }
    .logo-svg { width: 24px; height: 24px; fill: #0ea5e9; transition: 0.3s; }
    @keyframes logo-pulse { 0% { transform: scale(1); } 50% { transform: scale(1.4); fill: #10b981; filter: drop-shadow(0 0 10px #10b981); } 100% { transform: scale(1); } }
    .pulse { animation: logo-pulse 0.6s ease-out; }
</style></head><body>
    <div class="nav-bar">
        <a href="/" class="active">CAMERA</a>
        <a href="/readings">READINGS</a>
        <a href="/config">CONFIG</a>
        <a href="/wifi">WIFI</a>
    </div>

    <div class="header-wrap">
        %LOGO%
        <h2>EL-BASEET HAIR PEN V2.0</h2>
    </div>

    <div class="vid-clip">
        <img src="/stream" id="videoFeed">
    </div>

    <div class="card">
        <div class="flex-row">
            <div><div class="label">SPEED</div><div id="speedVal" class="value">0</div></div>
            <div><div class="label">FPS</div><div id="fps" class="value">0</div></div>
            <div>
                <div class="label">GRAFTS</div><div id="cnt" class="value" onclick="if(confirm('Reset counter?')) sendCmd('resetCnt', '0')" style="cursor:pointer;">0</div>
                <div style="font-size:0.6em; color:#94a3b8;">(TAP TO RESET)</div>
            </div>
        </div>
        <input type="range" min="0" max="255" value="200" class="slider" id="speedSlider" oninput="updateSpeed(this.value)">
        <div class="flex-row">
            <button class="btn btn-green" onclick="sendCmd('power', 'on')">START</button>
            <button class="btn btn-red" onclick="sendCmd('power', 'off')">STOP</button>
            <button class="btn btn-blue" onclick="sendCmd('dir', 'toggle')">REV</button>
        </div>
    </div>

    <div class="card">
        <label class="zoom-label">ZOOM: <span id="zVal">1.0</span>x</label>
        <input type="range" id="zoom" min="1" max="3" step="0.1" value="1" class="slider" oninput="applyZoom(this.value)">
        <label class="vol-label">ALARM VOLUME: <span id="vVal">10</span>%</label>
        <input type="range" id="volume" min="0" max="100" value="10" class="slider" oninput="updateVolume(this.value)">
        <div class="flex-row" style="flex-wrap:wrap;">
            <button class="btn btn-blue" style="width:45%" onclick="sendCmd('lamp', 'toggle')">LIGHT</button>
            <button class="btn btn-blue" style="width:45%" onclick="captureImage()">SNAP</button>
            <button class="btn btn-gray" style="width:28%" onclick="sendCmd('flipH', 'toggle')">MIR H</button>
            <button class="btn btn-gray" style="width:28%" onclick="sendCmd('flipV', 'toggle')">FLIP V</button>
            <button class="btn btn-gray" style="width:28%" onclick="resetView()">RST</button>
        </div>
    </div>

    <script>
        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }
        function updateSpeed(val) { 
            document.getElementById('speedVal').innerText = val; 
            sendCmd('speed', val); 
        }
        function applyZoom(val) {
            document.getElementById('zVal').innerText = val;
            document.getElementById('videoFeed').style.transform = `scale(${val})`;
        }
        function captureImage() {
            const link = document.createElement('a');
            link.href = '/capture?t=' + Date.now();
            link.download = 'surgical_snapshot.jpg';
            link.click();
        }
        function resetView() {
            document.getElementById('zoom').value = 1;
            applyZoom(1);
            fetch('/control?cmd=resetView&val=1');
        }
        let graftVolume = 0.1;
        function updateVolume(val) {
            document.getElementById('vVal').innerText = val;
            graftVolume = val / 100;
        }

        let audioCtx = null;
        function playGraftSound() {
            if (graftVolume <= 0) return;
            try {
                if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
                if (audioCtx.state === 'suspended') audioCtx.resume();
                
                const osc = audioCtx.createOscillator();
                const gain = audioCtx.createGain();
                
                osc.type = 'sine';
                osc.frequency.setValueAtTime(880, audioCtx.currentTime); // 880Hz - A5 Note
                gain.gain.setValueAtTime(graftVolume, audioCtx.currentTime);
                gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + 0.2);
                
                osc.connect(gain);
                gain.connect(audioCtx.destination);
                osc.start();
                osc.stop(audioCtx.currentTime + 0.2);
            } catch(e) { console.log('Audio error'); }
        }

        let lastCnt = -1;
        setInterval(() => {
            fetch('/telemetry').then(r => r.json()).then(data => {
                if(document.getElementById('cnt')) {
                    if (lastCnt !== -1 && data.cnt > lastCnt) {
                        const logo = document.getElementById('mainLogo');
                        logo.classList.remove('pulse');
                        void logo.offsetWidth; // Trigger reflow
                        logo.classList.add('pulse');
                        playGraftSound();
                    }
                    document.getElementById('fps').innerText = data.fps;
                    document.getElementById('cnt').innerText = data.cnt;
                    lastCnt = data.cnt;
                }
                if (document.activeElement.id !== 'speedSlider') {
                    document.getElementById('speedSlider').value = data.set;
                    document.getElementById('speedVal').innerText = data.set;
                }
                // Other telemetry updates handled in specific pages
            });
        }, 500);
    </script>
</body></html>)=====";

/**
 * @brief Helper function to inject the common logo into HTML templates.
 */
void sendPageWithLogo(const char* pageTemplate) {
    String html = String(pageTemplate);
    html.replace("%LOGO%", EL_BASEET_LOGO_SVG);
    server.send(200, "text/html", html);
}
void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

void handleRoot() { sendPageWithLogo(INDEX_HTML); }
void handleConfig() { sendPageWithLogo(CONFIG_HTML); }
void handleReadings() { sendPageWithLogo(READINGS_HTML); }
void handleWifi() { sendPageWithLogo(WIFI_HTML); }

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

/**
 * @brief Senior-Level Zero-Fragmentation Telemetry
 */
void handleTelemetry() {
    static char jsonBuffer[600];
    float snapV, snapI;
    
    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        snapV = batteryVoltage;
        snapI = batteryCurrent;
        xSemaphoreGive(telemetryMutex);
    } else {
        server.send(503, "application/json", "{\"err\":\"busy\"}");
        return;
    }

    snprintf(jsonBuffer, sizeof(jsonBuffer),
        "{\"v\":%.2f,\"i\":%.2f,\"fps\":%.1f,\"cnt\":%u,\"pct\":%d,\"spd\":%d,\"trip\":%d,\"calib\":%d,\"sensorErr\":%d}",
        snapV, snapI, currentFPS, penetrationCount, batteryPercent, 
        abs(motorSpeed), (safetyTripped || lowBatteryTripped), 
        calibrationNeeded, sensorError);
    
    server.send(200, "application/json", jsonBuffer);
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"fps\":%.1f}", currentFPS);
    server.send(200, "application/json", buf);
}

/**
 * @brief FreeRTOS Task to handle MJPEG streaming asynchronously.
 */
void mjpegStreamTask(void *pvParameters) {
    WiFiClient *client = (WiFiClient *)pvParameters;
    isStreaming = true; 
    unsigned long lastFPSCheck = millis();
    uint32_t frameCounter = 0;
    
    client->setNoDelay(true); // Disable Nagle's algorithm for surgical low-latency
    client->setTimeout(5);    // Aggressive timeout to prevent stale connections from hanging the core
    unsigned long lastCheck = millis();
    uint32_t frames = 0;

    client->setNoDelay(true);
    client->setTimeout(2);

    while (client->connected()) {
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(10));
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        frameCounter++;
        if (millis() - lastFPSCheck >= 1000) {
            currentFPS = frameCounter;
            frameCounter = 0;
            lastFPSCheck = millis();
        frames++;
        if (millis() - lastCheck >= 1000) {
            currentFPS = frames;
            frames = 0;
            lastCheck = millis();
        }

        // Send frame header
        if (client->printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len) <= 0 ||
            client->write(fb->buf, fb->len) != fb->len ||
            client->print("\r\n") <= 0) {
            esp_camera_fb_return(fb);
            break; // Socket error or disconnect
            break;
        }
        
        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(1)); // Allow Wi-Fi stack to process
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    client->stop();
    delete client; 
    currentFPS = 0;
    isStreaming = false;
    delete client;
    vTaskDelete(NULL);
}

/**
 * @brief Handles MJPEG Streaming
 * Spawns a background task so the web server remains responsive.
 */
void handleStream() {
    if (!cameraAvailable) {
        server.send(503, "text/plain", "Camera Hardware Not Found");
        return;
    }

    if (isStreaming) {
        server.send(429, "text/plain", "Stream Busy");
        return;
    }

    WiFiClient *pClient = new WiFiClient(server.client());
    if (!pClient || !pClient->connected()) {
        if (pClient) delete pClient;
    if (!pClient->connected()) {
        delete pClient;
        return;
    }

    // Send initial HTTP response header
    pClient->print("HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");

    // Spawn the streaming task on Core 0
    xTaskCreatePinnedToCore(mjpegStreamTask, "mjpeg_task", 4096, (void*)pClient, 8, NULL, 0); // Priority 8: High, but allows WiFi stack to breathe
    xTaskCreatePinnedToCore(mjpegStreamTask, "strm", 4096, (void*)pClient, 10, NULL, 0);
}

/**
 * @brief Captures a single high-quality frame.
 */
void handleCapture() {
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        server.send(500, "text/plain", "Sensor Error");
        return;
    }

    // Briefly boost quality (lower number = higher quality)
    int old_q = s->status.quality;
    s->set_quality(s, 5); 
    vTaskDelay(pdMS_TO_TICKS(150)); // Allow sensor to adjust to the new quality/bitrate

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        s->set_quality(s, old_q);
        server.send(503, "text/plain", "Camera Busy");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=capture.jpg");
    server.setContentLength(fb->len);
    server.send(200, "image/jpeg", "");
    server.client().write(fb->buf, fb->len);

    esp_camera_fb_return(fb);
    s->set_quality(s, old_q); // Revert to stream quality
}

void handleControl() {
    String cmd = server.arg("cmd");
    String val = server.arg("val");

    if (cmd == "power") {
        if (val == "on") {
            // Reset latched safety trips when user manually attempts to restart
            if (safetyTripped || lowBatteryTripped) {
                if (batteryVoltage > minVoltageThreshold && batteryCurrent < maxCurrentThreshold) {
                    safetyTripped = false;
                    lowBatteryTripped = false;
                }
            }
            
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
    if (cmd == "lamp") {
        lampState = !lampState;
        if (PIN_LAMP_FLASH != -1) digitalWrite(PIN_LAMP_FLASH, lampState ? HIGH : LOW);
        setSurgicalLightingMode(lampState); // Update sensor settings immediately
        if (lampState) lampStartTime = millis(); // Mark the start time for the safety timer
        displayWebConfirmation(lampState ? "Light ON" : "Light OFF");
    }
    if (cmd == "flipH") {
        hMirror = !hMirror;
        sensor_t * s = esp_camera_sensor_get();
        if (s) s->set_hmirror(s, hMirror);
        displayWebConfirmation(hMirror ? "H-Mirror ON" : "H-Mirror OFF");
    }
    if (cmd == "flipV") {
        vFlip = !vFlip;
        sensor_t * s = esp_camera_sensor_get();
        if (s) s->set_vflip(s, vFlip);
        displayWebConfirmation(vFlip ? "V-Flip ON" : "V-Flip OFF");
    }
    if (cmd == "resetView") {
        hMirror = false;
        vFlip = false;
        sensor_t * s = esp_camera_sensor_get();
        if (s) {
            s->set_hmirror(s, 0);
            s->set_vflip(s, 0);
        }
        displayWebConfirmation("View Reset");
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
    if (cmd == "lampTime") {
        lampAutoOffDuration = val.toInt() * 1000;
        preferences.putUInt("lmoTime", lampAutoOffDuration);
        displayWebConfirmation("Lamp: " + val + "s");
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
        maxCurrentThreshold = DEFAULT_MAX_CURRENT;
        minVoltageThreshold = DEFAULT_MIN_VOLTAGE;
        webBaseSpeed = 200;
        screenSwitchTime = 5000;
        penetrationOffset = DEFAULT_PEN_OFFSET;
        penetrationHysteresis = DEFAULT_PEN_HYST;
        baselineCurrent = 0.0; // Reset baseline current
        penetrationCount = 0;
        debugMode = false;
        lampAutoOffDuration = 30000;

        // Save defaults to NVM
        preferences.putFloat("maxCurr", DEFAULT_MAX_CURRENT);
        preferences.putFloat("minVolt", DEFAULT_MIN_VOLTAGE);
        preferences.putUChar("motSpd", 200);
        preferences.putUInt("scrTime", 5000);
        preferences.putFloat("penOff", DEFAULT_PEN_OFFSET);
        preferences.putFloat("penHyst", DEFAULT_PEN_HYST);
        preferences.putFloat("baseCurr", 0.0); // Clear persisted baseline
        preferences.putUInt("pCnt", 0);
        preferences.putBool("debug", false);
        preferences.putUInt("lmoTime", 30000);
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
        // Instant sensor refresh before triggering the burst
        readMAX471(batteryVoltage, batteryCurrent); 
        calibrationNeeded = true;
        displayWebConfirmation("Reading V/I...");
    }

    server.send(200, "text/plain", "OK");
}

void setupWeb() {
    // Soft-start delay: Prevent simultaneous Camera DMA and WiFi radio current spikes
    delay(500); 

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
    WiFi.softAP(WIFI_SSID_AP, WIFI_PASS_AP);
    server.on("/", handleRoot);
    server.on("/config", handleConfig);
    server.on("/readings", handleReadings);
    server.on("/telemetry", handleTelemetry);
    server.on("/control", handleControl);
    server.on("/wifi", handleWifi);
    server.on("/stream", handleStream);
    server.on("/capture", handleCapture);
    server.on("/scan", handleScan);
    server.on("/savewifi", HTTP_POST, handleSaveWifi);
    server.begin();
    Serial.println("AP IP: " + WiFi.softAPIP().toString());
}

#endif