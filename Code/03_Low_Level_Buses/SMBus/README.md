# 🚌 Deep Dive: SMBus & Smart Battery System (SBS)

This folder contains my implementation and exploration of the System Management Bus (SMBus), specifically focusing on how the Embedded Controller (EC) communicates with the laptop's Smart Battery. 

While it's easy to mock data in software, a true EC Firmware Engineer must understand how data travels across the physical wires and how the microcontroller's hardware translates voltages into C variables.

*Reference reading: [The introduction of SMBus on CSDN](https://blog.csdn.net/caiji0169/article/details/142892183)*

---

## 1. The Physical Layer & Strict Constraints


SMBus is a two-wire interface derived from I2C, utilizing **SCL (Serial Clock)** and **SDA (Serial Data)**. Both lines are "open-drain" and pulled HIGH via resistors. Devices communicate by pulling the lines LOW to GND.

However, SMBus imposes strictly defined electrical and timing constraints that standard I2C lacks:
* **The 35ms Timeout Rule:** If any device (Master or Slave) holds the SCL line LOW for more than ~35ms, the hardware forces a bus reset. This guarantees that a crashed battery IC will never permanently freeze the EC.
* **Fixed Logic Levels:** Unlike I2C (which is VCC relative), SMBus logic levels are fixed (e.g., HIGH is strictly > 2.1V), ensuring reliable communication across different power domains on the motherboard.
* **PEC (Packet Error Checking):** An extra checksum byte appended to the end of transactions to ensure critical battery data (like voltage and temperature) isn't corrupted by motherboard electrical noise.

---

## 2. The Protocol: The "Read Word" Timing Sequence


When the EC wants to read a 16-bit value (like Voltage) from the battery, it executes the **SMBus Read Word Protocol**. A critical detail here is **Little-Endian transmission**: the battery transmits the Least Significant Byte (LSB) first over the physical wire.

**The Exact Physical Sequence:**
1. **START Condition (S):** EC pulls SDA LOW while SCL is HIGH.
2. **Slave Address (Write):** EC sends the 7-bit battery address (e.g., `0x16`) + `0` (Write bit).
3. **ACK:** Battery pulls SDA LOW.
4. **Command Code:** EC sends the 8-bit SBS Command (e.g., `0x09` for Voltage). Battery ACKs.
5. **Repeated START (Sr):** EC changes data direction by issuing another START.
6. **Slave Address (Read):** EC sends battery address + `1` (Read bit). Battery ACKs.
7. **Data Byte Low (LSB):** Battery clocks out the lower 8 bits. **EC ACKs**.
8. **Data Byte High (MSB):** Battery clocks out the upper 8 bits. **EC NACKs** (leaves SDA HIGH) to signal "I have enough data."
9. **STOP Condition (P):** EC releases SDA to HIGH while SCL is HIGH.

---

## 3. The Firmware Layer: How C Code Drives the Hardware

In a real System-on-Chip (SoC) like an EC, firmware engineers don't toggle pins manually. Instead, we interact with **Memory-Mapped Hardware Registers**. The underlying hardware controller automatically generates the START, STOP, and Clock signals.

Here is a conceptual look at how a real EC firmware driver reads the SMBus:

```c
// Hardware Registers (Mapped to specific physical memory addresses)
#define SMBUS_STATUS_REG  (*(volatile uint8_t*)0x40001000)
#define SMBUS_CONTROL_REG (*(volatile uint8_t*)0x40001001)
#define SMBUS_ADDR_REG    (*(volatile uint8_t*)0x40001003)
#define SMBUS_CMD_REG     (*(volatile uint8_t*)0x40001004)
#define SMBUS_DATA0_REG   (*(volatile uint8_t*)0x40001005) // LSB
#define SMBUS_DATA1_REG   (*(volatile uint8_t*)0x40001006) // MSB

uint16_t SMBus_Real_ReadWord(uint8_t slave_addr, uint8_t command) {
    uint32_t timeout = 0;

    // 1. Configure the hardware controller
    SMBUS_ADDR_REG = (slave_addr << 1);     // Send slave address to the address register so the master knows where the message goes.
    SMBUS_CMD_REG  = command;

    // 2. Pull the trigger (Hardware starts generating the SCL/SDA waveform)
    SMBUS_CONTROL_REG = 0x0B; // Command to execute "Read Word"

    // 3. Polling with Timeout Protection
    // Wait for the hardware to finish the physical transaction
    do {
        timeout++;
        if (timeout > 50000) { 
            return 0xFFFF; // FATAL: Bus is hung. Abort to prevent EC crash!
        }
    } while (SMBUS_STATUS_REG & 0x01); // Check BUSY bit

    // 4. Retrieve data from hardware registers
    uint8_t lsb = SMBUS_DATA0_REG;
    uint8_t msb = SMBUS_DATA1_REG;

    // 5. Merge Little-Endian bytes (Bitwise shift magic)
    return (uint16_t)((msb << 8) | lsb);
}