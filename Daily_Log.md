# 📅 My EC Learning and Development Log

### 2026-03-04
* **Learning Content**: Hardware I2C vs. Software I2C (Bit-banging). Mastered the architectural difference: Hardware I2C uses dedicated silicon state machines to offload CPU overhead, while Software I2C relies on CPU-blocking delays but offers maximum pin flexibility.
* **Code Progress**: Implemented STM32 bare-metal I2C communication with the MPU6050 sensor. Transitioned from GPIO toggling to hardware register commanding (e.g., `AF_OD`, `I2C_GenerateSTART`). Handled sensor initialization (`PWR_MGMT_1`) and 16-bit data merging (`<< 8`).
* **Encountered Issues**: High-level libraries (SPL/HAL) obscured the true physical layer logic. Overcame this by studying the Reference Manual to manipulate raw registers directly (e.g., `GPIOB->BSRR`) and adding timeout protections to prevent hardware deadlocks.
* **Tomorrow's Plan**: Optimize the I2C read function using "Burst Read" (Sequential Read) to fetch multi-axis data efficiently, and flash the firmware to a physical STM32 board for real-world verification.

### 2026-03-03
* **Learning Content**: Deeply explored the hardware-software boundary of I2C, eSPI, and HMI peripherals. Mastered the physical signaling of I2C (START/STOP/Repeated START), the eSPI Master-Slave handshake mechanism (especially the `Alert#` pin), and the electrical logic of Keyboard Matrix Scanning (KSI/KSO).
* **Code Progress**: 
    * In `smbus_battery`, implemented SMBus battery data parsing with bitwise merging for LSB/MSB.
    * In `i2c_eeprom`, simulated the "Random Read" sequence involving "Dummy Write" to set internal address pointers.
    * In `espi_vw`, constructed Virtual Wire (VW) packets and simulated the hardware register-driven transmission logic.
* **Encountered Issues**: It was challenging to grasp why eSPI Slaves need `Alert#` to "beg" for a clock, and the logic of the I2C "Dummy Write" (switching from Write to Read without a STOP) was initially counter-intuitive. Also, realized that using `volatile` for hardware-mapped registers is crucial to prevent compiler over-optimization.
* **Tomorrow's Plan**: keep studying HMI peripherals and strengthen the true code in different level buses.

### 2026-03-02
* **Learning Content**: Deeply reviewed the logic of thermal management and SMBus.
* **Code Progress**: In `02_Thermal_Management`, completed the test code for `thermal_control.c`, `thermal_control.h` and `main.c`, successfully simulating the thermal control and the speed of fan. As for the SMBus, I simulated the rules of it and used it in `main.c`.
* **Encountered Issues**: I think the thermal control is easy and what I need is using `struct` to read the temperature and accord it to set different speed of fan. When it comes to the SMBus, I reckon it is similar to I2C's sequence.
* **Tomorrow's Plan**: Start learning the sequence of I2C and eSPI.

### 2026-03-01
* **Learning Content**: Deeply reviewed the logic of ACPI's sleep state transitions.
* **Code Progress**: In `01_Power_Management`, completed the test code for `main.c`, successfully simulating the process of waking up the motherboard by pressing the power button.
* **Encountered Issues**: Initially, it's difficult for me to understand the mode change and the settings but as time goes by, I had a good command of it.
* **Tomorrow's Plan**: Start learning the part of thermal control.
