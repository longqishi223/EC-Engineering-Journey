# STM32 Hardware Protocol Expedition: I2C, SPI, and USART

Welcome to the core communication layer of the `EC-Engineering-Journey`. 

This directory (`05_STM32_EXP`) is dedicated to tearing down, understanding, and rebuilding the drivers for the three most fundamental serial communication protocols in embedded systems: **I2C, SPI, and USART**. 

Instead of merely calling high-level HAL (Hardware Abstraction Layer) functions, the approach here is **bottom-up and bare-metal**. We dissect the ST Standard Peripheral Library (`stm32f10x_xxx.c`) line by line, mapping C structures to physical silicon registers, bitmasks, and hardware state machines.

## 🎯 Why Dissect These Three Protocols? (The "Why")

For an Embedded Controller (EC) firmware engineer, relying on "black box" libraries is a fatal weakness. When a smart battery stops responding, or a BIOS update over an SPI flash fails, high-level APIs won't tell you why. 

Mastering these three protocols at the register level provides a profound engineering foundation:

1. **The Holy Trinity of Embedded Buses**: 95% of all peripheral ICs (sensors, EEPROMs, Flash memory, PMICs, Debug consoles) communicate via one of these three buses. Mastering them means you can interface with almost any chip in the world.
2. **Decoding Hardware-Software Symbiosis**: By analyzing how protocols are implemented in C (e.g., bit-shifting macro routing, `rc_w0` clear mechanisms, interrupt alignment), we learn how to write highly efficient, atomic, and safe firmware that respects silicon-level quirks.
3. **From "Coder" to "Architect"**: Understanding *Clock Stretching* in I2C, *CPOL/CPHA* in SPI, and *Baud Rate Generation* in USART builds the intuition required to design custom drivers, handle catastrophic bus errors, and optimize real-time operating systems (RTOS).

---

## 🧭 The Protocol Breakdown

### 📂 1. [I2C (Inter-Integrated Circuit)](./I2C/)
* **The Complex Diplomat**: A 2-wire, half-duplex, open-drain bus. 
* **Focus Areas**: 
  * Multi-master arbitration and addressing (7-bit/10-bit).
  * The notoriously complex internal state machine (EV5, EV6, EV8).
  * Hardware quirks like Clock Stretching and precise clearing sequences for Event/Error flags.
  * **EC Relevance**: The direct foundation for **SMBus** (System Management Bus), which is strictly required for reading Smart Batteries and thermal sensors in laptops.

### 📂 2. [SPI (Serial Peripheral Interface)](./SPI/)
* **The High-Speed Sprinter**: A 4-wire, full-duplex, push-pull bus.
* **Focus Areas**:
  * Raw speed and pure shift-register mechanics.
  * Clock Polarity (CPOL) and Clock Phase (CPHA) configurations.
  * Hardware vs. Software Slave Select (NSS) management.
  * **EC Relevance**: The backbone for **eSPI** (Enhanced SPI) and communicating with BIOS/UEFI Flash ROMs, requiring absolute timing precision.

### 📂 3. [USART (Universal Synchronous/Asynchronous Receiver Transmitter)](./USART/)
* **The Reliable Workhorse**: The classic asynchronous serial interface.
* **Focus Areas**:
  * Asynchronous timing, oversampling, and Baud Rate generation (Fractional dividers).
  * Parity, framing errors, and noise detection at the register level.
  * Ring buffers and interrupt-driven Tx/Rx architectures.
  * **EC Relevance**: The ultimate debugging lifeline. EC engineers rely heavily on UART consoles to output system states during power-sequencing before the main CPU even turns on.

---

## 🛠️ The Engineering Methodology

Throughout these sub-directories, the learning process follows a strict routine:
1. **Datasheet & Reference Manual**: Map the physical registers (`CR1`, `CR2`, `SR`, `DR`).
2. **Source Code Autopsy**: Deconstruct ST's `stm32f10x_xxx.c` files to uncover C-language architecture tricks (e.g., mapping structs to absolute memory addresses, pointer casting, bitwise masking).
3. **Bare-Metal Implementation**: Write clean, state-machine-driven algorithms (like Burst Reads) without relying on blocking `while()` loops wherever possible.
4. **Logic Analysis**: Verify the physical waveforms (SCL/SDA, SCK/MOSI/MISO, TX/RX) against the theoretical code.

> *"To command the hardware, you must speak its native tongue: Registers."*