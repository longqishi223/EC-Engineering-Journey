# 📅 My EC Learning and Development Log

### 2026-03-08
* **Learning Content**: Analyzed Software SPI (Bit-banging) physical timing and introduced the HID protocol (Application Layer).
* **Core Concepts Mastered**:
  1. **Software SPI (`SwapByte`)**: Visualized the shift-register ring exchange. Confirmed TX/RX are exactly simultaneous. Mapped the manual GPIO toggling to SPI Mode 0 (CPOL=0, CPHA=0).
  2. **Hardware Multiplexing (`I2SMOD`)**: Discovered STM32 reuses SPI silicon for I2S (digital audio). The `I2SMOD` bit must be forcefully cleared to keep the bus in pure SPI mode.
  3. **The HID Abstraction**: Shifted from physical buses to OS-level protocols. Learned that HID achieves driverless Plug-and-Play via a **Report Descriptor**—a hardcoded contract telling the OS how to parse raw byte arrays.
  4. **EC Relevance**: Hardware SPI (eSPI) is mandatory for high-speed CPU comms. HID over I2C/SPI is the absolute standard for translating laptop hardware (keyboards, hotkeys) into native Windows events.
* **Tomorrow's Plan**: Decode a real-world HID Report Descriptor and map hexadecimal arrays to official USB-IF Usage Tables.

### 2026-03-07
* **Learning Content**: Decoded ST's standard library source code (`stm32f10x_i2c.c`), uncovering 32-bit macro routing tricks, hardware clearing mechanisms.
* **Core Concepts Mastered**:
  1. **32-Bit Macro Routing (`>> 28` & `FLAG_Mask`)**: Discovered how ST packs both the register address tag (SR1 vs. SR2) and the physical bitmask into a single `I2C_FLAG` variable, using bitwise shifts and masks to extract the pure hardware address.
  2. **Interrupt Alignment Magic (`>> 16`)**: Learned that `I2C_IT_XXX` macros embed the `CR2` interrupt enable bit in the high byte. Shifting it right by 16 aligns it perfectly with the physical silicon, allowing dual-checking of state and interrupt switches in one go.
  3. **The `rc_w0` Quirk (Write 0 to Clear)**: Mastered the hardware-level error clearing mechanism (`I2Cx->SR1 = ~flagpos;`). Writing a `0` clears the target bit, while writing a `1` is safely ignored by the hardware, preventing race conditions.
  4. **Clock Stretching & Software Sequences**: Realized that event flags (`ADDR`, `BTF`) cannot be cleared directly. They require strict read/write sequences (e.g., reading SR1 then SR2) to release the SCL line; otherwise, the bus deadlocks.
* **Tomorrow's Plan**: Dissect the `I2C_TypeDef` struct memory mapping logic, or finally write the highly anticipated "Burst Read" algorithm for the MPU6050.

### 2026-03-06
* **Learning Content**: Explored advanced STM32 I2C hardware quirks, interrupt configurations, and register multiplexing logic.
* **Core Concepts Mastered**:
  1. **Hardware Interrupts (`CR2`)**: Configured Event/Error/Buffer interrupts using `|=` and `&= ~` masks, allowing the CPU to avoid blocking `while` loops (Polling) and handle bus states asynchronously.
  2. **The 2-Byte NACK Bug (`CR1_POS`)**: Uncovered the STM32 hardware flaw during 2-byte reception. Used `I2C_NACKPositionConfig` to force the hardware to apply NACK to the *next* byte in the shift register, preventing data overflow.
  3. **Register Multiplexing (SMBus PEC)**: Discovered that hardware designers reuse the exact same `POS` bit for both I2C NACK timing and SMBus PEC (Packet Error Checking). ST uses semantic API wrappers (different function names) for readability.
  4. **Control vs. Data Flow**: Solidified the physical boundary between Control Registers (`CR1/CR2` act as the "driver" issuing internal commands) and Data Registers (`DR` acts as the "cargo" holding actual SDA bus payloads).
* **Tomorrow's Plan**: Translate this low-level mastery into code by writing a highly efficient "Burst Read" (Sequential Read) algorithm for the MPU6050.

### 2026-03-05
* **Learning Content**: Transitioned from STM32 Standard Peripheral Library (SPL) wrappers to bare-metal register operations. Uncovered the physical hardware logic hidden behind the APIs.
* **Core Concepts Mastered**:
  1. **Defensive Programming (`assert_param`)**: Validates inputs (like `I2Cx`) during Debug mode to prevent crashes; compiles to zero overhead in Release mode.
  2. **Memory-Mapped I/O & `volatile` (`__IO`)**: Registers are just absolute memory addresses. The `volatile` keyword forces the CPU to fetch real-time hardware data instead of using cached optimizations.
  3. **Bitwise Masking**: Using precise hex masks (e.g., `& 0xFF01` or `| 0x01`) to safely clear or set specific bits without corrupting adjacent data.
  4. **Read-Modify-Write**: The golden rule for register safety. Read to a temp variable, modify target bits (like Address), and write back to protect unrelated switches (like `OAR2_ENDUAL`).
* **Tomorrow's Plan**: Implement I2C "Burst Read" (Sequential Read) to maximize MPU6050 data fetching efficiency.

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
