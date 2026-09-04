#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_camera.h"
#include "BoardConfig.h"
#include <Preferences.h>

WebServer server(STREAM_PORT);
volatile float currentFPS = 0.0;
extern Preferences preferences;
extern uint8_t targetMac[6];
extern volatile bool cameraActive;

// Externs and Prototypes for Surgical Control
extern int16_t motorSpeed;   
extern float batteryVoltage, currentDraw, batteryPower, baselineCurrent;
extern float maxCurrentThreshold, minVoltageThreshold, penetrationOffset, penetrationHysteresis;
extern uint32_t estimatedRPM, penetrationCount;
extern uint8_t webBaseSpeed, batteryPercent;
extern bool systemEnabled, forwardDirection, safetyTripped, lowBatteryTripped, debugMode, isPenetrating, calibrationNeeded, wifiConnecting;
extern const char* ssid_sta;
extern const char* pass_sta;
extern const char* ssid_ap;
extern Preferences preferences;

extern void displayWebConfirmation(String msg);

void setMotorSpeed(int speed);

// WiFi Config Page HTML
const char WIFI_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>EL-BASEET - WiFi</title>
<style>
    body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }
    .container { max-width: 400px; margin: auto; padding: 20px; }
    .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }
    .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }
    @keyframes spin { 100% { transform: rotate(360deg); } }
    h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; margin: 0; }
    .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); }
    input, select { width: 90%; padding: 12px; margin: 10px 0; background: #334155; color: white; border: 1px solid #475569; border-radius: 8px; outline: none; }
    .btn { padding: 12px 20px; font-size: 15px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; color: white; transition: 0.3s; font-weight: 600; text-transform: uppercase; }
    .btn-blue { background: #0284c7; }
    .btn-green { background: #10b981; width: 100%; margin-top: 15px; }
    .link-footer { margin-top: 25px; display: block; color: #94a3b8; text-decoration: none; font-size: 0.85em; }
</style>
</head><body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>WIFI SETUP</h2>
        </div>
        <div class="card">
            <button class="btn btn-blue" onclick="scan()">SCAN NETWORKS</button>
            <div id="networks"></div>
            <form action="/savewifi" method="POST">
                <input type="text" name="ssid" id="ssid" placeholder="Enter SSID" required>
                <input type="password" name="pass" id="pass" placeholder="Enter Password">
                <input type="submit" class="btn btn-green" value="SAVE & RESTART">
            </form>
        </div>
        <a href="/" class="link-footer">&larr; Return to Dashboard</a>
    </div>
    <script>
        function scan() {
            document.getElementById('networks').innerHTML = "<p>Scanning...</p>";
            fetch('/scan').then(r => r.json()).then(data => {
                let html = '<select onchange="document.getElementById(\'ssid\').value=this.value">';
                html += '<option value="">-- Select Network --</option>';
                data.forEach(n => { html += `<option value="${n.ssid}">${n.ssid} (${n.rssi}dBm)</option>`; });
                html += '</select>';
                document.getElementById('networks').innerHTML = html;
            });
        }
    </script>
</body></html>)=====";

// Detailed Readings Page
const char READINGS_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>EL-BASEET - Readings</title>
<style>
    body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }
    .container { max-width: 400px; margin: auto; padding: 20px; }
    .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 25px; }
    .logo-svg { width: 32px; height: 32px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 8px rgba(56, 189, 248, 0.4)); }
    @keyframes spin { 100% { transform: rotate(360deg); } }
    h2 { font-weight: 300; letter-spacing: 2px; color: #38bdf8; text-transform: uppercase; }
    .card { background: rgba(30, 41, 59, 0.7); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 0.05); display: flex; justify-content: space-around; }
    .btn { padding: 8px 15px; font-size: 12px; cursor: pointer; border: none; border-radius: 6px; color: white; transition: 0.3s; font-weight: 600; text-transform: uppercase; }
    .value { color: #10b981; font-weight: 600; font-size: 1.3em; font-family: 'Courier New', Courier, monospace; }
    .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px; }
    .link-footer { margin-top: 25px; display: block; color: #38bdf8; text-decoration: none; font-size: 0.85em; }
</style>
</head><body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>READINGS</h2>
        </div>
        <div class="card">
            <div><div class="label">VOLTAGE</div><div id="v" class="value">0.0V</div></div>
            <div><div class="label">CURRENT</div><div id="i" class="value">0.0A</div></div>
            <div><div class="label">POWER</div><div id="p" class="value">0.0W</div></div>
        </div>
        <div class="card">
            <div><div class="label">BATTERY</div><div id="pct" class="value">0%</div></div>
            <div><div class="label">FPS</div><div id="fps" class="value">0</div></div>
        </div>
        <div class="card">
            <div><div class="label">GRAFTS</div><div id="cnt" class="value">0</div></div>
            <div><div class="label">RPM</div><div id="rpm" class="value">0</div></div>
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
                document.getElementById('fps').innerText = data.fps;
                document.getElementById('cnt').innerText = data.cnt;
                document.getElementById('rpm').innerText = data.rpm;
                document.getElementById('baseI').innerText = data.baseI + 'A';
                document.getElementById('wifi').innerText = data.wifi_mode;
                document.getElementById('ip').innerText = data.ip;
                document.getElementById('calMsg').style.display = data.calib ? 'block' : 'none';
                document.getElementById('calBtn').style.display = data.calib ? 'none' : 'inline-block';
            });
        }, 1000);
    </script>
</body></html>)=====";

// Configuration & Safety Page
const char CONFIG_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
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
</head><body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>CONFIG</h2>
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
</body></html>)=====";

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>EL-BASEET HAIR PEN-V1</title>
<style>
    body { font-family: 'Segoe UI', system-ui, sans-serif; text-align: center; background: radial-gradient(circle at top, #1e293b, #0f172a); color: white; margin: 0; min-height: 100vh; }
    .container { max-width: 420px; margin: auto; padding: 25px; }
    .header-wrap { display: flex; align-items: center; justify-content: center; gap: 15px; margin-bottom: 30px; }
    .logo-svg { width: 45px; height: 45px; fill: #38bdf8; animation: spin 5s linear infinite; filter: drop-shadow(0 0 10px rgba(56, 189, 248, 0.5)); }
    @keyframes spin { 100% { transform: rotate(360deg); } }
    h2 { font-weight: 300; letter-spacing: 3px; background: linear-gradient(to right, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin: 0; }
    .stream-box { width: 100%; margin: 20px 0; border-radius: 20px; overflow: hidden; border: 2px solid #38bdf8; background: #000; box-shadow: 0 0 20px rgba(56, 189, 248, 0.3); }
    img { width: 100%; height: auto; display: block; }
    .card { background: rgba(30, 41, 59, 0.6); backdrop-filter: blur(10px); padding: 20px; margin: 15px 0; border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.05); box-shadow: 0 15px 35px rgba(0,0,0,0.4); }
    .btn { padding: 12px 24px; font-size: 14px; margin: 6px; cursor: pointer; border: none; border-radius: 12px; color: white; transition: all 0.3s; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; }
    .btn-on { background: linear-gradient(135deg, #10b981, #059669); box-shadow: 0 4px 15px rgba(16, 185, 129, 0.3); }
    .btn-off { background: linear-gradient(135deg, #ef4444, #dc2626); box-shadow: 0 4px 15px rgba(239, 68, 68, 0.3); }
    .btn-dir { background: linear-gradient(135deg, #0ea5e9, #2563eb); box-shadow: 0 4px 15px rgba(14, 165, 233, 0.3); }
    .slider { -webkit-appearance: none; width: 100%; height: 8px; border-radius: 5px; background: #334155; outline: none; margin: 25px 0; }
    .slider::-webkit-slider-thumb { -webkit-appearance: none; width: 22px; height: 22px; border-radius: 50%; background: #38bdf8; border: 3px solid #0f172a; }
    .value { color: #10b981; font-weight: 700; font-size: 1.4em; font-family: 'Courier New', Courier, monospace; }
    .label { font-size: 0.75em; color: #94a3b8; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 1.5px; }
    .nav-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-top: 15px; }
    .alarm { background: rgba(239, 68, 68, 0.2) !important; color: #f87171; border: 1px solid #ef4444 !important; font-weight: bold; display: none; animation: pulse 1s infinite; }
    .link-footer { margin-top: 25px; display: block; color: #64748b; text-decoration: none; font-size: 0.85em; transition: 0.3s; }
    .flex-row { display: flex; justify-content: space-around; align-items: center; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
</style>
</head><body>
    <div class="container">
        <div class="header-wrap">
            <svg class="logo-svg" viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.97 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.97 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/></svg>
            <h2>EL-BASEET<br><span style="font-size: 0.65em; letter-spacing: 4px; color: #38bdf8;">HAIR PEN-V1</span><br><span style="font-size: 0.35em; letter-spacing: 2px; color: #94a3b8;">SURGICAL ASSISTANT</span></h2>
        </div>
        <div id="alarmBox" class="card alarm">SYSTEM HALTED: OVERLOAD</div>
        <div class="stream-box"><img src="/stream"></div>
        
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
            <a href="/config" class="btn btn-dir" style="text-decoration:none; background: #334155;">SETTINGS</a>
            <a href="/readings" class="btn btn-dir" style="text-decoration:none; background: #7c3aed;">READINGS</a>
        </div>
        <a href="/wifi" class="link-footer">WIFI CONNECTION SETTINGS</a>
    </div>
    <script>
        function updateSpeed(val) { document.getElementById('speedVal').innerText = val; sendCmd('speed', val); }
        function sendCmd(cmd, val) { fetch(`/control?cmd=${cmd}&val=${val}`); }
        let isFetching = false;
        setInterval(() => {
            if(isFetching) return;
            isFetching = true;
            fetch('/telemetry').then(r => r.json()).then(d => {
                document.getElementById('rpm').innerText = d.rpm;
                document.getElementById('penCount').innerText = d.cnt;
                if (document.activeElement.id !== 'speedSlider') {
                    document.getElementById('speedSlider').value = d.set;
                    document.getElementById('speedVal').innerText = d.set;
                }
                document.getElementById('alarmBox').style.display = d.trip ? 'block' : 'none';
            }).finally(() => { isFetching = false; });
        }, 1000); // Increased to 1000ms to prevent CPU starvation
    </script>
</body></html>)=====";

void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }
void handleConfig() { server.send_P(200, "text/html", CONFIG_HTML); }
void handleReadings() { server.send_P(200, "text/html", READINGS_HTML); }
void handleWifi() { server.send_P(200, "text/html", WIFI_HTML); }

void handleTelemetry() {
    // Dynamic allocation of 512 bytes on stack can be risky; using String for small JSON
    String json = "{";
    json += "\"v\":" + String(batteryVoltage, 2) + ",";
    json += "\"i\":" + String(currentDraw, 2) + ",";
    json += "\"p\":" + String(batteryPower, 2) + ",";
    json += "\"rpm\":" + String(estimatedRPM) + ",";
    json += "\"cnt\":" + String(penetrationCount) + ",";
    json += "\"pct\":" + String(batteryPercent) + ",";
    json += "\"spd\":" + String(abs(motorSpeed)) + ",";
    json += "\"set\":" + String(webBaseSpeed) + ",";
    json += "\"mc\":" + String(maxCurrentThreshold, 1) + ",";
    json += "\"mv\":" + String(minVoltageThreshold, 1) + ",";
    json += "\"trip\":" + String(((safetyTripped || lowBatteryTripped) && !debugMode) ? 1 : 0) + ",";
    json += "\"debug\":" + String(debugMode ? 1 : 0) + ",";
    json += "\"po\":" + String(penetrationOffset, 2) + ",";
    json += "\"ph\":" + String(penetrationHysteresis, 2) + ",";
    json += "\"calib\":" + String(calibrationNeeded ? 1 : 0) + ",";
    json += "\"baseI\":" + String(baselineCurrent, 2) + ",";
    json += "\"wifi_mode\":\"" + String(WiFi.status() == WL_CONNECTED ? "STA" : "AP") + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"fps\":" + String(currentFPS, 1);
    json += "}";
    server.send(200, "application/json", json);
}

void handleControl() {
    String cmd = server.arg("cmd");
    String val = server.arg("val");

    if (cmd == "power") {
        if (val == "on") {
            systemEnabled = true;
            safetyTripped = false; 
            lowBatteryTripped = false;
            if (webBaseSpeed > 0) {
                isPenetrating = false;
                calibrationNeeded = true;
            }
        } else {
            systemEnabled = false;
            baselineCurrent = 0.0;
            calibrationNeeded = false;
        }
        isPenetrating = false;
        displayWebConfirmation(systemEnabled ? "Motor ON" : "Motor OFF");
    }
    if (cmd == "dir") {
        forwardDirection = !forwardDirection;
        displayWebConfirmation(forwardDirection ? "FWD" : "REV");
    }
    if (cmd == "resetDefaults") {
        maxCurrentThreshold = 2.5;
        minVoltageThreshold = 3.2;
        webBaseSpeed = 200;
        penetrationOffset = 0.04;
        penetrationHysteresis = 0.02;
        penetrationCount = 0;
        debugMode = false;
        preferences.putFloat("maxCurr", 2.5);
        preferences.putFloat("minVolt", 3.2);
        preferences.putUChar("motSpd", 200);
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
        preferences.putUInt("pCnt", 0); 
        displayWebConfirmation("Counter Reset");
    }
    if (cmd == "calibrate") {
        isPenetrating = false;
        calibrationNeeded = true;
        displayWebConfirmation("Calibrating...");
    }
    if (cmd == "speed") {
        webBaseSpeed = val.toInt();
        preferences.putUChar("motSpd", webBaseSpeed);
        calibrationNeeded = true;
        displayWebConfirmation("Spd: " + val);
    }
    if (cmd == "maxCurr") {
        maxCurrentThreshold = val.toFloat();
        preferences.putFloat("maxCurr", maxCurrentThreshold);
    }
    if (cmd == "minVolt") {
        minVoltageThreshold = val.toFloat();
        preferences.putFloat("minVolt", minVoltageThreshold);
    }
    if (cmd == "penOff") {
        penetrationOffset = val.toFloat();
        preferences.putFloat("penOff", penetrationOffset);
    }
    if (cmd == "penHyst") {
        penetrationHysteresis = val.toFloat();
        preferences.putFloat("penHyst", penetrationHysteresis);
    }
    if (cmd == "debug") {
        debugMode = !debugMode;
        if (debugMode) safetyTripped = false;
        preferences.putBool("debug", debugMode);
    }

    server.send(200, "text/plain", "OK");
}

void handleScan() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n && i < 15; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handleSaveWifi() {
    if (server.hasArg("ssid")) {
        preferences.putString("sta_ssid", server.arg("ssid"));
        preferences.putString("sta_pass", server.arg("pass"));
        server.send(200, "text/html", "Settings Saved. Restarting...");
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Missing SSID");
    }
}

void mjpegStreamTask(void *pvParameters) {
    WiFiClient *client = (WiFiClient *)pvParameters;
    unsigned long lastCheck = millis();
    uint32_t frames = 0;

    client->setNoDelay(true);
    client->setTimeout(5); // Slight increase to allow for network retries

    while (client->connected()) {
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        frames++;
        if (millis() - lastCheck >= 1000) {
            currentFPS = frames;
            frames = 0;
            lastCheck = millis();
        }

        if (client->printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len) <= 0 ||
            client->write(fb->buf, fb->len) != fb->len ||
            client->print("\r\n") <= 0) {
            esp_camera_fb_return(fb);
            break;
        }
        esp_camera_fb_return(fb);
        // Minimize delay to maximize FPS
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    client->stop();
    delete client;
    vTaskDelete(NULL);
}

void handleStream() {
    if (!cameraActive) {
        server.send(503, "text/plain", "Camera Module Not Active");
        return;
    }

    WiFiClient *pClient = new WiFiClient(server.client());
    if (!pClient->connected()) {
        delete pClient;
        return;
    }
    pClient->print("HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");
    // Priority 6 allows WiFi stack (18-20) to breathe while keeping stream responsive
    xTaskCreatePinnedToCore(mjpegStreamTask, "strm", 4096, (void*)pClient, 6, NULL, 0); 
}

volatile bool frameRequested = false;
volatile bool switchResRequested = false;
volatile bool zoomRequested = false;

// Callback to handle the "Pull" request from the CrowPanel
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void onEspNowRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    const uint8_t* mac = info->src_addr;
#else
void onEspNowRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
    Serial.printf("ESP-NOW Recv len %d, cmd 0x%02X\n", len, data[0]);
    if (len == 1) {
        if (data[0] == 0x01) switchResRequested = true;
        else if (data[0] == 0x02) frameRequested = true;
        else if (data[0] == 0x03) zoomRequested = true;
        else if (data[0] == 0x05) {
            static bool flashState = false;
            if (CAM_PIN_FLASH != -1) {
                flashState = !flashState;
                pinMode(CAM_PIN_FLASH, OUTPUT);
                digitalWrite(CAM_PIN_FLASH, flashState ? HIGH : LOW);
            }
        }
    } else if (len == 2 && data[0] == 0x04) {
        sensor_t * s = esp_camera_sensor_get();
        if (s) s->set_brightness(s, (int8_t)map(data[1], 0, 100, -2, 2));
    }
}

void espNowStreamTask(void *pvParameters) {
    uint8_t packetData[250];
    while (1) {
        // Handle resolution change request
        if (switchResRequested) {
            switchResRequested = false;
            sensor_t * s = esp_camera_sensor_get();
            if (s) {
                // Cycle between QVGA (320x240) and CIF (400x296)
                framesize_t nextRes = (s->status.framesize == FRAMESIZE_QVGA) ? FRAMESIZE_CIF : FRAMESIZE_QVGA;
                s->set_framesize(s, nextRes);
                Serial.printf("Resolution changed to: %s\n", (nextRes == FRAMESIZE_CIF) ? "CIF" : "QVGA");
            }
            vTaskDelay(pdMS_TO_TICKS(200)); // Give sensor time to reset
        }

        // Handle digital zoom request
        if (zoomRequested) {
            zoomRequested = false;
            sensor_t * s = esp_camera_sensor_get();
            if (s) {
                static bool isZoomed = false;
                isZoomed = !isZoomed;
                // OV2640 Hack: Using set_special_effect or binning to simulate zoom/focus
                s->set_special_effect(s, isZoomed ? 2 : 0); // 2 = Grayscale/Retro, used here as visual feedback
            }
        }

        if (!cameraActive || !frameRequested) { 
            vTaskDelay(pdMS_TO_TICKS(1)); 
            continue; 
        }

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        frameRequested = false; // Reset request flag
        size_t offset = 0;
        uint16_t chunkSeq = 0;

        while (offset < fb->len) {
            size_t chunkSize = (fb->len - offset) > 248 ? 248 : (fb->len - offset);
            
            // Metadata: Sequence Number (2 bytes)
            packetData[0] = chunkSeq & 0xFF;
            packetData[1] = (chunkSeq >> 8) & 0xFF;
            
            // Payload
            memcpy(&packetData[2], fb->buf + offset, chunkSize);
            
            esp_now_send(broadcastAddress, packetData, chunkSize + 2);
            
            offset += chunkSize;
            chunkSeq++;
            // Small throttle to prevent flooding the receiver's buffer
            delayMicroseconds(500); 
        }

        esp_camera_fb_return(fb);
        // No forced delay here; the receiver controls the speed
    }
}

void setupWeb() {
    // Use AP_STA to support both Web Server and ESP-NOW concurrently
    WiFi.mode(WIFI_AP_STA);

    // 1. Setup Access Point (Local Device Network)
    WiFi.softAP(WIFI_SSID_AP, WIFI_PASS_AP, 1, 0, 4);

    // 2. Attempt to connect to saved Station (Router Network)
    String s = preferences.getString("sta_ssid", "");
    String p = preferences.getString("sta_pass", "");
    if (s.length() > 0) {
        Serial.println("Connecting to Station: " + s);
        wifiConnecting = true;
        WiFi.begin(s.c_str(), p.c_str());
    }

    WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set maximum WiFi transmission power

    // 3. Setup mDNS (Access via http://el-baseet.local)
    if (MDNS.begin("el-baseet")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("mDNS started: http://el-baseet.local");
    }

    server.on("/", handleRoot);
    server.on("/config", handleConfig);
    server.on("/readings", handleReadings);
    server.on("/wifi", handleWifi);
    server.on("/telemetry", handleTelemetry);
    server.on("/control", handleControl);
    server.on("/scan", handleScan);
    server.on("/savewifi", HTTP_POST, handleSaveWifi);
    server.on("/stream", handleStream);
    server.begin();
    Serial.println("AP IP: " + WiFi.softAPIP().toString());
}

#endif