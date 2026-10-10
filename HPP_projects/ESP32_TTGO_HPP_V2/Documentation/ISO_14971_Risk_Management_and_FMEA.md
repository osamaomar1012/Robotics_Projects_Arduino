# ISO 14971 Risk Management & FMEA Report
## EL-BASEET Hair Pen Surgical Assistant (V2 / V3)
**Document Ref:** RMF-HPP-2026-REV-A  
**Regulatory Standard:** ISO 14971:2019 / EN ISO 14971:2019 (Medical Devices — Application of risk management to medical devices)  
**Classification:** Medical Device Class IIa (EU MDR 2017/745 Rule 9) / Class I (FDA 21 CFR 878.4820)  
**Author:** Eng. Osama Omar, Senior Integration Lead  
**Date:** October 2026  

---

## 1. Document Control & Approvals

| Version | Date | Author | Reviewer | Description of Change |
| :--- | :--- | :--- | :--- | :--- |
| **1.0** | 2026-10-04 | Eng. Osama Omar | Quality & Regulatory Assurance | Initial Commercial Release for Metanoia Rev A Hardware |

---

## 2. Scope & Device Identification

### 2.1 Device Identification
* **Commercial Name:** EL-BASEET Hair Pen-V2 / V3
* **Product Model:** HPP-V2 (ESP32 TTGO T-Display) & HPP-V3 (ESP32-S3 T-Display-S3)
* **Underlying Hardware Platform:** Metanoia Motor Control Carrier Board (Rev A)
* **Applied Actuator:** JGA12-N20 Micro Metal Gear Motor with Quadrature Encoder (V1) / Standard Micro DC Motor (V2)

### 2.2 Intended Purpose & Clinical Indication
The **EL-BASEET Hair Pen** is a motorized, battery-operated surgical instrument intended for use by trained hair restoration surgeons and dermatologists to score, dissect, and extract follicular unit grafts from donor scalp tissue during Follicular Unit Extraction (FUE) and Follicular Unit Implantation (FUI) procedures.

### 2.3 Environmental & Operating Conditions
* **Environment:** Ambulatory surgical centers, hospital procedure rooms, dermatological clinics.
* **Operating Temperature:** $15^\circ\text{C}$ to $35^\circ\text{C}$ ($59^\circ\text{F}$ to $95^\circ\text{F}$).
* **Sterilization Interface:** Autoclavable / disposable punch handpiece; carrier console operates in clean non-sterile field or sterile draped enclosure.

---

## 3. Risk Assessment Methodology

### 3.1 Severity Ranking ($S$)

| Level | Severity Name | Clinical & System Criteria |
| :---: | :--- | :--- |
| **5** | **Catastrophic** | Patient death, permanent severe disability, electrical shock causing cardiac arrest. |
| **4** | **Critical** | Irreversible follicle transection rate $> 20\%$, scalp burn, skin necrosis, secondary surgery required. |
| **3** | **Moderate** | Reversible injury, superficial dermal abrasion, procedure prolonged $> 30\text{ min}$, minor motor malfunction. |
| **2** | **Minor** | Inconvenience, procedure paused $< 5\text{ min}$, minor display flicker, battery discharged prematurely. |
| **1** | **Negligible** | Aesthetic cosmetic defect, non-clinical firmware log discrepancy. |

### 3.2 Occurrence Probability Ranking ($O$)

| Level | Probability Name | Frequency of Occurrence |
| :---: | :--- | :--- |
| **5** | **Frequent** | Occurs in $> 10\%$ of surgical operations ($\ge 1 \times 10^{-1}$). |
| **4** | **Probable** | Occurs in $1\% - 10\%$ of surgical operations ($1 \times 10^{-2}$ to $1 \times 10^{-1}$). |
| **3** | **Occasional** | Occurs in $0.1\% - 1\%$ of surgical operations ($1 \times 10^{-3}$ to $1 \times 10^{-2}$). |
| **2** | **Remote** | Occurs in $0.001\% - 0.1\%$ of surgical operations ($1 \times 10^{-5}$ to $1 \times 10^{-3}$). |
| **1** | **Improbable** | Less than $0.001\%$ probability ($< 1 \times 10^{-5}$). |

### 3.3 Detection Ranking ($D$)

| Level | Detection Name | Detection Capability Prior to Impact |
| :---: | :--- | :--- |
| **5** | **Undetectable** | Fault cannot be detected by hardware, firmware, or surgeon prior to patient harm. |
| **4** | **Low** | Latent software fault detected only upon catastrophic failure. |
| **3** | **Moderate** | Detected by surgeon via tactile feedback or audible noise within $1 - 2$ seconds. |
| **2** | **High** | Automatic real-time telemetry fault trip (OLED warning, buzzer, web alert). |
| **1** | **Almost Certain** | Immediate hardware lockout (hardware trip $< 10\text{ ms}$, hardware comparator, reverse ideal diode). |

### 3.4 Risk Evaluation Matrix & Action Thresholds
$$\text{RPN} = S \times O \times D$$

* **$\text{RPN} \le 30$:** Broadly Acceptable (Residual risk acceptable without further redesign).
* **$31 \le \text{RPN} \le 60$:** ALARP (As Low As Reasonably Practicable — requires documented mitigation).
* **$\text{RPN} > 60$:** Unacceptable (Design block — device cannot be released until mitigated).

---

## 4. Failure Mode and Effects Analysis (FMEA)

| ID | Component / Subsystem | Potential Failure Mode | Potential Effect of Failure | Initial $S$ | Initial $O$ | Initial $D$ | Initial RPN | Root Cause | Engineering Mitigations & Risk Control Measures | Verification Method | Final $S$ | Final $O$ | Final $D$ | Final RPN | Status |
| :--- | :--- | :--- | :--- | :---: | :---: | :---: | :---: | :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **F-01** | **Battery (BT1)** | 18650 cell inserted with reversed polarity. | Electrical component damage, fire risk, battery venting. | **5** | **3** | **4** | **60** | Operator error during battery swap in surgery. | **Dual Ideal-Diode Protection:** Parallel AO3401A P-MOSFETs (Q2/Q3) drop $< 80\text{ mV}$ forward; automatically isolate circuit when reversed ($V_{\text{GS}} = 0\text{ V}$, $< 1\text{ µA}$ leakage). | Reverse-voltage bench testing with $-4.2\text{ V}$ applied; verify $0\text{ mA}$ draw. | **1** | **1** | **1** | **1** | **Accepted** |
| **F-02** | **Charger (U2)** | Short-circuit or over-discharge on lithium cell. | Cell permanent damage, fire, deep discharge below $2.5\text{ V}$. | **4** | **3** | **4** | **48** | External circuit fault, cell aging. | **Protected OUT Subsystem:** TP4056 module includes DW01A protection IC + FS8205 dual MOSFET cut-off. Dedicated `BAT_N` net isolates cell negative from board GND. | Short output terminals; verify instantaneous cut-off. | **2** | **1** | **1** | **2** | **Accepted** |
| **F-03** | **Motor Driver (U4)** | Continuous high current / motor rotor mechanical stall. | Pen tip heating, scalp dermal burn, motor coil burnout. | **4** | **4** | **3** | **48** | Tissue entanglement, bone contact, needle jamming. | **10ms Hard Real-Time Trip:** Core 0 background task checks INA219 current every $10\text{ ms}$; if $I > 2.5\text{ A}$, motor is instantaneously halted (`setMotorSpeed(0)`), OLED alerts `OVERCURRENT`. | Jam rotor mechanically; verify shutdown within $10\text{ ms}$. | **2** | **2** | **1** | **4** | **Accepted** |
| **F-04** | **Graft Counting** | False positive graft counting from electrical noise or tremors. | Inaccurate EHR surgical records, miscounted graft tally. | **3** | **4** | **3** | **36** | High-frequency PWM spikes, hand tremors during manipulation. | **Current-Speed Sensor Fusion:** Requires simultaneous electrical current spike crossing dynamic threshold **AND** physical motor RPM dipping $\ge 10\%$ via encoder feedback. | Synthetic bench testing with manual pen shaking vs true penetration. | **2** | **1** | **2** | **4** | **Accepted** |
| **F-05** | **Oscillation Mode** | False count trigger during motor direction reversal. | Graft counter increments on every direction flip (CW $\leftrightarrow$ CCW). | **3** | **4** | **2** | **24** | Motor H-bridge reverse transient causes back-EMF current surge. | **Blanking Period Algorithm:** Firmware implements a deterministic $400\text{ ms}$ dead-band (`isTransitioning`) during direction reversal where counting is suppressed. | Execute 100 continuous direction reversals in air; confirm 0 counts. | **1** | **1** | **1** | **1** | **Accepted** |
| **F-06** | **Encoder (J4)** | Interrupt bounce / PWM EMI on Channel A/B lines. | RPM reading reads astronomically high (MHz noise), breaking fusion. | **3** | **4** | **3** | **36** | Motor commutator brush electrical arcing, parasitic PCB coupling. | **150µs Hardware-Filtered ISR:** `encoderISR()` enforces a $150\text{ µs}$ minimum threshold ($> 1,100\text{ RPM}$ upper limit), discarding MHz electrical bounce pulses. | Signal generator pulse injection with noise superposition. | **1** | **1** | **1** | **1** | **Accepted** |
| **F-07** | **Illumination (J5)** | Surgical LED stuck permanently ON (cannot be dimmed). | Dermal glare, excessive thermal radiation on scalp. | **3** | **3** | **2** | **18** | Incorrect P-channel MOSFET used as low-side switch (Finding F3). | **Logic-Level N-Channel MOSFET:** Replaced with AO3400A ($R_{\text{DS(on)}} < 48\text{ mΩ}$) with $47\text{ Ω}$ gate resistor R6 and $10\text{ kΩ}$ pull-down R7. | PWM duty cycle sweep from $0\%$ to $100\%$; confirm zero lux at $0\%$. | **1** | **1** | **1** | **1** | **Accepted** |
| **F-08** | **Software (RTOS)** | Priority inversion or deadlock between Web Server and Safety. | Safety trip delayed while WiFi client downloads web dashboard. | **4** | **3** | **4** | **48** | Single-core blocking socket operations. | **Dual-Core Asymmetric Tasking:** Safety, INA219 sampling, and motor loop run pinned to **Core 0** (Priority 3, $100\text{ Hz}$). Web server & UI run pinned to **Core 1** (Priority 1). | Saturate web server with 50 concurrent HTTP requests; verify Core 0 $10\text{ ms}$ loop. | **1** | **1** | **1** | **1** | **Accepted** |
| **F-09** | **Power Supply** | Battery voltage drops below safe threshold during surgery. | Uncontrolled motor speed deceleration, graft transection. | **3** | **4** | **2** | **24** | 18650 cell exhausted during prolonged procedure. | **Active Low-Battery Trip & Live Gauge:** When cell terminal drops below $3.2\text{ V}$, motor halts automatically. Live battery percentage and voltage displayed on OLED & Web. | Discharge battery on bench; verify clean cut-off at exactly $3.2\text{ V}$. | **2** | **2** | **1** | **4** | **Accepted** |
| **F-10** | **Sterilization** | Autoclave steam penetrates motor carrier enclosure. | Short-circuit, electrical destruction, corrosion. | **4** | **3** | **3** | **36** | Operator places electronic carrier unit inside steam autoclave. | **Design Separation & Labeling:** Handpiece sheath is detachable and fully autoclavable; electronic carrier is clearly labeled "NON-AUTOCLAVABLE — STERILE DRAPE / WIPE ONLY". | Label durability test per IEC 60601-1; autoclave validation on sheath. | **2** | **1** | **2** | **4** | **Accepted** |

---

## 5. Risk-Benefit Analysis & Conclusion

All identified hazards have been subjected to risk reduction measures following the standard ISO 14971 hierarchy:
1. **Inherent Safety by Design:** Reverse-polarity protection via ideal-diode FETs, dual-core task segregation, low-side N-channel switching, and 10 mΩ precision shunt measurement.
2. **Protective Measures:** 10 ms hard overcurrent trip, low battery voltage cut-off, and $400\text{ ms}$ oscillation blanking.
3. **Information for Safety:** Comprehensive warning labels, Instructions for Use (IFU), real-time on-device OLED warnings, and EHR report generation.

**Conclusion:** The overall residual risk of the **EL-BASEET Hair Pen** is determined to be **broadly acceptable**, with clinical benefits in graft survival and surgical ergonomics far outweighing all identified residual risks.
