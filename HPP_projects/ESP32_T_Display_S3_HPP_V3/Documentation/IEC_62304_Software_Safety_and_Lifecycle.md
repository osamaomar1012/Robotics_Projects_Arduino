# IEC 62304 Software Safety & Architectural Dossier
## EL-BASEET Hair Pen Embedded Firmware (V2 / V3)
**Document Ref:** SWD-HPP-62304-REV-A  
**Regulatory Standard:** IEC 62304:2006 / AMD 1:2015 (Medical device software — Software life cycle processes)  
**Software Safety Classification:** **Class B** (Software contributes to a hazard which could result in non-serious injury if unmitigated)  
**Author:** Eng. Osama Omar, Senior Integration Lead  
**Date:** October 2026  

---

## 1. Software Safety Classification Justification

In accordance with **IEC 62304 Clause 4.3**, the firmware for the EL-BASEET Hair Pen is classified as **Class B**:
* The software controls an electromechanical actuator (micromotor) used directly on human patient scalp tissue.
* In the event of software failure (e.g., motor over-speed runaway or failed stall detection), the resulting harm is restricted to local dermal laceration or graft transection (non-serious injury).
* Independent hardware and architectural risk control measures (hardware current shunt, reverse protection, low battery cut-offs) prevent any severe or life-threatening harm.

---

## 2. Software System Architecture & Multicore Segregation

To ensure absolute deterministic responsiveness for surgical safety, the firmware utilizes the ESP32 / ESP32-S3 dual-core asymmetric architecture managed by FreeRTOS:

```
+===================================================================================+
|                              ESP32 DUAL-CORE HARDWARE ARCHITECTURE                |
+===================================================================================+
|                                        |                                          |
|        CORE 0: DETERMINISTIC SAFETY    |          CORE 1: UI & CONNECTIVITY       |
|            (High Priority 3)           |             (Normal Priority 1)          |
|                                        |                                          |
|  +----------------------------------+  |  +------------------------------------+  |
|  | FreeRTOS 100 Hz Loop (Every 10ms)|  |  | Main Arduino loop() Loop (~10-33Hz)|  |
|  | - INA219 Telemetry Read          |  |  | - ST7789 TFT Display Rendering     |  |
|  | - 10ms Hard Overcurrent Trip     |  |  | - Screensaver Frame Playback       |  |
|  | - 3.2V Low Battery Trip          |  |  | - Dual Button Debounce & Menu     |  |
|  | - Bidirectional PWM Speed Ramp  |  |  | - Web Server & WebSocket Streaming|  |
|  | - Sensor-Fusion Graft Counting   |  |  | - DNS Captive Portal Handling      |  |
|  +----------------------------------+  |  +------------------------------------+  |
|                   |                    |                    |                     |
|                   v                    |                    v                     |
|  +----------------------------------+  |  +------------------------------------+  |
|  | Shared Telemetry Volatiles       |==|==| Non-Volatile Memory (NVM)          |  |
|  | (Speed, Current, RPM, Counts)    |  |  | (Preferences / Flash SPIFFS)      |  |
|  +----------------------------------+  |  +------------------------------------+  |
+===================================================================================+
```

### 2.1 Core 0: Real-Time Safety & Sensor Fusion Engine
* **Execution Frequency:** $100\text{ Hz}$ ($10\text{ ms}$ fixed period using FreeRTOS `vTaskDelayUntil`).
* **Priority Level:** Priority 3 (Higher than idle and standard user tasks).
* **Guaranteed Maximum Latency:** Any motor stall, short-circuit, or overcurrent event is isolated, evaluated, and de-energized within **$< 10\text{ ms}$**.
* **Zero-Blocking Guarantee:** Contains zero blocking I/O, zero network calls, and zero filesystem writes.

### 2.2 Core 1: User Interface & Web Connectivity Engine
* **Execution Frequency:** Dynamic interval ($30\text{ ms} \approx 33\text{ FPS}$ during animated screensavers; $100\text{ ms}$ in static surgery mode).
* **Isolation Boundary:** WiFi socket stalls or high HTTP client traffic on Core 1 have **zero effect** on motor control or graft counting on Core 0.

---

## 3. Software Requirements Specification (SRS) & Traceability

| Req ID | Requirement Description | Safety Impact | Verification Method |
| :--- | :--- | :--- | :--- |
| **SRS-01** | The firmware shall evaluate INA219 motor current every $10\text{ ms}$. If $I_{\text{motor}} > 2.5\text{ A}$, the motor PWM shall be driven to 0 immediately. | Prevents tissue burn or motor thermal runaway. | **Fault Injection Test:** Inject $3.0\text{ A}$ load; verify shutdown $< 10\text{ ms}$. |
| **SRS-02** | The firmware shall halt motor rotation and lock operation if battery cell voltage drops below $3.20\text{ V}$. | Prevents deep cell discharge and erratic motor speed. | **Bench Voltage Sweep:** Ramp DC supply from $4.2\text{ V} \rightarrow 3.1\text{ V}$; verify trip at $3.2\text{ V}$. |
| **SRS-03** | The graft counter shall increment if and only if: (1) Current exceeds baseline $+ \Delta I_{\text{spike}}$, (2) RPM drops $\ge 10\%$ below baseline, and (3) Current falls below fallback threshold. | Eliminates false-positive graft counts from air-shakes. | **Sensor Fusion Bench Rig:** Compare true physical punctures vs synthetic vibration. |
| **SRS-04** | During oscillation mode, the firmware shall suppress graft counting during the $400\text{ ms}$ transition window following direction reversal. | Prevents back-EMF spikes from triggering false counts. | **Oscillation Test:** Run 500 reversals in air; confirm exactly 0 false counts. |
| **SRS-05** | The encoder interrupt routine shall discard any pulse arriving within $150\text{ µs}$ of the preceding pulse. | Prevents high-frequency brush electrical bounce noise. | **Square-Wave Generator:** Inject $1\text{ MHz}$ noise bursts; verify RPM remains accurate. |
| **SRS-06** | All surgical parameters (Base Speed, Mode, LED Brightness, Graft Count, Patient Metadata) shall be persisted to NVM on change. | Prevents clinical data loss during unexpected battery disconnect. | **Power-Cycle Test:** Abruptly disconnect power during surgery; verify recovery of count. |
| **SRS-07** | Dual long-press of physical buttons for $> 1.0\text{ s}$ shall toggle the on-device OLED menu without external web dependencies. | Guarantees full surgeon control even if WiFi is jammed. | **Usability Stress Test:** Validate 100 consecutive menu entries/exits. |
| **SRS-08** | The local web dashboard shall stream live Voltage, Current, and RPM at $\ge 2\text{ Hz}$ using client-side HTML5 Canvas. | Provides instantaneous clinical diagnostics. | **Web Socket Latency Test:** Measure frame delivery under 5 client connections. |

---

## 4. Software Verification & Validation (V&V) Protocol

### 4.1 Boundary Value & Stress Testing
1. **Speed Sweep:** Increment speed from $0 \rightarrow 255$ and reverse $-255 \rightarrow 0$. Confirm continuous linear PWM output without overflow or wrap-around.
2. **Current Calibration Range:** Test with motor unloaded ($I \approx 20\text{ mA} - 50\text{ mA}$) through heavy load ($I \approx 800\text{ mA} - 1.8\text{ A}$). Verify INA219 calibration factor ($10\text{ mΩ}$ shunt) accurately reads true amperage against an external calibrated Keysight DMM within $\pm 2\%$.
3. **Watchdog Timer & Task Starvation:** Verify FreeRTOS Task Watchdog Timer (TWDT) is active. Artificially hang Core 1; confirm Core 0 continues safe 10 ms monitoring.

### 4.2 Data Integrity & Non-Volatile Storage (NVM)
* **Namespace Segregation:** Preferences stored under `"hpp_v1"` namespace.
* **Integrity Checks:** Values read on boot undergo sanity checks (e.g., if `sleepTime < 600000`, reset to default $900,000\text{ ms}$).
* **EHR Report Generation:** Post-op reports stored as structured JSON in internal flash SPIFFS with sanitized string escaping to prevent injection attacks.

---

## 5. Cybersecurity & Medical Data Privacy

* **Air-Gapped Operation:** The device does not connect to the public internet or external cloud infrastructure. All web operations run locally via an autonomous on-chip Access Point (`HPP_CONTROL_V2` / `V1`).
* **WPA2 Security:** Access Point is secured with WPA2-PSK encryption. Default credentials must be changed by clinical administrators upon initial facility deployment.
* **Patient Data Protection:** Patient names, ages, and medical IDs stored in volatile/NVM memory are localized entirely on the pen console; no telemetry is leaked across public networks.
* **Buffer Overflow Prevention:** Firmware uses standard safe C++ `String` classes with explicit size constraints and sanitized input parsers for all HTTP POST endpoints.
