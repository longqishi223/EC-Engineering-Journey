# ⌨️ Deep Dive: HMI & Keyboard Matrix Scanning

This folder explores how the Embedded Controller (EC) manages Human-Machine Interfaces (HMI), with a primary focus on the laptop's built-in keyboard. 

Unlike external USB mechanical keyboards that contain their own dedicated microcontrollers, a laptop's internal keyboard is completely "dumb." It is merely a passive membrane of conductive traces forming a matrix. The EC must do all the heavy lifting: scanning, debouncing, and translating physical shorts into logical OS keystrokes.

---

## 1. The Physical Layer: KSI and KSO


The keyboard membrane is connected directly to the EC's General-Purpose Input/Output (GPIO) pins, divided into two groups:
* **KSO (Keyboard Scan Out - Columns):** Configured as outputs.
* **KSI (Keyboard Scan In - Rows):** Configured as inputs with internal pull-up resistors (defaulting to HIGH / Logic 1).

**The Scanning Process:**
The EC firmware runs a continuous, high-speed loop. It drives one KSO line LOW (0V) at a time while keeping the others HIGH. Simultaneously, it reads the state of all KSI lines. 
* If no key is pressed, all KSI lines read HIGH (pulled up).
* If a key is pressed at the intersection of the active KSO and a specific KSI, the physical switch closes. The LOW signal from the KSO pulls the KSI line LOW. The EC registers the exact `[Row, Column]` coordinate of the keypress.

---

## 2. The Firmware Challenges: Debounce & Anti-Ghosting

Raw physics makes scanning difficult. The EC firmware must filter out mechanical imperfections before sending data to the OS.

### A. Mechanical Bouncing (The Debounce Algorithm)

When metal contacts strike each other, they physically vibrate (bounce) for 10-20 milliseconds before settling. Without firmware intervention, a single physical press of the 'A' key would look like `1010101111` to the CPU, resulting in "AAAAA" on the screen. 
* *Solution:* The firmware implements a **State Machine Timer** (demonstrated in the C code in this folder) that waits for the GPIO signal to remain stable for a strict threshold (e.g., 20ms) before validating the press.

### B. Anti-Ghosting and Sneak Paths
When three keys forming three corners of a rectangle on the matrix are pressed simultaneously, the electrical current can flow backward through a "sneak path," causing the EC to falsely read the fourth corner as being pressed. This is called a **Ghost Key**.
* *Solution:* High-end gaming ECs use complex firmware algorithms to detect these ambiguous matrix states and intentionally ignore the phantom signals, ensuring N-Key Rollover (NKRO) accuracy.

---

## 3. Power State Management: Interrupt-Driven Wake

The EC is the guardian of battery life. When the laptop enters Sleep (S3) or Modern Standby (S0ix), it is a massive waste of power for the EC to continuously poll the KSO/KSI matrix thousands of times a second.

Instead, the EC halts the scanning loop. It configures all KSO lines to output LOW simultaneously and sets the KSI lines to trigger a **Hardware Asynchronous Interrupt** on a falling edge. 
When the user presses *any* key, the matrix shorts a KSI line to ground. This physical voltage drop wakes the EC's CPU core instantly. The EC then:
1. Resumes the high-speed KSO/KSI scanning loop to figure out *which* key was pressed.
2. Triggers an eSPI Virtual Wire (like `WAKE#` or `SCI#`) to wake up the Intel/AMD main processor and turn on the screen.

---

## 4. Beyond the Keyboard: Battery & Charging Safety
*(Note: Refer to the `03_Low_Level_Buses` folder for the SMBus implementation of this feature).*

As the ultimate safety middleman, the EC uses these same real-time concepts to manage the discrete Charger IC. Using the Smart Battery System (SBS) over SMBus, the EC continuously polls the battery's Relative State of Charge (RSOC) and temperature. If a thermal spike occurs, the EC acts autonomously—throttling the CPU or physically cutting off the Charger IC via GPIO to prevent thermal runaway, proving its role as the most critical safety microcontroller in the system.