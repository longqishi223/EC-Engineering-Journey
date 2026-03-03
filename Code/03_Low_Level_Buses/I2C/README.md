# 🔌 Deep Dive: I2C & EEPROM Communication

This folder explores the Inter-Integrated Circuit (I2C) protocol, a fundamental serial communication bus used by the Embedded Controller (EC) to interact with non-volatile memory (EEPROM), USB PD controllers, and various sensors. 

## 1. I2C vs. SMBus: What's the Difference?
The total introduction of I2C on CSDN: https://blog.csdn.net/2301_77220428/article/details/157399020

While SMBus is derived from I2C, firmware engineers must treat them differently:
* **Timeouts:** SMBus hardware enforces strict timeouts (~35ms) to prevent bus hangs. Standard I2C has **no timeout limit**. If a slave device pulls SDA low indefinitely, the EC firmware must manually implement a "Bus Clear/Recovery" sequence (clocking SCL 9 times).
* **Speed:** SMBus typically runs at 100 kHz. I2C frequently runs at 400 kHz (Fast Mode) or even 1 MHz (Fast Mode Plus).
* **Clock Stretching:** I2C allows slave devices to hold the SCL line low to pause the transaction if they need more time to process data.

## 2. The EEPROM Challenge: "Random Read" Sequence
Reading from an EEPROM via I2C is a classic interview topic because it requires a complex maneuver called a **Dummy Write**. 

To read data from a specific memory address inside the EEPROM, the EC must first perform a "Write" operation just to send the memory address, and then abruptly switch to a "Read" operation using a Repeated START.

**The Sequence:**
1. `START`
2. `Slave Address + WRITE` -> `ACK`
3. `EEPROM Memory Word Address` -> `ACK` *(The Dummy Write)*
4. `Repeated START (Sr)`
5. `Slave Address + READ` -> `ACK`
6. `Data Byte Read` -> `NACK` *(Master signals no more data needed)*
7. `STOP`

This project simulates this exact sequence, demonstrating how physical I2C waveforms are abstracted into robust C firmware.
