
# 🚌 Deep Dive: SMBus & Smart Battery System (SBS)

This folder contains my implementation and exploration of the System Management Bus (SMBus), specifically focusing on how the Embedded Controller (EC) communicates with the laptop's Smart Battery. 

While it's easy to mock data in software, a true EC Firmware Engineer must understand how data travels across the physical wires and how the microcontroller's hardware translates voltages into C variables.

---

## 1. The Physical Layer: Two Wires to Rule Them All

The introduction of SMBus on CSDN: https://blog.csdn.net/caiji0169/article/details/142892183.

SMBus is a two-wire interface derived from I2C. Physically, it consists of:
* **SCL (Serial Clock):** Driven by the Host (the EC) to synchronize data transfers.
* **SDA (Serial Data):** Bi-directional line used to transmit the actual bits.

**The Open-Drain Concept:**
Both lines are "open-drain" and connected to a positive supply voltage (e.g., 3.3V) via pull-up resistors. 
* By default, the lines are pulled **HIGH** (Logic 1). 
* To send a `0`, a device pulls the line **LOW** (to GND). 
* This prevents short circuits if two devices try to talk at the same time and allows for clock stretching.

---

## 2. The Protocol: The "Read Word" Timing Sequence


When the EC wants to read a 16-bit value (like Voltage or Temperature) from the battery, it executes a strict hardware sequence known as the **SMBus Read Word Protocol**:

1. **START Condition (S):** The EC pulls SDA low while SCL is high. ("Attention everyone!")
2. **Slave Address (Write):** EC sends the 7-bit battery address (e.g., `0x16`) + `0` (Write bit).
3. **ACK:** Battery pulls SDA low to acknowledge.
4. **Command Code:** EC sends the 8-bit SBS Command (e.g., `0x09` for Voltage). Battery ACKs.
5. **Repeated START (Sr):** EC issues another START condition to change data direction.
6. **Slave Address (Read):** EC sends battery address + `1` (Read bit). Battery ACKs.
7. **Data Byte Low (LSB):** Battery sends the lower 8 bits. **EC ACKs**.
8. **Data Byte High (MSB):** Battery sends the upper 8 bits. **EC NACKs** (leaves SDA high) to signal "I have enough data."
9. **STOP Condition (P):** EC releases SDA to high while SCL is high, terminating the bus transaction.

---

## 3. The Firmware Layer: How C Code Drives the Hardware

In a real System-on-Chip (SoC) like an EC, firmware engineers don't toggle pins manually (bit-banging). Instead, we interact with **Memory-Mapped Hardware Registers**. The underlying hardware controller automatically generates the START, STOP, and Clock signals.

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
    SMBUS_ADDR_REG = (slave_addr << 1);     //send slave address to the address register so that the master know where the message go to.
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

    // 5. Merge Little-Endian bytes
    return (uint16_t)((msb << 8) | lsb);
}