# Clinical Evaluation & Instructions for Use (IFU)
## EL-BASEET Hair Pen Surgical Assistant (V2 / V3)
**Document Ref:** IFU-HPP-2026-REV-A  
**Device Classification:** Medical Device Class IIa (EU MDR 2017/745) / Class I (US FDA 21 CFR 878.4820)  
**Applicable Standards:** ISO 17664 (Processing of health care products), ISO 10993-1 (Biocompatibility), IEC 60601-1 (Medical electrical equipment)  
**Author:** Eng. Osama Omar, Senior Integration Lead  
**Date:** October 2026  

---

## 1. Device Description & Clinical Overview

The **EL-BASEET Hair Pen** is a cordless, battery-powered motorized surgical instrument engineered for high-precision Follicular Unit Extraction (FUE) and Follicular Unit Implantation (FUI). 

The instrument combines a high-speed micro-gear actuator, an embedded dual-core microprocessor, an integrated optical/magnetic quadrature encoder, and high-side precision electrical current monitoring ($\text{INA219}$) to provide:
1. **Low-Torque Dissection:** Smooth, bidirectional micro-rotary and oscillating punch actuation that cleanly separates the arrector pili muscle and connective tissue attachment without shearing the follicle bulb.
2. **Real-Time Sensor-Fusion Graft Counting:** Automatic detection and recording of genuine follicular harvest events based on simultaneous electrical and mechanical resistance vectors, completely eliminating the need for manual foot-pedal tallying or secondary assistants.
3. **Onboard Telemetry & Surgical Headlight:** Integrated surgical LED illumination and live OLED status feedback showing operational speed, battery state, current draw, and total graft tallies.

---

## 2. Clinical Indications & Contraindications

### 2.1 Indications for Use
The EL-BASEET Hair Pen is indicated for:
* Androgenetic alopecia (male and female pattern hair loss).
* Cicatricial / scar alopecia requiring follicular graft transplantation.
* Reconstructive eyebrow, beard, and mustache restoration.
* Body hair harvesting (BHT) for secondary donor supplementation.

### 2.2 Contraindications
The device is contraindicated in patients with:
* Active bacterial, fungal, or viral infections of the scalp (e.g., folliculitis decalvans, tinea capitis).
* Known bleeding diathesis, unmanaged coagulopathies, or active anticoagulant therapy that cannot be temporarily bridged.
* History of keloidal scarring or hypertrophic tissue proliferation in the donor zone.
* Severe hypersensitivity to surgical stainless steel ($316\text{L}$) or medical-grade titanium punch instruments.

---

## 3. Applied Parts & Biocompatibility (ISO 10993-1)

* **Patient Applied Part (Type B / BF):** Surgical dissection punch tip ($0.7\text{ mm} - 1.0\text{ mm}$ internal diameter). Constructed exclusively from implant-grade ASTM F138 $316\text{L}$ stainless steel or medical-grade titanium alloy. Dermal contact duration is categorized as *Limited Exposure* ($< 24\text{ hours}$).
* **Operator Contact Part:** Anodized aluminum surgical handpiece sleeve and ergonomic medical-grade silicone grip. Biocompatible under ISO 10993-5 (Cytotoxicity) and ISO 10993-10 (Irritation and Skin Sensitization).

---

## 4. Pre-Operative Setup & Calibration Protocol

```
+-----------------------------------------------------------------------------------+
|                        PRE-OPERATIVE CLINICAL WORKFLOW                            |
+-----------------------------------------------------------------------------------+
|  [Step 1] Inspect Battery & Charge State (> 80% / > 3.9V)                        |
|       |                                                                           |
|       v                                                                           |
|  [Step 2] Attach Sterile Punch & Sleeve; Inspect for concentricity & runout        |
|       |                                                                           |
|       v                                                                           |
|  [Step 3] Power ON via Main Switch (J2); Observe Welcome Animation & Self-Test    |
|       |                                                                           |
|       v                                                                           |
|  [Step 4] Execute Air-Load Calibration ("RE-CALIBRATE" on OLED or Web Dashboard)  |
|       |   - Pen held freely in air; spins 2 seconds to establish clean baseline   |
|       v                                                                           |
|  [Step 5] Select Clinical Mode (Continuous vs Oscillating) & Tune Thresholds      |
+-----------------------------------------------------------------------------------+
```

### 4.1 Air-Load Self-Calibration Procedure
Before commencing surgery or upon changing the punch diameter, the surgeon **must calibrate the air baseline**:
1. Hold the pen vertically in free air without touching any tissue, drapes, or surfaces.
2. Enter the on-device menu (dual long-press) and select **`Re-Calibrate`**, or click **`RE-CALIBRATE BASELINE`** on the local web dashboard (`http://elbaseet.local`).
3. The motor will spin automatically for **$2.0\text{ seconds}$**. During this window, the firmware averages $200$ samples to compute:
   * **Baseline Current ($I_{\text{base}}$):** Typically $25\text{ mA} - 55\text{ mA}$ at $200\text{ PWM}$.
   * **Baseline RPM ($\text{RPM}_{\text{base}}$):** Measured via encoder hardware interrupts.
4. When the display confirms **`CALIBRATED`**, the pen is primed for surgery.

---

## 5. Intra-Operative Surgical Guidelines

### 5.1 Operating Modes
1. **Continuous Rotation Mode:**
   * **Clinical Indication:** Straight, coarse hair follicles with thick scalp dermis.
   * **Recommended Speed:** $800 - 1,200\text{ RPM}$ ($160 - 210\text{ PWM}$).
   * **Technique:** Align punch axis parallel to the emergent hair angle; gently engage scalp with light forward pressure until a tactile release is felt ($1.5\text{ mm} - 2.5\text{ mm}$ depth).
2. **Oscillating Mode (FUE Oscillation):**
   * **Clinical Indication:** Fine hair, Afro-textured / curly follicles, or delicate graft attachments where rotational torque risks graft transection.
   * **Timing Parameters:** Clockwise duration ($t_{\text{CW}} = 1,500\text{ ms} - 2,000\text{ ms}$), Counter-Clockwise duration ($t_{\text{CCW}} = 1,500\text{ ms} - 2,000\text{ ms}$).
   * **Technique:** The alternating motion breaks the dermal ring with minimal axial torsion, preserving outer root sheath integrity.

### 5.2 Sensor Fusion Sensitivity Tuning

| Donor Tissue Type | Scalp Characteristics | Spike Offset ($\Delta I_{\text{spike}}$) | Speed Dip Threshold ($\Delta\text{RPM}$) | Clinical Rationale |
| :--- | :--- | :---: | :---: | :--- |
| **Delicate / Fine** | Thin scalp, soft dermis | **`0.030 A`** | **`8%`** | Higher sensitivity catches subtle mechanical entries without excessive force. |
| **Standard Adult** | Normal occipital donor zone | **`0.040 A`** *(Default)* | **`10%`** *(Default)* | Optimal balance between counting fidelity and tremor rejection. |
| **Coarse / Fibrotic** | Thick, scarred, or sun-damaged scalp | **`0.060 A`** | **`12%`** | Elevated thresholds prevent false counts from superficial epidermal dragging. |

---

## 6. Cleaning, Disinfection & Sterilization (ISO 17664)

```
+===================================================================================+
|                        STERILIZATION & REPROCESSING BOUNDARY                      |
+===================================================================================+
|                                                                                   |
|  [DETACHABLE SURGICAL SLEEVE & PUNCH]       [ELECTRONIC CONTROLLER & BATTERY]     |
|             (STERILE ZONE)                               (NON-STERILE ZONE)       |
|                                                                                   |
|  - Fully Autoclavable (Steam)               - DO NOT AUTOCLAVE                    |
|  - Pre-vacuum steam: 134°C for 5 min        - DO NOT IMMERSE IN LIQUID            |
|  - Gravity displacement: 121°C for 20 min   - Wipe clean with 70% Isopropanol     |
|  - Ultrasonic cleaning in enzymatic bath     - Use sterile surgical barrier drape  |
|                                                                                   |
+===================================================================================+
```

### 6.1 Reprocessing the Surgical Handpiece Sheath
1. **Disassembly:** Unscrew the front collet and detach the stainless steel sleeve from the motor housing.
2. **Enzymatic Pre-wash:** Immerse punch and sleeve in neutral pH enzymatic cleaner ($< 40^\circ\text{C}$) for $5\text{ minutes}$. Use a nylon lumen brush to clean the internal cannulation.
3. **Steam Sterilization:** Autoclave in validated medical sterilization pouches:
   * **Pre-vacuum Cycle:** $134^\circ\text{C}$ ($273^\circ\text{F}$) for $\ge 4\text{ minutes}$, dry time $20\text{ minutes}$.
   * **Gravity Cycle:** $121^\circ\text{C}$ ($250^\circ\text{F}$) for $\ge 30\text{ minutes}$, dry time $15\text{ minutes}$.

### 6.2 Cleaning the Electronic Carrier & Console
* **Wipe-Down Only:** Decontaminate the exterior console using hospital-grade disinfectant wipes (e.g., $70\%$ Isopropyl Alcohol, CaviWipes, or quaternary ammonium).
* **Barrier Protection:** During clinical surgery, the controller console should be enclosed in a clear, sterile polyethylene surgical drape (e.g., camera/laser drape).

---

## 7. Electronic Health Record (EHR) Post-Op Certification

Upon completion of graft harvesting:
1. Long-press Left Button while in PAUSED mode (or click **`STOP OPERATION`** on the web dashboard).
2. All surgical data—including total grafts, active harvesting duration, pause intervals, and date/time stamps—are consolidated into non-volatile memory (`SPIFFS`).
3. Click **`PRINT EHR REPORT`** from the web portal (`http://elbaseet.local/report`).
4. The generated document contains formatted clinical summary tables, legal verification stamps, and designated signature fields for the attending surgeon and surgical technician.

---

## 8. Clinical Troubleshooting & Alarm Matrix

| Observed Display / State | Clinical Root Cause | Immediate Surgeon Action |
| :--- | :--- | :--- |
| **`CRITICAL: OVERCURRENT`** | Needle blocked against dense tissue or punch jammed with bone/coagulum. | Remove punch from scalp immediately. Inspect punch lumen for tissue blockage. Motor will auto-recover once clear. |
| **`LOW BATTERY: TRIP`** | Battery voltage fallen below $3.2\text{ V}$. | Pause procedure. Swap cell with a charged 18650 battery or connect USB-C charger. Data is safely stored in NVM. |
| **Graft Counts Not Incrementing** | Air-load baseline shifted due to thermal warming or change of operator hand position. | Hold pen in air and execute **`Re-Calibrate`**. Verify punch is rotating freely. |
| **Display Remains Dark on Power-On** | Main switch (J2) open, or battery deeply discharged $< 2.5\text{ V}$. | Ensure switch J2 is ON. Plug in USB-C cable to verify TP4056 charging LED indicator. |
