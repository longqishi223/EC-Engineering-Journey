# 📅 My EC Learning and Development Log

### 2026-03-13
* **Learning Content**: Transitioned from bare-metal C to FreeRTOS multithreading logic, dissecting the boot sequence, task creation contracts, and atomic hardware operations critical for RTOS thread safety.
* **Core Concepts Mastered**:
  1. **The Startup Sequence (`.s` File)**: Demystified the hardware boot process before `main()`. Understood how the assembly file aggressively initializes the SP (Stack Pointer), copies `.data` to SRAM, and builds the Vector Table. Realized FreeRTOS *must* hijack `SysTick`, `PendSV`, and `SVC` handlers here to enable context switching.
  2. **The RTOS Task Contract (`TaskFunction_t`)**: Dissected why a task must return `void` (a task is an isolated infinite loop; attempting to `return` without a valid LR causes a HardFault) and accept `void *` (enabling universal, polymorphic parameter injection). Verified that function names natively decay into physical Flash addresses passed to the TCB.
  3. **Task Handles & Preemption Hazards**: Differentiated Task Handles (`&xHandleTask1` for strict lifecycle control) vs. `NULL` ("Fire and Forget" mode). Identified the fatal system crash risks of using bare-metal `Delay()` or unthrottled `printf()` inside tasks, which trigger CPU monopolization and instant Stack Overflows.
  4. **Atomic Hardware Bit-Banding (`BSRR`/`BRR` vs `ODR`)**: Analyzed ST's silicon design. Uncovered the fatal Read-Modify-Write (RMW) vulnerability of the `ODR` register in multithreaded environments. Mastered the use of `BSRR` (Set) and `BRR` (Reset) for single-cycle, atomic hardware writes that are immune to RTOS task preemption or interrupt corruption.
  5. **GPIO Independence vs. EXTI Multiplexing**: Clarified the physical routing limits. While GPIO outputs (like PA0 and PB0) are fully independent, hardware external interrupts are strictly multiplexed (e.g., PA0, PB0, PC0 all compete for the single `EXTI0` line, enforcing a "survival of the fittest" rule).
* **Tomorrow's Plan**: Utilize Keil's Logic Analyzer to visually verify the FreeRTOS scheduler's time-slicing (Time Slicing/Context Switching), or dive into IPC (Inter-Process Communication) mechanisms like Queues and Semaphores to sync the tasks.

### 2026-03-12
* **Learning Content**: Transitioned from C-level abstractions to physical memory architecture and successfully built a modern VS Code + Keil MDK hybrid toolchain for upcoming RTOS development.
* **Core Concepts Mastered**:
  1. **Cross-IDE Toolchain Integration**: Hijacked Keil's native `armcc` compiler inside VS Code using the `Keil Assistant` extension. Resolved XML parsing "Silent Crashes" by enforcing a strict English-only, no-space workspace path.
* **Tomorrow's Plan**: Leverage this hybrid workspace to dive into FreeRTOS source code, specifically dissecting `xTaskCreate` and how the OS allocates independent, private stacks for multi-threading.

### 2026-03-11
* **Learning Content**: Transitioned to RTOS fundamentals by deeply dissecting STM32 physical memory addressing, C pointer mechanics, and ARM Linker memory alignment rules.
* **Core Concepts Mastered**:
  1. **Pointer Mechanics & `void *`**: Clarified the critical distinction between *pointer functions* (e.g., `void* my_malloc()` returning a raw address) and *function pointers* (essential for RTOS task scheduling). Mastered `void *` as a universal, size-agnostic memory address contract.
  2. **Physical Byte-Addressing**: Visually verified SRAM (`0x20000000`) in Keil's Memory window. Confirmed that all pointers (regardless of their target type) strictly occupy 4 bytes on a 32-bit CPU.
  3. **Linker Memory Alignment**: Discovered the "invisible hand" of the ARM Linker. It dynamically rearranges global variables (placing a 4-byte `int` before a 1-byte `char` array, ignoring C-code sequence) to enforce strict 4-byte hardware alignment and eliminate memory padding waste.
  4. **Simulator Debugging & Vector Table**: Resolved Keil Simulator `Error 65` (RCC peripheral access violation) by correcting Dialog DLL mappings. Peeked into the hardware Interrupt Vector Table (`0x00000064`) to identify repeated `Default_Handler` function pointers.
* **Tomorrow's Plan**: Initialize a FreeRTOS blank project and apply these memory/pointer concepts to dissect Task Creation (`xTaskCreate`) and individual task stack allocation.

### 2026-03-10
* **Learning Content**: Upgraded the project repository architecture and dissected STM32 SPI Control Register (`SPI_CR1`) macros to map C code directly to physical silicon behaviors.
* **Core Concepts Mastered**:
  1. **Repository Architecture**: Integrated STM32 Bare-Metal, HID Protocol, and AI PC frameworks into the GitHub README, demonstrating full-stack system engineering visibility.
  2. **Hardware Register Mapping**: Translated abstract C macros into physical register bit-masks. Understood how passing `0x0000` forcefully clears target bits in the hardware state machine.
  3. **Master/Slave Selection (`MSTR`)**: Analyzed how `SPI_Mode_Slave` (`0x0000`) clears Bit 2 of `SPI_CR1`, cutting off the internal clock generator and forcing the SCK pin into a passive listening state.
  4. **Data Frame Format (`DFF`)**: Mapped `SPI_DataSize_16b` (`0x0800`) to Bit 11, revealing how toggling a single bit physically expands the shift register from 8 to 16 bits to optimize bandwidth for high-resolution peripherals.
* **Tomorrow's Plan**: Unpack the baud rate/clock prescaler configurations or tear down the underlying bitwise logic (`&=` and `|=`) inside the `SPI_Init()` library function.

### 2026-03-09
* **Learning Content**: Decoded a physical HID Report Descriptor, implemented payload bit-packing in C, and analyzed the AI PC (NPU) hardware architecture.
* **Core Concepts Mastered**:
  1. **Descriptor Decoding**: Translated hex arrays using USB-IF Usage Tables (Tag-Value pairs). Mastered "Bit vs. Byte" packing (`REPORT_SIZE` vs. `REPORT_COUNT`) to achieve extreme bandwidth optimization.
  2. **Payload Bit-Packing**: Wrote C code using bitwise operations (`|= 1 << X`) to strictly align physical hardware states (buttons) with the descriptor's byte contract. 
  3. **Dynamic Sizing (`sizeof`)**: Understood why descriptor length must be calculated dynamically in memory to prevent OS enumeration failures (Error 43).
  4. **AI PC Architecture & NPU**: Contrasted CPU, GPU, and NPU (measured in TOPS). Identified how low-power NPUs disrupt traditional EC design, requiring new PID thermal curves, S0ix wake sequences for "always-on" vision, and Copilot key HID mapping.
* **Tomorrow's Plan**: Explore Vendor-Defined HID descriptors for custom OEM hardware (e.g., hotkeys) or dive into the C-level mathematics of PID thermal control.

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
