# 🔌 Deep Dive: I2C & EEPROM Communication

This folder explores the Inter-Integrated Circuit (I2C) protocol, a fundamental serial communication bus used by the Embedded Controller (EC) to interact with non-volatile memory (EEPROM), USB PD controllers, and various sensors. 

*Reference reading: [The total introduction of I2C on CSDN](https://blog.csdn.net/2301_77220428/article/details/157399020)*

## 1. The Physical Layer & Waveform Mechanics


I2C relies on two open-drain wires: **SDA (Data)** and **SCL (Clock)**, both pulled up to VCC (e.g., 3.3V). The physical signaling is strictly defined by the relationship between these two lines:
* **Data Validity:** The SDA line must remain stable while SCL is HIGH. The state of SDA is only allowed to change when SCL is LOW.
* **START Condition (S):** A unique violation of the data validity rule. A HIGH-to-LOW transition on the SDA line *while* SCL is HIGH signals the beginning of a transaction.
* **STOP Condition (P):** A LOW-to-HIGH transition on the SDA line *while* SCL is HIGH signals the end of a transaction.
* **ACK/NACK (The 9th Clock):** After 8 bits of data are transferred, the transmitter releases the SDA line. During the 9th clock pulse, the receiver must pull SDA LOW to Acknowledge (ACK). If it leaves SDA HIGH, it is a Not-Acknowledge (NACK), often indicating an error or the end of a read sequence.

## 2. I2C vs. SMBus: The Firmware Engineer's Perspective
While SMBus is derived from I2C, EC firmware engineers must treat them differently at the driver level:
* **Timeouts (The Fatal Flaw):** SMBus hardware enforces strict timeouts (~35ms) to prevent bus hangs. Standard I2C has **no timeout limit**. If a slave device glitches and holds SDA low indefinitely, the entire bus freezes. The EC firmware must manually implement a "Bus Clear/Recovery" sequence (clocking SCL 9 times manually via GPIO toggling) to save the system.
* **Speed:** SMBus typically runs at 100 kHz. I2C frequently runs at 400 kHz (Fast Mode) or 1 MHz (Fast Mode Plus).
* **Clock Stretching:** I2C allows slave devices to hold the SCL line low to pause the transaction if they need more time to process data, forcing the Master (EC) to wait.

## 3. The EEPROM Challenge: "Random Read" Sequence


Reading from a specific address in an EEPROM via I2C is a classic interview topic because it requires a complex maneuver called a **Dummy Write**. 

**Why is it needed?** An EEPROM contains an *Internal Address Pointer*. When you want to read from memory address `0x4A`, the EEPROM needs to know that address first. Since I2C doesn't have a dedicated "Set Address" command, the EC must pretend it is going to perform a "Write" operation just to push the `0x4A` into the EEPROM's address pointer, and then abruptly switch to a "Read" operation.

**The Exact Physical Sequence:**
1. `START` (EC claims the bus)
2. `Slave Address + WRITE (0)` -> EEPROM answers `ACK`
3. `EEPROM Memory Address (0x4A)` -> EEPROM answers `ACK` *(This is the Dummy Write setting the internal pointer)*
4. `Repeated START (Sr)` (EC changes direction without releasing the bus)
5. `Slave Address + READ (1)` -> EEPROM answers `ACK`
6. `Data Byte Read` -> EC answers `NACK` *(Master signals no more data needed)*
7. `STOP` (EC releases the bus)

## 4. The Project
This project simulates the exact "Random Read" and "Byte Write" sequences described above, demonstrating how physical I2C waveforms (START, ACK, Repeated START) are abstracted into robust C firmware logic.