# 🚀 Deep Dive: eSPI (Enhanced Serial Peripheral Interface)

This folder explores eSPI, the modern, high-speed, low-voltage (1.8V) bus that replaced the legacy LPC (Low Pin Count) bus for communication between the Embedded Controller (EC) and the Host (Intel/AMD PCH).

*Reference reading: [The introduction of eSPI](https://zhuanlan.zhihu.com/p/5157949788)*

## 1. The Physical Layer: Pins and the `Alert#` Mechanism
Before discussing logical packets, an EC firmware engineer must understand the physical hardware topology. eSPI drastically reduces motherboard routing by multiplexing data over a few critical pins:

* **CLK (Clock):** Driven *strictly* by the Host (PCH). The EC cannot generate its own clock on the eSPI bus.
* **CS# (Chip Select):** Driven by the PCH to select the EC for communication.
* **I/O [3:0] (Data Lines):** Bi-directional lines used to transmit commands, headers, and payloads (often operating in Quad-IO mode for massive bandwidth).
* **Reset#:** Hardware reset signal.
* **Alert# (The Game Changer):** An active-low, asynchronous signal driven by the EC. Because the PCH is the absolute master and controls the clock, the EC uses the `Alert#` pin to physically "raise its hand." It pulls this pin low to beg the PCH to turn on the clock so the EC can transmit urgent data.

## 2. The Four Logical Channels
Unlike traditional SPI, eSPI multiplexes four distinct logical channels over those same physical pins:
1. **Peripheral Channel (PC):** Replaces legacy LPC I/O and memory cycles (e.g., KBC legacy ports 60h/64h).
2. **Virtual Wire Channel (VW):** The most revolutionary feature. It serializes physical sideband pins (like `SMI#`, `SCI#`, `RCIN#`, `SLP_S3#`) into digital packets.
3. **Out-of-Band Channel (OOB):** Used for management data (like SMBus tunneling) without waking up the main CPU.
4. **Flash Access Channel (FAC):** Allows the EC to share the main SPI BIOS flash with the PCH.

## 3. Hardware-Software Boundary: How a VW is Actually Transmitted
When the firmware modifies a Virtual Wire state (e.g., asserting `SCI#` for a low battery warning), it isn't just sending a packet immediately. The underlying hardware executes a strict physical sequence:
1. **Interrupt Request:** The EC hardware controller pulls the physical `Alert#` pin low.
2. **Host Response:** The PCH detects `Alert#`, pulls `CS#` low, and starts driving the `CLK` signal.
3. **In-Band Polling:** The PCH sends a command over the `I/O` lines asking, "EC, why did you pull Alert?"
4. **Data Transmission:** The EC responds over the `I/O` lines: "I have a Virtual Wire packet to send. Index 40h, assert SCI#."
5. **Bus Idle:** The PCH acknowledges, pulls `CS#` back high, and stops the clock.

## 4. The Project: Virtual Wire (VW) Simulation
In the legacy days, triggering an ACPI System Control Interrupt (SCI) meant pulling a dedicated physical `SCI#` copper trace low. Today, that wire is gone.

**This project simulates the firmware abstraction of this process:**
1. The EC detecting a critical hardware event (e.g., Battery < 10%).
2. The firmware constructing an eSPI Virtual Wire packet via bitwise masking (`1 << n`).
3. Modifying the simulated hardware registers (`INDEX_REG`, `DATA_REG`, `CTRL_REG`) to trigger the physical `Alert#` transmission sequence described above.