# 📅 My EC Learning and Development Log

### 2026-05-15
* **Learning Content**: Transitioned from theory to hands-on practice: created Phase 1 learning plan with 4 coding exercises.
* **Core Concepts Mastered**:
  1. Added `CPRINTS` trace in `set_charge_state()` (`common/charge_state.c:770`) — prints every state transition with AC/battery/SOC context.
  2. Wrote `battery_health` console command (`common/battery.c:330`) — health score (100pts), temperature/voltage/SOC check, GOOD/FAIR/POOR rating.
  3. Prepared breaker exercise: change `BATTERY_LEVEL_SHUTDOWN` from 3 to 50 in `include/battery.h:45`, observe which tests fail.
  4. Read 3 test files: `power_button.c` (GPIO sim + debounce test), `battery_config.c` (CBI battery info), `charge_manager.c` (supplier priority + mock functions).
* **Tomorrow's Plan**: Run all 4 exercises in VM (`make BOARD=host`), verify ccprintf output, fix any compilation issues with `battery_health`, break and restore `BATTERY_LEVEL_SHUTDOWN` test.

### 2026-05-14
* **Learning Content**: Discussed hands-on learning plan and build environment setup.
* **Core Concepts Mastered**:
  1. **Build Environment**: `make BOARD=host -j$(nproc)` compiles EC as Linux native executable. Planning to set up Ubuntu VM for hands-on practice.
* **Tomorrow's Plan**: Set up Ubuntu VM, run `make BOARD=host`, add custom console command.

### 2026-05-13
* **Learning Content**: Studied Chapter 25 (Host Command Protocol deep-dive). Discussed hands-on learning plan and build environment setup.
* **Core Concepts Mastered**:
  1. **EC↔PD MCU via I2C Host Command**: Same Host Command protocol packets over I2C bus, 3 retries on bus error.
  2. **HOOK Mechanism**: State changes broadcast via `hook_notify()` (CHIPSET_STARTUP/RESUME/SUSPEND/SHUTDOWN/HARD_OFF, POWER_BUTTON_CHANGE, AC_CHANGE, BATTERY_SOC_CHANGE).
* **Tomorrow's Plan**: Set up Ubuntu VM, run `make BOARD=host`, add custom console command.

### 2026-05-12
* **Learning Content**: Studied Chapter 25 (Host Command Protocol deep-dive). Discussed hands-on learning plan and build environment setup.
* **Core Concepts Mastered**:
  1. **Power Button Debounce**: 30ms timer-based debounce (`power_button_change_deferred()`), 3-layer protection (hardware Schmitt trigger + s/w debounce + lid-close masking).
  2. **Apollo Lake Power Sequencing**: EC enables PP3300/PP5000 → waits PGOOD → asserts RSMRST# → asserts PWRBTN# → PCH releases SLP_S4#/SLP_S3# → EC enables core voltage → ALL_SYS_PGOOD → SYS_PWROK → PLTRST# → S0.
  3. **Host Command Protocol (V3)**: Request (6B header) + Response (7B header) RPC model. Checksum: all bytes sum to 0. 200+ commands registered via `DECLARE_HOST_COMMAND` with linker-sorted `.rodata.hcmds` section for O(log n) binary search.
  4. **host_command_received() ISR-Task Split**: ISR does minimal work (save args, wake task), except for reboot (immediate) and get_comms_status (immediate). Task does actual command processing.
  5. **host_packet_receive() 6-Layer Defense**: Driver error → too small → too large → wrong version → incomplete data → bad checksum.
* **Tomorrow's Plan**: Set up Ubuntu VM, run `make BOARD=host`, add custom console command.

### 2026-05-11
* **Learning Content**: Completed Chapter 24 (Power Management Subsystem source code line-by-line analysis) and Chapter 26 (build environment setup). Successfully compiled and ran first Chrome EC unit test (`battery_config`) on Ubuntu VM.
* **Core Concepts Mastered**:
  1. **PCH (Platform Controller Hub)**: EC doesn't connect to CPU directly — PCH bridges eSPI/LPC bus, SCI#/SMI# interrupts, and power state management between them.
  2. **Power State Machine (power/common.c)**: 15 states — steady (G3/S5/S4/S3/S0) vs. transition (G3S5/S5S3/...). `chipset_task()` loop: chipset-specific → common state → hook_notify(). ISR only records signals + wakes task; actual state transition happens in task context.
* **Tomorrow's Plan**: Review the 4 compatibility fixes in detail, understand practical value of the build environment for EC development.

### 2026-05-10
* **Learning Content**: Studied Chapter 24 (Power Management Subsystem deep-dive) and Chapter 25 (Host Command Protocol deep-dive). Discussed hands-on learning plan and build environment setup.
* **Core Concepts Mastered**:
  1. **Power State Machine (power/common.c)**: Enum with 15 states split into steady-states (G3/S5/S4/S3/S0) and transition-states (G3S5/S5S3/S3S0...). Signal snapshot mechanism (`power_update_signals()`) captures all GPIO states atomically. Event-driven via `task_wait_event()` + `task_wake()` — no polling.
  2. **chipset_task() Main Loop**: Infinite loop: chipset-specific handling (`power_handle_state()`) → generic steady-state handling (`power_common_state()`) → state change notification (`hook_notify()`).
  3. **Power Button Debounce**: 30ms timer-based debounce (`power_button_change_deferred()`), 3-layer protection (hardware Schmitt trigger + s/w debounce + lid-close masking).
  4. **Apollo Lake Power Sequencing**: EC enables PP3300/PP5000 → waits PGOOD → asserts RSMRST# → asserts PWRBTN# → PCH releases SLP_S4#/SLP_S3# → EC enables core voltage → ALL_SYS_PGOOD → SYS_PWROK → PLTRST# → S0.
  5. **Host Command Protocol (V3)**: Request (6B header) + Response (7B header) RPC model. Checksum: all bytes sum to 0. 200+ commands registered via `DECLARE_HOST_COMMAND` with linker-sorted `.rodata.hcmds` section for O(log n) binary search.
  6. **host_command_received() ISR-Task Split**: ISR does minimal work (save args, wake task), except for reboot (immediate) and get_comms_status (immediate). Task does actual command processing.
  7. **host_packet_receive() 6-Layer Defense**: Driver error → too small → too large → wrong version → incomplete data → bad checksum.
  8. **EC↔PD MCU via I2C Host Command**: Same Host Command protocol packets over I2C bus, 3 retries on bus error.
  9. **HOOK Mechanism**: State changes broadcast via `hook_notify()` (CHIPSET_STARTUP/RESUME/SUSPEND/SHUTDOWN/HARD_OFF, POWER_BUTTON_CHANGE, AC_CHANGE, BATTERY_SOC_CHANGE).
  10. **Build Environment**: `make BOARD=host -j$(nproc)` compiles EC as Linux native executable. Planning to set up Ubuntu VM for hands-on practice.
* **Tomorrow's Plan**: Set up Ubuntu VM, run `make BOARD=host`, add custom console command.

### 2026-05-09
* **Learning Content**: Reviewed charge_state.c and charger driver code, asked several C language syntax questions.
* **Core Concepts Mastered**:
  1. **|= (Bitwise OR Assignment)**: `flags |= BATT_FLAG_BAD_VOLTAGE` preserves existing flags while adding new one. `flags = BATT_FLAG_BAD_VOLTAGE` would overwrite all flags.
  2. **goto Statement**: Unconditional jump in C. Used for error handling cleanup (e.g., `if (ret) goto done`). Common in I2C/SPI read/write functions.
  3. **raw_read16() Return Value**: Returns 0 on success, negative error code (like -EIO) on failure. Not 1.
  4. **register abbreviation**: `reg` in Chrome EC code means "register" - hardware register address or value.
  5. **Function wrapping pattern**: `isl923x_set_voltage()` wraps `isl9237_set_voltage()` - chip series abstraction layer, unified interface for ISL923x family.
* **Tomorrow's Plan**: Continue reviewing document, clarify more C syntax questions as needed.

### 2026-05-08
* **Learning Content**: Reviewed Power/Charger/Battery subsystem (Chapter 20) and added three new chapters (21-23) to learning document. Clarified several C language syntax questions.
* **Core Concepts Mastered**:
  1. **Chrome EC Power/Charger/Battery Architecture**: Four charging states (ST_IDLE/ST_DISCHARGE/ST_CHARGE/ST_PRECHARGE), `charger_task()` main polling loop, `decide_charge_state()` decision logic, `shutdown_on_critical_battery()` with timer-based shutdown.
  2. **Charger Abstraction Layer**: `struct charger` function pointer table, `charger_get_params()` reads charger IC via I2C, `charger_closest_voltage()` clamps to valid range.
  3. **charge_manager Multi-Port Coordination**: Supplier priority array (DEDICATED=0 > PD=1 > Type-C=2), `available_charge[][]` matrix across ports and suppliers.
  4. **memset() Usage**: `memset(ptr, value, num)` fills memory with value. Commonly used to zero-initialize structures: `memset(&chg, 0, sizeof(chg))`.
  5. **Designated Initializer `[index] = value`**: C99 syntax for array initialization. `supplier_priority[CHARGE_SUPPLIER_DEDICATED] = 0` is equivalent to `[10] = 0` in the initializer list. Makes code more readable and order-independent.
  6. **break vs continue in switch**: `break` exits switch only (for loop continues). `continue` in switch jumps to next loop iteration (not directly applicable in switch).
  7. **USB-C PD Protocol**: PDO (Power Data Object) types, Source/Sink negotiation flow, `enum pd_power_role` (PD_ROLE_SINK=0, PD_ROLE_SOURCE=1).
  8. **ISL923x Charger Driver**: Register definitions (CHG_CURRENT, SYS_VOLTAGE_MAX, ADAPTER_CURRENT_LIMIT), `isl923x_set_voltage()`, initialization sequence.
  9. **MAX17055 Fuel Gauge**: I2C battery monitoring IC, `battery_get_params()` reads voltage/current/temperature/SoC, unit conversion formulas (VOLTAGE_CONV, CURRENT_CONV).
* **Tomorrow's Plan**: Continue reviewing document, focus on any knowledge gaps or start practical debugging practice.

### 2026-05-07
* **Learning Content**: Practical hardware I2C debugging with Saleae logic analyzer, STM32 software vs hardware I2C comparison.
* **Core Concepts Mastered**:
  1. **Saleae Logic Analyzer I2C Capture**: Successfully captured STM32 hardware I2C (PB10=SCL, PB11=SDA) communication with MPU6050 using 8MHz sampling rate. Configured I2C protocol analyzer to decode: Start condition → 7-bit device address + R/W bit → ACK → Repeated Start → 8-bit register address → ACK → 8-bit data from slave → ... → Stop.
  2. **I2C Waveform Analysis**: Confirmed MPU6050 default address 0xD0 (write). Observed complete I2C sequence: START → ADDR+W → ACK → RESTART → REGADDR → ACK → DATA → ... → STOP.
  3. **Software vs Hardware I2C**: Software I2C uses GPIO bit-banging (any pin, high CPU overhead) vs Hardware I2C uses dedicated silicon state machine (specific pins, low CPU overhead, more reliable).
  4. **GPIO_Mode_AF_OD**: Hardware I2C must use Alternate Function Open-Drain mode — OD allows multiple devices to pull bus low without short circuit.
  5. **defined() macro**: `#if defined(CONFIG_X)` checks if macro is defined (vs `#ifdef` which is equivalent but less flexible for complex expressions).
* **Tomorrow's Plan**: Continue practical debugging — analyze I2C read/write bursts, verify MPU6050 register values match expected data sheet values.

### 2026-05-06
* **Learning Content**: Deep-dived into Chrome EC GPIO, and SPI/eSPI subsystems.
* **Core Concepts Mastered**:
  1. **GPIO MIWU**: NPCX 192 interrupts via 3 tables × 8 groups × 8 bits. Pending cleared by writing 1 (W1C). `gpio_irq_handlers[]` routes to handlers.
  2. **spi_transaction()**: Uses mutex lock + async pattern. CS controlled via GPIO. `spi_transaction_async()` loops with BSY/RBF polling.
  3. **eSPI 4 channels**: VW (power signals), OOB, Flash, Peripheral. VW signals defined in `enum espi_vw_signal`.
* **Tomorrow's Plan**: Review Flash/Memory (Chapter 19), then start practical debugging practice.

### 2026-05-02
* **Learning Content**: Deep-dived into Chrome EC I2C framework and PWM subsystem.
* **Core Concepts Mastered**:
  1. **i2c_xfer() / i2c_xfer_unlocked()**: Layered I2C transfer with lock protection and 3x NACK retry. State machine: SMB_IDLE → SMB_CONTROLLER_START → SMB_WRITE_OPER → SMB_READ_OPER → SMB_IDLE.
  2. **task_wait_event_mask()**: Task blocks and waits for event. I2C ISR wakes it via `task_set_event()`.
  3. **pwm_set_duty() → pwm_set_raw_duty()**: Converts percentage 0-100 to 16-bit raw 0-65535.
  4. **fan_smart_control()**: Closed-loop RPM with step algorithm (|diff|≥2000→step=20, ≥1000→step=10...).
  5. **Bit mask ops**: `|= (1<<x)` set bit, `&= ~(1<<x)` clear bit.
* **Tomorrow's Plan**: Continue GPIO/MIWU (Chapter 17), then SPI/eSPI (Chapter 18).

### 2026-05-01
* **Learning Content**: Completed all 5 subsystem analyses — I2C, PWM, GPIO, SPI/eSPI, Flash/Memory. Wrote detailed line-by-line function analysis for each subsystem in `ACPI_eSPI_HID_Protocol_Analysis.md` (Chapter 15-19).
* **Core Concepts Mastered**:
  1. **Chrome EC I2C Framework**: Layered architecture (common/i2c_controller.c → chip/npcx/i2c.c). Key functions: `i2c_xfer()`, `i2c_xfer_unlocked()`, `chip_i2c_xfer()`. Uses task blocking (`task_wait_event_mask`) instead of polling. NACK retry up to 3 times.
  2. **PWM/Timer Subsystem**: `pwm_set_duty()` → `pwm_set_raw_duty()`. `fan_smart_control()` with closed-loop RPM control using MFT (Multi-Function Timer). Step algorithm: |diff|≥2000→step=20, ≥1000→step=10, etc.
  3. **GPIO/MIWU Subsystem**: NPCX uses MIWU (Multi-function Input Wake-Up) for 192 GPIO interrupts (3 tables × 8 groups × 8 bits). `gpio_pre_init()` → `gpio_set_flags_by_mask()` → `gpio_interrupt()`. `gpio_wui_table[]` maps signals to MIWU hardware.
  4. **SPI/eSPI**: `spi_transaction()` uses mutex + async pattern. eSPI has 4 channels: VW, OOB, Flash, Peripheral. `espi_vw_set_wire()` writes VWEVSM register. Traditional SPI uses GPIO CS, eSPI embeds CS in protocol.
  5. **Flash/Memory**: RO/RW partition layout. `jump_to_image()` performs sysjump. `system_add_jump_tag()` preserves data across images via `jdata` struct. A/B backup with rollback protection.
* **Tomorrow's Plan**: Summarize all 19 chapters, identify any knowledge gaps, or proceed to practical EC debugging practice.

### 2026-04-30
* **Learning Content**: Continued I2C protocol deep-dive — clarified GPIO mode naming and I2C address-direction mechanics.
* **Core Concepts Mastered**:
  1. **AF_OD (Alternate Function Open-Drain)**: GPIO mode for hardware I2C pins. AF = Alternate Function (pin connects to I2C peripheral), OD = Open-Drain (needs external pull-up). Contrast with Software I2C's GPIO_Mode_Out_OD where CPU manually toggles GPIO registers.
  2. **I2C_Direction_Transmitter**: `I2C_Send7bitAddress()` takes a direction parameter. Hardware I2C automatically encodes the R/W bit into the address byte — no manual bit-shift needed. EV6 confirms address sent + ACK received.
  3. **Device Address vs Register Address**: Distinction between MPU6050_ADDRESS (0xD0, the I2C device address on the bus) and RegAddress (internal register like 0x6B). Write sequence: device address → register address → data.
* **Tomorrow's Plan**: Review Chrome EC I2C driver framework (`i2c_*`), or focus on practical hardware debugging with logic analyzer.

### 2026-04-29
* **Learning Content**: Added Chapter 12 (EC Development Workflow) and Chapter 13 (Power/Charger Subsystem) to `ACPI_eSPI_HID_Protocol_Analysis.md`.
* **Core Concepts Mastered**:
  1. **EC Build System**: `make BOARD=<board_name>` generates `build/<BOARD>/ec.bin` + `ec.elf`. Config hierarchy: `chip → baseboard → board`. Kconfig config files at `common/Kconfig`, `chip/npcx/build.mk`.
  2. **Flash Tools**: `./util/flash_ec --board=<name> --image=<path>` with `servod` running. NPCX uses `flashrom` via servod. Need USB connection: Servo probe → target board → servod on Linux.
  3. **Power State Machine**: `enum power_state { POWER_G3, POWER_S5, POWER_S4, POWER_S3, POWER_S0 }` mapped to ACPI states. `chipset_task()` main loop handles transitions via `power_handle_state()`.
  4. **eSPI VW Power Signals**: `VW_SLP_S3_L`/`S4_L`/`S5_L` defined in `vw_events_list[]` at index 0x02. `SLP_S3_SIGNAL_L` macro conditionally selects VW or GPIO mode. `espi_vw_power_signal_interrupt()` routes to `power_signal_interrupt()`.
* **Tomorrow's Plan**: Review and summarize all 13 chapters, prepare for practical EC development environment setup.

### 2026-04-28
* **Learning Content**: Chapter 10.3-10.4 continued. Deep-dived into MKBP event handling (`mkbp_event.c`) and keyboard matrix scanning subsystem (`keyboard_scan.c`, `keyboard_raw.c`).
* **Core Concepts Mastered**:
  1. **MKBP Event State Machine**: `set_inactive_if_no_events()` clears to INTERRUPT_INACTIVE only when `state.events==0`, calls `mkbp_set_host_active(0)` to notify AP. `force_mkbp_if_events()` is a 1-second safety net that retries up to 3 times if AP fails to respond.
  2. **Ghosting Detection**: `has_ghosting()` uses `x & (x-1)` trick to detect >1 bit set — if non-zero, multiple keys pressed on intersecting rows, which causes phantom key. Returns 1 to reject all key reports when ghosting detected.
  3. **keyboard_scan_task() States**: Wait state (`task_wait_event(-1)`) → Poll mode (`check_keys_changed()` loop) → timeout → back to wait. `force_poll` flag triggers immediate polling. `KEYBOARD_COLUMN_NONE/ALL` control column drive modes.
  4. **KSO/KSI Matrix Scanning**: KSO=Key Scanner Output (column drive), KSI=Key Scanner Input (row read). NPCX has 18 columns split across KBSOUT0 (KSO0-15) and KBSOUT1 (KSO16-17). Rows read via `~NPCX_KBSIN` (inverted, active-low).
* **Tomorrow's Plan**: EC development workflow — build system, flash tools, debug methods.

### 2026-04-27
* **Learning Content**: Chapter 10 source code deep-dive (continued from 04-24). Focused on eSPI VW signal analysis in `chip/npcx/espi.c`. Added Chapter 11 x86 vs ARM architecture overview to `ACPI_eSPI_HID_Protocol_Analysis.md`.
* **Core Concepts Mastered**:
  1. **NPCX_HIPMST(n) PMC Status Register**: Host Interface PMC Status register at offset `PM_CH_BASE_ADDR(n)+0x000`. Key bits: IBF(0x02)=Input Buffer Full, FRMH(0x08)=Command vs Data, F0(bit2)=Processing Flag, ST1/ST2(bit5/6)=SCI/SMI flags. Used for EC-Host handshake via SET_BIT/CLEAR_BIT.
  2. **PMC (Power Management Controller)**: ACPI-spec-defined channel for Host→EC command interface. Chrome EC uses `lpc_pmc_ibf_interrupt()` to handle ACPI commands, distinguished from Host Command channel (PMC_CHAN_2). FRMH bit indicates first byte is command.
  3. **eSPI VW Signal Mechanism**: `espi_vw_set_wire()` writes EC→Host via VWEVSM registers (with DIRTY polling). `espi_vw_get_wire()` reads Host→EC via VWEVMS registers. `vw_events_list[]` maps `{signal_name, evt_idx, evt_val}` for all 20 VW signals.
  4. **eSPI PLTRST# Handler**: `espi_vw_evt_pltrst()` calls `host_register_init()` on deassert and triggers `HOOK_CHIPSET_RESET` deferred call on assert. Updates AP boot time tracking.
* **Tomorrow's Plan**: Practical EC development workflow — build system, flash tools, debug methods.

### 2026-04-24
* **Learning Content**: Completed Chapter 10 of ACPI_eSPI_HID_Protocol_Analysis.md — line-by-line source code analysis of 7 core .c files.
* **Core Concepts Mastered**:
  1. **ACPI `acpi_read()` Burst Cache**: `memcpy(acpi_read_cache.data, memmap_addr, ACPI_VALID_CACHE_SIZE(addr))` pre-fetches up to 4 bytes. Cache miss when `start_addr > addr` or `addr - start_addr >= 4`. `ACPI_VALID_CACHE_SIZE` calculates actual copy size to prevent OOB.
  2. **eSPI VW Signal Access**: `espi_vw_set_wire()` writes output via VWEVSM register (EC→Host). `espi_vw_get_wire()` reads input via VWEVMS register (Host→EC). Both map signal enums through `vw_events_list[]` lookup table.
  3. **MKBP Interrupt State Machine**: `activate_mkbp_with_events()` manages `INTERRUPT_INACTIVE → INACTIVE_TO_ACTIVE → ACTIVE` transitions. Uses `interrupt_id` to handle race conditions. Four notification paths: GPIO / HOST_EVENT / HECI / CUSTOM.
  4. **Matrix Scan Flow**: `keyboard_scan_task()` → `check_keys_changed()` → `read_matrix()` → `keyboard_raw_drive_column()/keyboard_raw_read_rows()`. Debounce uses `debouncing[]` array and `scan_edge_index[][]` timestamps.
  5. **Ghost Detection**: `has_ghosting()` uses "x & (x-1)" trick — if `common & (common-1)` is non-zero, multiple keys pressed on intersecting rows (ghosting).
* **Tomorrow's Plan**: Practical EC development workflow (build/flash/debug).

### 2026-04-23
* **Learning Content**: Reviewed ACPI_eSPI_HID_Protocol_Analysis.md up to line 1200, covering Host Command and Hook/Task systems (extensions of ACPI/eSPI topics).
* **Core Concepts Mastered**:
  1. **DECLARE_HOST_COMMAND Macro**: Uses `__attribute__((section(".rodata.hcmds.xxx")))` to place each command struct in its own section, collected by linker into `__hcmds[]` array. The `##` operator concatenates tokens, `#` stringifies.
  2. **Host Command Dispatch**: `host_packet_receive()` validates checksum, `host_command_received()` routes to handler, `host_command_task()` processes async commands via `EC_RES_IN_PROGRESS`. LPC channel = PMC_CHAN_2 (vs ACPI's PMC_CHAN_1).
  3. **Hook System**: `DECLARE_HOOK()` places callbacks in `.rodata.{hook_type}` sections, `hook_notify()` calls them in priority order. `DECLARE_DEFERRED()` schedules delayed callbacks via `hook_call_deferred()`.
  4. **Task-Wake Mechanism**: `task_wake()` sends `TASK_EVENT_WAKE`, `task_wait_event(-1)` blocks indefinitely until event arrives. Cooperative scheduling — tasks must yield via `task_wait_event()`.
* **Tomorrow's Plan**: Continue reviewing document from line 1200 (remaining Hook/Task + Appendix).  

### 2026-04-22
* **Learning Content**: Reviewed generated `ACPI_eSPI_HID_Protocol_Analysis.md` document up to line 800 (ACPI + eSPI sections).
* **Core Concepts Reinforced**:
  1. **ACPI Burst Mode**: 4-byte read cache pre-fetches consecutive memmap data. `EC_CMD_ACPI_BURST_ENABLE` returns 0x90 (ACPI 5.0 ACK). Safety deferred function auto-disables after 1 second.
  2. **VWEVSM DIRTY Bit**: After EC writes VW signal, must poll DIRTY bit (bit16) until Host clears it to confirm receipt.
  3. **MIWU Interrupt Handling**: `PND` register reads pending bits, `PCL` register clears them (write-1-clear pattern).
  4. **`!!` Operator**: Double negation converts any value to strict 0 or 1 boolean.
* **Tomorrow's Plan**: Continue reviewing document from line 800 (HID/MKBP section + appendix).

### 2026-04-20
* **Learning Content**: Completed HID (MKBP) protocol study and generated comprehensive ACPI/eSPI/HID learning document.
* **Core Concepts Mastered**:
  1. **MKBP Three-State Interrupt Machine**: `INTERRUPT_INACTIVE → INACTIVE_TO_ACTIVE → ACTIVE`. `activate_mkbp_with_events()` manages transitions. Four notification paths: GPIO / HOST_EVENT (SCI) / HECI / CUSTOM.
  2. **Matrix Scanning (`read_matrix()`)**: Hardware layer drives KSO columns low via `KBSOUT` register, reads KSI rows via `KBSIN` register. Debounce logic uses `debounced_state[]` array with configurable down/up thresholds (9ms/30ms).
  3. **Host Command System**: `DECLARE_HOST_COMMAND()` registers handlers to `__hcmds` section. Async commands return `EC_RES_IN_PROGRESS`, task wakes via `TASK_EVENT_CMD_PENDING`. LPC channel = `PMC_CHAN_2`.
  4. **Power State Machine**: `chipset_task()` loop calls `power_handle_state()` per chipset. SLP_S3/S4/S5 signals monitored via GPIO or VW depending on platform.
* **Tomorrow's Plan**: Review generated document and continue with Host Command protocol deep-dive if needed.

### 2026-04-19
* **Learning Content**: Deep-dived into ACPI source code and complete eSPI+ACPI communication chain.
* **Core Concepts Mastered**:
  1. **ACPI Command State Machine**: `acpi_ap_to_ec()` handles 5 commands (READ/WRITE/QUERY_EVENT/BURST_ENABLE/DISABLE). `acpi_data_count` tracks state.
  2. **PMC Channel & IBF**: NPCX has two PMC channels (ACPI/HostCmd). IBF flag triggers `lpc_pmc_ibf_interrupt()` to dispatch.
  3. **64-bit Host Event System**: `host_set_single_event()` sets bits. `lpc_update_host_event_status()` checks SMI/SCI/WAKE masks and generates interrupts.
  4. **SCI# via eSPI VW**: On eSPI platforms, SCI# is a VW packet, not a GPIO.
* **Tomorrow's Plan**: Continue ACPI power state transitions (`power/` module). Then HID protocol.

### 2026-04-18
* **Learning Content**: Deep-dived into Chrome EC eSPI source code — from high-level `common/espi.c` abstraction down to NPCX chip-level hardware driver `chip/npcx/espi.c` (744 lines).
* **Core Concepts Mastered**:
  1. **eSPI Two-Layer Architecture**: `common/espi.c` handles only signal naming (`espi_vw_names[]`) and range validation (`espi_signal_is_vw()`) with zero hardware register operations; all register-level logic lives in `chip/npcx/espi.c`. This layered design decouples upper-layer protocol logic from chip-specific implementations.
  2. **VW Signal Three-Field Mapping Table (`vw_events_list[]`)**: Each VW signal is described by a `{name, evt_idx, evt_val}` triplet. `evt_idx` is the eSPI-spec-defined hardware register index (0x02/0x03...), while `evt_val` specifies the exact bit position within that index's 4-bit WIRE field (bit0=0x01, bit1=0x02...). This explains why SLP_S3/S4/S5 share index 0x02 but occupy different bits.
  3. **Input vs Output Register Division**: `VWEVMS` (Input/Host→EC) stores VW signals sent by the Host, read by EC via `espi_vw_get_wire()`; `VWEVSM` (Output/EC→Host) stores VW signals sent by EC, requiring polling of the `DIRTY` bit to wait for hardware acknowledgment after writing.
  4. **MIWU Interrupt Routing**: NPCX's Multi-function Input Wake-Up module maps hardware pin changes to VW events. `espi_vw_int_list[]` defines the MIWU table/group/num triplet for each VW signal, ultimately dispatched through `__espi_wk2a_interrupt()` and `__espi_wk2b_interrupt()` ISRs to specific event handlers.
  5. **Event Handlers and ACPI State Machine Integration**: Each VW signal transition (e.g., `espi_vw_evt_slp_s3()`) calls `power_signal_interrupt()` to forward the signal to the `power/` module, triggering the ACPI power state machine. Events like OOB_RST/SUS_WARN also automatically trigger ACK back transmissions via `espi_vw_set_wire()`.
  6. **Compile-Time Safety Check**: `BUILD_ASSERT(ARRAY_SIZE(espi_vw_names) == VW_SIGNAL_COUNT)` ensures the enum count strictly matches the string array — any mismatch causes an immediate build failure.
* **Tomorrow's Plan**: Continue deep-diving into ACPI source code implementation, tracing the memory space read/write handlers in `common/acpi.c` through to chip-level hardware, establishing a complete chain from ACPI commands to eSPI VW signals.

### 2026-04-17
* **Learning Content**: Deep-dived into ACPI UART command parsing state machine, ASCII character handling, and identified a boundary protection bug.
* **Core Concepts Mastered**:
  1. **ACPI Core Concepts**: LID (Laptop lid switch, 0=closed 1=open), AC (AC adapter connection, 1=connected), SCI (System Control Interrupt for power events), `\0` (C string terminator).
  2. **UART Command Parsing State Machine**: Three states (IDLE→READ_CMD→READ_PARAMS), first comma skips `$ACPI,` prefix, second comma marks command end, `break` exits entire switch block.
  3. **Boundary Protection Bug**: When `gUartParamIdx >= ACPI_MAX_PARAM`, code overwrites slot 9 instead of discarding. Correct fix: check `if (gUartParamIdx < ACPI_MAX_PARAM)` before storing.
  4. **ASCII Filtering & Hex Escaping**: Printable range `0x20`-`0x7E`; non-printable bytes converted to hex via nibble extraction (`>> 4`, `& 0x0F`) and ternary conversion `(n<10)?'0'+n:'A'+n-10`.
  5. **Enum vs Struct**: Enum used directly without instantiation; struct requires creating instance first, then access via `.`; pointer uses `->` operator.
* **Tomorrow's Plan**: Continue ACPI protocol analysis, study `ACPI_ParseCommand` execution flow and SCI event triggering mechanism.

### 2026-04-16
* **Learning Content**: ACPI protocol emulation environment setup and UART command parsing state machine debugging.
* **Core Concepts Mastered**:
  1. **ACPI Emulation Architecture**: Built EC-to-Host command response protocol via UART on FreeRTOS. Established `$ACPI,CMD` command format and `$ACK,ret`/`$EVT,event,data` response format.
  2. **UART Command Parsing State Machine**: Implemented character-level state machine with `\r\n` terminator recognition for command boundary detection. Uses ring buffer for interrupt-driven RX data consumption.
  3. **Edge Detection & Event Notification**: SCI events use edge detection (comparing last state vs current state) to avoid duplicate notifications.
  4. **Protocol Debugging Insight**: Terminal input must send `\r\n` to be recognized correctly, matching real ACPI hardware's SCI edge-triggered mechanism.
* **Tomorrow's Plan**: Deep-dive into ACPI specific function implementations, understand real EC hardware SCI interrupt mechanism and power state transition logic.

### 2026-04-15
* **Learning Content**: Deep-dived into HID Report Descriptor encoding mechanics, Global vs. Main Item state machine, and USB property/protocol layer architecture (`usb_prop.c`).
* **Core Concepts Mastered**:
  1. **Global Items Persistence & State Machine**: Grasped the fundamental architectural difference between Global and Local items. Global items (e.g., `Usage Page`, `Report Size`, `Report Count`) establish persistent state that governs all subsequent Main items until explicitly reconfigured, creating a true state machine within the descriptor array. This explains why identical prefixes (like `0x75` and `0x95`) can represent radically different physical quantities at different positions.
  2. **The True Nature of REPORT_SIZE vs. REPORT_COUNT**: Demystified the comment error in `usb_desc.c`. `0x95` (Report Count) represents a raw **quantity** (number of fields), not a bit-width. The total bit-width is the mathematical product `REPORT_SIZE × REPORT_COUNT`. For example, `Report Size = 1 bit` combined with `Report Count = 8` yields exactly 8 bits (1 byte), not "8 bits" as the misleading comment suggested.
  3. **USB Device Property Table Architecture**: Deconstructed the `Device_Property` struct as the "callback function pointer table" pattern. The USB library acts as a passive router, matching incoming standard/class requests (GET_DESCRIPTOR, SET_CONFIGURATION) against this table and dispatching to the appropriate handler function (e.g., `CustomHID_Data_Setup`, `CustomHID_Reset`).
  4. **Chip Unique ID & Serial Number Generation**: Traced the physical STM32 die-level 96-bit unique identifier (`0x1FFFF7E8`-`0x1FFFF7F0`) and understood how `Get_SerialNum()` converts this into UTF-16LE Unicode strings for the USB string descriptor, enabling per-device serial numbers for inventory tracking and driver signing.
  5. **Endpoint Initialization & USB Packet Memory Area (PMA)**: Analyzed the `CustomHID_Reset()` function's dual-endpoint initialization. ENDP0 (Control) uses bidirectional TX/RX with dedicated PMA buffers, while ENDP1 (Interrupt IN for keyboard) uses a unidirectional TX-only path with `EP_TX_NAK` default state—signaling to the host that the buffer is "not yet ready" until `SetEPTxValid()` is called.
  6. **HID Class-Specific Request Dispatching**: Traced the filtering logic in `CustomHID_Data_Setup()` that decodes the USB request fields (`USBbmRequestType`, `USBwValue1`, `USBwIndex0`) to disambiguate between standard requests (GET_DESCRIPTOR) and HID class requests (GET_PROTOCOL), routing each to the appropriate descriptor copy routine.
* **Tomorrow's Plan**: Continue hardware debugging of the PA12 pull-up resistor. Once USB enumeration succeeds, proceed to integrate the HID keyboard report transmission into the FreeRTOS task architecture. Begin analyzing the STM32 USB interrupt routing layer (`usb_istr.c`) to understand how physical USB events trigger firmware callbacks.

### 2026-04-14
* **Learning Content**: STM32 USB-FS-Device Library Architecture, HID Protocol Fundamentals, Report Descriptors, and Hardware Pull-up Debugging.
* **Core Concepts Mastered**:
  1. **USB Hardware Enumeration Roadblock (PA12 Pull-up)**: Encountered a physical layer enumeration failure where the host PC could not recognize the STM32 USB device. Pinpointed the root cause to the absence/misconfiguration of the mandatory 1.5kΩ pull-up resistor on the USB D+ line (PA12), which is required to signal a Full-Speed device to the host. Pivoted to a software-first code analysis strategy while pending hardware correction.
  2. **STM32 Standard Peripheral Library (USB-FS) Architecture**: Deconstructed the legacy STM32 USB-FS-Device library tree. Mapped the separation of concerns between application-layer implementation (`usb_prop.c`, `usb_desc.c`) and lower-level interrupt routing. Recognized the absence of high-level abstraction wrappers (like modern HAL's `USBD_HID_SendReport`) in favor of direct PMA (Packet Memory Area) buffer manipulation and endpoint status toggling (`SetEPTxValid()`).
  3. **HID Protocol Anatomy (Report Descriptors)**: Deep-dived into `usb_desc.c` to decode the HID Report Descriptor (`CustomHID_ReportDescriptor` / `Joystick_ReportDescriptor`). Grasped the fundamental concept that this array acts as the device's "instruction manual," using specific Tags to define data packet structures (e.g., bitfields for button states, bytes for axis offsets), allowing the host OS to natively parse the payload without requiring custom drivers.
  4. **Endpoint Communication & Interrupt Routing**: Traced the USB data flow architecture. Analyzed `usb_endp.c` for IN endpoint callbacks (e.g., `EP1_IN_Callback`) which act as the "tollbooth" triggering upon successful packet transmission. Mapped the hardware interrupt routing through `usb_istr.c` (Interrupt Service Routine), which acts as the central dispatcher for all USB bus events (reset, suspend, data transfer).
  5. **Conceptual Bridge to EC Engineering**: Contextualized the current HID deep-dive within the broader PC architecture required for the upcoming Embedded Controller (EC) role. Established a baseline understanding of how HID (often mapped over I2C or USB in laptops) interacts with the system, laying the groundwork for upcoming studies into ACPI (SCI power management interrupts) and eSPI (Virtual Wires replacing legacy LPC).
* **Tomorrow's Plan**: Resolve the hardware PA12 1.5kΩ pull-up issue to successfully enumerate the STM32 on the host PC. Deploy Wireshark with the USBPcap plugin to capture and analyze the raw HID enumeration handshake and hex data packets. Begin writing the application-layer code to map RTOS-managed physical button presses into the USB PMA buffer for endpoint transmission.

### 2026-04-13
* **Learning Content**: FreeRTOS IPC (Queues & Mutex), Interrupt Management (FromISR), Stack Memory Tuning, and Event-Driven UI State Machine.
* **Core Concepts Mastered**:
  1. **Thread-Safe IPC (Message Queues)**: Eradicated global variable data races by replacing `g_temp_c` with FreeRTOS Queues. Implemented a "Producer-Consumer" model using `xQueueOverwrite` for the high-frequency Thermal task (20ms) and `xQueueReceive` with `portMAX_DELAY` for OLED/UART tasks. This achieved a pure event-driven architecture, ensuring 0% CPU waste when no new data is pending.
  2. **Resource Protection (Mutex)**: Secured the UART telemetry stream against task preemption collisions. Engineered a robust `Safe_Printf` wrapper utilizing FreeRTOS Mutex, `<stdarg.h>`, and `vprintf` to guarantee atomic serial transmissions. Resolved implicit C89 declaration bugs by strictly managing `semphr.h` and `configUSE_MUTEXES`.
  3. **RTOS Interrupt Architecture (EXTI & FromISR)**: Mastered the strict hardware/software boundary in Cortex-M3. Bridged a physical EXTI4 button press to a task-level state switch using `xSemaphoreGiveFromISR` and `portYIELD_FROM_ISR` for instant preemption. Successfully bypassed the notorious "SysCall Priority Assertion" trap by enforcing `NVIC_PriorityGroup_4` and assigning a safe hardware preemption priority (Level 5). Implemented non-blocking RTOS-tick debouncing via `xTaskGetTickCount()`.
  4. **Memory Profiling & Stack Overflow Resolution**: Diagnosed and resolved a fatal `HardFault` triggered by a stack overflow in the OLED task. Learned the critical 32-bit architecture distinction where 1 Word = 4 Bytes. Rescued the system from the 512-byte limit crash (caused by string constants and branching) by surgically expanding the task's stack to 256 words (1024 bytes).
  5. **I2C Bus Optimization & UI State Machine**: Architected an edge-triggered UI state machine (`volatile uint8_t g_display_mode`). Eliminated I2C bus congestion and visual "screen tearing" by decoupling static UI rendering (drawn only once per mode switch via a `last_mode` shadow variable) from high-frequency dynamic data updates (refreshed every 20ms).
* **Tomorrow's Plan**: Conduct a long-term stability burn-in test. Evaluate transitioning the UART task into a FreeRTOS Software Timer callback to reclaim another 1KB of SRAM, and prepare for the physical hardware upgrade to the 25kHz Intel PWM specification for the 4-wire industrial fan.

### 2026-04-12
* **Learning Content**: FreeRTOS Kernel Surgical Migration, ARM Cortex-M3 Porting, and Preemptive Task Scheduling.
* **Core Concepts Mastered**:
  1. **Surgical Kernel Extraction**: Optimized the FreeRTOS integration by selectively extracting only 6 core files (`tasks.c`, `list.c`, `queue.c`, `port.c`, `heap_4.c`, `FreeRTOSConfig.h`) from the official source tree. Manually mapped the `RVDS/ARM_CM3` portable layer to bridge the STM32 hardware abstraction with the RTOS scheduler, achieving a minimalist and high-performance kernel footprint.
  2. **Vector Table Harmonization**: Resolved the architectural "naming gap" between ST’s standard startup code and the FreeRTOS kernel. Successfully remapped `SVC_Handler`, `PendSV_Handler`, and `SysTick_Handler` via C-preprocessor defines, effectively handing over the CPU’s "heartbeat" control from the legacy `systick.c` to the kernel's preemptive tick handler.
  3. **Heap_4 Memory Strategy**: Implemented the `heap_4.c` allocation algorithm, enabling a robust, thread-safe memory management scheme with automatic block coalescing. This prevents heap fragmentation, ensuring long-term stability for dynamic task creation and the system's internal stack allocation.
  4. **Preemptive Task Decoupling**: Transformed the manual non-blocking state machine into a true multi-tasking environment. Leveraged `vTaskDelayUntil()` for the High-Priority Thermal Task (20ms) to achieve deterministic, jitter-free control, while assigning lower priorities to OLED UI (100ms) and UART Telemetry (500ms), allowing the RTOS to perform real-time context switching and resource arbitration.
* **Tomorrow's Plan**: Transition from global variables to **FreeRTOS Queues** for thread-safe Inter-Task Communication (ITC). Begin implementing the Intel 25kHz PWM specification for the 4-wire industrial fan to replace the SG90 servo.

### 2026-04-11
* **Learning Content**: STM32 Hardware Timers (PWM), UART Telemetry, and Cortex-M3 SysTick Non-Blocking State Machine.
* **Core Concepts Mastered**:
  1. **Hardware Timer & PWM Slicing**: Configured `TIM3` to generate a precise 50Hz PWM signal for the SG90 servo actuator. Mastered the math behind the Prescaler (PSC) and Auto-Reload Register (ARR), slicing the 72MHz core clock down to a 1-microsecond resolution (`PSC = 71`) to accurately control physical servo angles via absolute microsecond pulse widths.
  2. **UART Telemetry & Printf Redirection**: Broke the microcontroller's "mute" state by configuring `USART1` (PA9/PA10) at 115200 bps. Implemented `fputc` redirection to hijack the standard C library's `printf` stream, enabling real-time data logging to a PC terminal. Successfully bypassed the Keil "Semihosting" hard-fault trap by enabling MicroLIB.
  3. **SysTick Non-Blocking Scheduler (RTOS Prelude)**: Completely eradicated blocking `Delay_ms()` calls from the super-loop. Awakened the Cortex-M3's internal SysTick hardware to generate a 1ms system heartbeat. Architected a time-sliced, non-blocking state machine that safely decouples execution frequencies: Thermal Control (20ms), OLED UI (100ms), and UART Logging (500ms), achieving 100% CPU utilization without task starvation.
  4. **Multi-Step Bang-Bang Control**: Designed a highly efficient discrete state controller using nested ternary operators `(temp > 32) ? 180 : (temp > 29) ? 90 : 0`. This proves optimal for actuators requiring distinct physical thresholds (like the SG90 servo) rather than continuous proportional curves.
* **Tomorrow's Plan**: Finalize the hardware transition from the SG90 servo to a 4-Wire PC Fan, implementing the Intel 25kHz PWM specification. Once the physical thermal loop is validated, prepare to inject FreeRTOS to replace the manual SysTick state machine with a true preemptive multitasking kernel.

### 2026-04-10
* **Learning Content**: STM32F103 Bare-Metal ADC & NTC Thermistor Integration, Integer-based Signal Processing, and Toolchain Synchronization (VS Code + Keil MDK).
* **Core Concepts Mastered**:
  1. **Non-Linear Sensor Calibration (LUT & Interpolation)**: Discarded computationally expensive and physically inaccurate floating-point linear equations for the NTC thermistor. Implemented a lightning-fast, integer-only Lookup Table (LUT) with piecewise linear interpolation to perfectly map the logarithmic Steinhart-Hart curve, guaranteeing precise thermal tracking without FPU overhead.
  2. **Deterministic Signal Filtering & Resolution Deadbands**: Mitigated high-frequency electromagnetic noise by implementing an 8-sample sliding average filter. Utilized bitwise shifting (`>> 3`) instead of hardware division to preserve Cortex-M3 clock cycles. Leveraged the natural "resolution deadband" between ADC steps and integer temperatures to achieve absolute output stability, preventing future PID-induced acoustic fan jitter.
  3. **Silicon-Level Pin Binding & Macro Safety**: Resolved a critical "0℃ output" bug caused by passing `GPIO_Pin_0` (0x01) instead of `ADC_Channel_0` (0x00). Realized that analog input channels are hardwired at the silicon level and cannot be arbitrarily remapped like digital peripherals. This macro mismatch accidentally pointed the ADC to a floating pin, triggering the LUT's open-circuit safety net.
  4. **Modern Toolchain Architecture**: Successfully integrated VS Code (for LSP-based IntelliSense) with the Keil MDK compiler. Solved cross-directory `#include` failures by aligning Keil's internal "Include Paths" with VS Code's `c_cpp_properties.json` (`${workspaceFolder}/**`), achieving a "dual-wielding" workflow combining modern code editing with raw register-level hardware debugging.
* **Tomorrow's Plan**: Tackle the STM32 Hardware Timer (`TIM2`). Configure the Pre-scaler (PSC) and Auto-Reload Register (ARR) to generate a mathematically precise 25kHz PWM signal compliant with Intel's 4-wire fan specification, establishing the core actuator for the EC thermal control loop.

### 2026-04-09
* **Learning Content**: FreeRTOS Internals: `heap_4.c` Physical Memory Allocation & Kernel Double-Linked List (`list.c`) Architecture.
* **Core Concepts Mastered**:
  1. **Heap_4 Alignment & Coalescence**: Uncovered the hidden `BlockLink_t` overhead (the "property deed") prepended to every allocation. Mastered the bitwise masking technique `(x + (ALIGN - 1)) & ~MASK` that mathematically forces hardware-safe byte alignment without branching. Understood the algorithmic "stitching" of adjacent RAM blocks by physical address to permanently defeat memory fragmentation.
  2. **Real-Time Concurrency in Allocation**: Analyzed why `heap_4.c` uses `vTaskSuspendAll()` instead of disabling global interrupts (`taskENTER_CRITICAL()`) during memory allocation. This brilliant design protects the heap linked list from concurrent task access while guaranteeing zero-latency response for hardware interrupts.
  3. **The "Closed-Loop Sentinel" List Architecture**: Deconstructed the physical anatomy of `list.c`. Realized that FreeRTOS lists are circular and initialized with an indestructible `xListEnd` sentinel node holding the maximum value (`portMAX_DELAY`). This creates a perfect topological ring, completely eliminating `NULL` pointer boundary checks and saving crucial CPU branch-prediction cycles.
  4. **O(1) Physical Extraction & Dangling Pointer Defense**: Mastered how nodes track their parent lists via `pxContainer`, allowing `uxListRemove()` to perform instantaneous physical extraction without O(N) traversal. Discovered the vital `pxIndex` fallback logic (`if (pxIndex == pxItemToRemove) pxIndex = pxPrevious`) that prevents catastrophic Hard Faults and preserves fair Round-Robin scheduling when a currently active task is destroyed.
* **Tomorrow's Plan**: Descend into the core Task Scheduler (`tasks.c`). Trace the complete life cycle of a task, deconstruct the `vTaskDelay` blocking mechanism, and analyze exactly how `pxCurrentTCB` is manipulated during the `PendSV` hardware exception to achieve true deterministic preemption.

### 2026-04-08
* **Learning Content**: Physical Memory Foundations: TCB vs. Stack Isolation, 32-Bit Bus Throughput & Hardware Alignment Constraints.
* **Core Concepts Mastered**:
  1. **Physical Decoupling of TCB & Stack**: Shattered the misconception that the Stack resides directly adjacent to or inside the TCB. In physical RAM, the TCB is merely a control structure (the "property deed") locked within a kernel list, while the Stack is an independently allocated memory block (the "house"). `pxStack` does not store TCB data; it is purely a pointer *inside* the TCB holding the lowest physical coordinate of the stack's foundation.
  2. **Byte-Addressability vs. 32-Bit Throughput**: Mastered the realities of a 4GB address space. Memory consists of sequentially numbered 8-bit drawers, but a 32-bit CPU utilizes a 4-byte-wide physical data bus. While the CPU can logically read/write a single byte using hardware Byte Enable masks, the physical transaction generally activates a full 4-byte channel simultaneously.
  3. **The Penalty of Unaligned Memory Access**: Uncovered the hardware origin of strict memory alignment (e.g., 4-byte boundaries). A 32-bit data bus is physically engineered to snap to aligned addresses like `0x0000`, `0x0004`, `0x0008`. An unaligned access (e.g., fetching a 32-bit integer starting at `0x0001`) forces the hardware to perform two separate boundary-crossing fetch cycles and internally stitch the data, severely degrading performance.
  4. **The Read-Modify-Write (RMW) Pipeline**: Analyzed the non-atomic nature of bitwise operations. Since standard memory cannot be addressed bit-by-bit (excluding specific hardware like Cortex-M Bit-banding), modifying a single bit requires a three-step physical sequence: load the entire byte into a register, apply an ALU mask, and write the full byte back. This physical reality dictates exactly why Critical Sections are mandatory to protect variables from concurrent ISR corruption.
* **Tomorrow's Plan**: Dive directly into the FreeRTOS memory allocator `heap_4.c`. Deconstruct the `BlockLink_t` node structure, explore how the OS enforces strict byte alignment boundaries using bitwise masking (`x & ~portBYTE_ALIGNMENT_MASK`), and master the underlying algorithm used to stitch fragmented RAM blocks back together.

### 2026-04-07
* **Learning Content**: C Language Physical Memory Mapping (Flash vs. RAM), The Duality of `static`, and FreeRTOS Mutual Exclusion (Hardware vs. Logical Locks).
* **Core Concepts Mastered**:
  1. **The True Nature of "Handles"**: A Handle (e.g., `TimerHandle_t`) is physically just a disguised `void *` pointer. It holds the starting address of a dynamically allocated RAM block (Heap). Evaluating `if(xTimer)` simply checks if the pointer is non-zero (i.e., RAM allocation was successful, not `NULL`).
  2. **The Duality of `static` (Storage vs. Linkage)**: 
     * On **Variables**: Alters *Storage Duration*. It moves the variable from the temporary Stack into the permanent Static RAM area (.data/.bss), making its lifespan match the system's uptime.
     * On **Functions**: Alters *Linkage Visibility*. Functions permanently reside in Flash. `static` here acts as a "physical cloak," making the function strictly Private to the current `.c` file and invisible to others.
  3. **The Absolute Physical Memory Map**: 
     * **Flash (ROM)**: Non-volatile. Stores executable machine code (.text), read-only constants (`const`), and initializers for static variables.
     * **RAM**: Volatile. The CPU's workbench. Contains the Stack (locals), Heap (dynamic allocations), and BSS/Data segments. Startup assembly code (`.s`) is responsible for copying initializers from Flash to RAM before `main()` executes.
  4. **Critical Sections vs. Scheduler Suspension (Debunking Doc Errors)**:
     * **Critical Section (`taskENTER_CRITICAL`)**: A brutal hardware lock. Modifies the CPU's `BASEPRI` register to physically mask low-priority hardware interrupts, inherently killing SysTick and PendSV (stopping all scheduling).
     * **Scheduler Suspension (`vTaskSuspendScheduler`)**: A gentle logical lock. Hardware interrupts (ISRs) still fire normally, but the kernel places a "do not switch" flag, preventing context switches until resumed. These two locks possess independent nesting counters and must never be conceptually mixed.
* **Tomorrow's Plan**: Capitalize on today's RAM architecture mastery to conquer FreeRTOS Memory Management. Deconstruct `pvPortMalloc` and analyze the surgical precision of `Heap_1` through `Heap_5` allocation algorithms.

### 2026-04-06
* **Learning Content**: Software Timers: Deferred Interrupt Processing, Daemon Task Architecture & Callback Multiplexing.
* **Core Concepts Mastered**:
  1. **Deferred Interrupt Processing**: ISR-safe APIs (e.g., `xTimerStopFromISR`) act merely as "couriers". They do not manipulate timer lists directly. Instead, they quickly drop command packets into the Timer Command Queue, deferring the heavy lifting to the Daemon Task to prevent race conditions and ensure microsecond-level ISR exit.
  2. **The Mandatory "Priority Audit"**: `portYIELD_FROM_ISR()` is a critical hardware trigger. If an ISR wakes a higher-priority task (flagging `pdTRUE`), this macro forces an immediate PendSV context switch right before the ISR exits, guaranteeing strict real-time preemption.
  3. **The Daemon Task (Timer Service)**: The ultimate "System Manager" with absolute authority over timer linked lists. It wakes up under two conditions: (a) instantly when a command enters the queue (to schedule the timer), and (b) when the SysTick matches the expiry time (to execute the callback).
  4. **Callback Multiplexing (RAM/Flash Optimization)**: Mastered the industrial practice of sharing a single Callback Function across multiple timers (One-shot and Auto-reload). By extracting the unique Timer ID via `pvTimerGetTimerID()`, we achieve dynamic execution paths while saving valuable Flash memory space.
* **Tomorrow's Plan**: Descend into the physical foundation of RTOS: Memory Management. Deconstruct `Heap_1` through `Heap_5` to uncover exactly where and how TCBs, Queues, and Task Stacks allocate physical RAM.

### 2026-04-03
* **Learning Content**: Task Notification Physics & RTOS Causality.
* **Core Concepts Mastered**:
  1. **TCB-Based IPC**: Task Notifications use the 32-bit `ulNotifiedValue` inside the TCB. No external objects = 45% faster and zero RAM overhead.
  2. **Unidirectional Causality**: `Give/Take` are software APIs for Task sync. Only Tasks "sleep" (Blocked); Interrupts are hardware-driven and never block or wait for software signals.
  3. **Atomic Counter**: `Give` increments the TCB counter; `Take` is a "conditional sleep" that yields the CPU only if the counter is 0.
  4. **The Swiss Army Knife**: `xTaskNotify` uses `eAction` to transform the 32-bit variable into an Event Group, a Mailbox, or a Counting Semaphore.
* **Tomorrow's Plan**: Master the receiving end with `xTaskNotifyWait` to achieve precise bit extraction and "State-Bit" management.

### 2026-04-02
* **Learning Content**: The Grand Unification of FreeRTOS IPC, Mutex Ownership, and Recursive Mutex Physics.
* **Core Concepts Mastered**:
  1. **The IPC Grand Unification (Queue as the Universal Engine)**: Discovered that Semaphores and Mutexes are physically just Queues under the hood. The API `xSemaphoreGive` is a macro that calls `xQueueGenericSend` with a payload pointer of `NULL` and an item size of `0`. This zero-copy execution achieves extreme speed while reusing the queue's blocking/unblocking mechanisms.
  2. **The "Ownership" Contract (Mutex vs. Binary Semaphore)**: Differentiated the physical structure of a Mutex. Unlike a Binary Semaphore, a Mutex utilizes a C `union` within the `QueueDefinition` to store the `xMutexHolder` (a pointer to the `pxCurrentTCB` of the task that took it). This strict binding enables Priority Inheritance but relies on the programmer's discipline (No `configASSERT` in release builds = unauthorized unlocking is mechanically possible but architecturally fatal).
  3. **Recursive Mutex Physics (The "Russian Doll" Lock)**: Mastered the mechanics of `xSemaphoreTakeRecursive`. It utilizes the `uxRecursiveCallCount` variable. It bypasses the blocking state if the requesting task's TCB matches the `xMutexHolder`. Formulated the iron rule: The number of `GiveRecursive` calls must strictly match the `TakeRecursive` calls for the `CallCount` to reach `0` and physically release the lock.
  4. **The Deadlock Core & Priority Inversion**: Identified the "Hold and Wait" condition as the hidden killer in Deadlocks. Analyzed the Priority Inversion waveform, distinguishing it from a true Deadlock, and understanding how a medium-priority task can hijack the CPU while a high-priority task is blocked by a low-priority lock holder.
* **Tomorrow's Plan**: Finalize the deep dive into Priority Inheritance implementation details and unlock the ultimate lightweight communication weapon: **Task Notifications**. Compare its speed and RAM footprint against the legacy Queue-based IPC.

### 2026-04-01
* **Learning Content**: FreeRTOS Binary Semaphore Physics, API Asymmetry, and the "Vanishing Error" Mystery (`portMAX_DELAY`).
* **Core Concepts Mastered**:
  1. **The "Empty Box" Initial State**: Verified that `xSemaphoreCreateBinary()` creates a semaphore with an initial value of 0 (empty). This is the fundamental physical difference from Mutexes (initial 1), making binary semaphores the perfect tool for "waiting for a signal" rather than "protecting a resource."
  2. **The Asymmetry of Give vs. Take**: 
     - *The Producer (Give)*: Hardcoded as a "Non-blocking" action (`xTicksToWait = 0`). In event-driven systems, producers like ISRs cannot wait; if the buffer is full, the event is simply dropped (resulting in `Give ERR`).
     - *The Consumer (Take)*: Designed for "Patient Waiting." By providing `xTicksToWait`, the task can gracefully transition from the Ready List to the Blocked List, avoiding CPU wastage.
  3. **The Physics of Blocking**: Analyzed how `portMAX_DELAY` triggers a "Physical Disappearance" of the task. The kernel removes the TCB from the Ready List and hangs it on the Semaphore's waiting list, ensuring zero CPU overhead until a `Give` occurs.
  4. **The Mystery of the "Missing Take Error"**: Solved the forensic puzzle of why `Take ERR` never appeared in the logs. Understood that `portMAX_DELAY` creates an infinite block—the code never reaches the `else` (Error) branch because the task is physically suspended until it succeeds.
* **Tomorrow's Plan**: Shift the focus to the "Ownership" model. Unlock the mystery of the Mutex and how its "Priority Inheritance" magic prevents the fatal "Priority Inversion" deadlock.

### 2026-03-31
* **Learning Content**: FreeRTOS IPC (Inter-Process Communication) Arsenal, ISR (Interrupt Service Routine) Hardware Privileges, and Queue Memory Optimization (Pass-by-Reference).
* **Core Concepts Mastered**:
  1. **The IPC Arsenal**: 
     - *Queue/Semaphore/Mutex*: Share the same underlying `QueueDefinition` structure. Mastered the critical distinction: Mutexes possess "Ownership" (strictly unlocked by the locker), while Semaphores are blind counters.
     - *Event Groups*: The only mechanism capable of "AOE Broadcasting" to wake multiple tasks simultaneously.
     - *Task Notifications*: The ultimate RAM-saving weapon that bypasses queue creation entirely by writing directly to the target's TCB.
  2. **ISR Physics & The 3 Iron Rules**: Defined ISR as a hardware-triggered preemptive strike that completely freezes the OS scheduler. Enforced the absolute survival rules: 1. Lightning fast execution; 2. Absolute ban on blocking APIs (`vTaskDelay`); 3. Mandatory use of `FromISR` API variants.
  3. **Deferred Context Switching**: Demystified `xQueueSendFromISR`. Understood how the `pxHigherPriorityTaskWoken` flag safely records if a high-priority task was awakened, deferring the actual CPU context switch (`portYIELD_FROM_ISR`) until the very end of the interrupt to prevent stack corruption.
  4. **Queue Pointer Passing & `sprintf`**: Decoded `sprintf` as a tool for writing formatted strings directly into physical RAM. Optimized Queue RAM by sending a 4-byte pointer instead of copying entire 30-byte string buffers (Pass-by-Reference). Identified and sealed the fatal trap of passing local stack variable pointers by enforcing `static` or global buffers.
* **Tomorrow's Plan**: Trace the pointer out of the queue on the consumer side, or officially unlock the FreeRTOS Memory Management final boss: The `Heap_4` block coalescing algorithm.

### 2026-03-30
* **Learning Content**: FreeRTOS Preemptive vs. Cooperative Scheduling, Priority 0 Cohabitation, and Logic Analyzer Waveform Forensics (`configIDLE_SHOULD_YIELD`).
* **Core Concepts Mastered**:
  1. **Boot Selection vs. Runtime Eviction**: Corrected the preemption misconception. Discovered that turning off preemption (`configUSE_PREEMPTION = 0`) does *not* change the initial task execution order; `pxCurrentTCB` always guarantees the highest priority task boots first. Preemption only dictates whether a running task can be forcibly evicted later.
  2. **The Priority 0 Illusion**: Shattered the myth that Priority 0 is exclusively reserved for the OS Idle Task. User tasks can legally share this priority, triggering Round-Robin time-slicing alongside the system's garbage collector.
  3. **The Yielding Physics (`configIDLE_SHOULD_YIELD`)**: Analyzed logic analyzer waveforms to map software macros to physical electrical signals:
     - *YIELD = 1 (Needle Waveforms)*: The Idle Task acts as a "submissive gentleman," instantly aborting its CPU time slice if other Priority 0 tasks are ready. This risks Idle Task starvation, leading to fatal memory leaks (unfreed zombie tasks).
     - *YIELD = 0 (Square Waveforms)*: The Idle Task acts as an equal citizen, refusing to yield until the hardware SysTick forces a context switch, ensuring perfect time-slicing.
  4. **Absolute Preemption Dominance**: Visually verified that regardless of Priority 0 yielding mechanics, a high-priority task (e.g., Task 3) acts as an absolute guillotine, instantly hijacking the CPU the microsecond it unblocks.
* **Tomorrow's Plan**: Step into the ultimate FreeRTOS Memory Management battlefield: Uncover the "Memory Coalescing" magic of `Heap_4` and how it physically stitches fragmented RAM blocks back together.

### 2026-03-29
* **Learning Content**: FreeRTOS Scheduler Mechanics, RAM/Flash Memory Physical Layout, TCB Anatomy, and Absolute Time Management.
* **Core Concepts Mastered**:
  1. **SysTick & Time Slicing**: Shattered the illusion of infinite loops. Understood that `configUSE_TIME_SLICING = 1` empowers the hardware SysTick interrupt to forcefully preempt tasks of equal priority (Round-Robin), slicing CPU time mechanically.
  2. **Physical Memory Map (`.data` vs `.bss`)**: Demystified the `static` keyword and memory initialization. Flash holds immutable `.text` and initializer backups. The `startup.s` assembly physically copies `.data` to RAM and ruthlessly zeros out `.bss` (which physically houses `heap_1`'s massive `ucHeap` array).
  3. **TCB Anatomy & Pointer Physics**: 
     - *Inline Data vs Pointers*: Discovered that TCBs embed critical data (state list items, task names, priorities) directly as inline structures to prevent fragmentation and CPU cache misses, reserving pointers exclusively for the dynamically sized Stacks.
     - *Stack Overflow Math*: Mastered the downward-growing ARM stack forensic formula: `pxTopOfStack <= pxStack` (comparing absolute physical addresses, completely stripping away the `*` dereference trap).
     - *The OS HR Analogy*: Solidified that Tasks are blind "employees"; the TCB is the kernel's strictly classified "HR file" used exclusively by the scheduler to manipulate task states.
  4. **Scheduler Boot & Ready List FIFO**: Traced the elusive `pxCurrentTCB` pointer during task creation. Uncovered that higher-priority tasks instantly hijack this pointer, while equal-priority tasks queue strictly FIFO (First-In, First-Out) in the Ready List via `vListInsertEnd`.
  5. **Time Drift Defense (`vTaskDelayUntil`)**: Contrasted `vTaskDelay` (Relative time, highly vulnerable to execution-time drift) with `xTaskDelayUntil` (Absolute time). Mastered the physical mechanism of anchoring cycle times (`&Pre`) to guarantee strict periodicity for critical industrial applications like Motor PID control and audio sampling.
* **Tomorrow's Plan**: Step into the ultimate FreeRTOS Memory Management battlefield: Uncover how `Heap_4` actively merges adjacent free memory blocks to solve the fragmentation chaos left behind by `Heap_2`.

### 2026-03-28
* **Learning Content**: FreeRTOS Memory Management Architecture, `heap_1` Physical Implementation, and MISRA C Safety Philosophy.
* **Core Concepts Mastered**:
  1. **The Static Array Illusion**: Shattered the illusion of dynamic allocation. Discovered that `heap_1` avoids true dynamic heap mechanisms by reserving a massive, static global `uint8_t` array (`ucHeap`) in the `.bss` segment at compile time.
  2. **Deterministic O(1) Allocation**: Mastered the "Sausage Slicer" algorithm. `pvPortMalloc` simply increments a single pointer (`pxNextFreeByte`) upwards through the array to carve out chunks for TCBs and Stacks. This guarantees an absolute, deterministic execution time crucial for real-time responsiveness.
  3. **The "No-Free" Survival Law**: Decoded the deliberate omission of `vPortFree()`. Understood that in life-critical systems (aerospace, automotive ABS, medical devices), memory fragmentation and unpredictable allocation times are strictly banned (e.g., by MISRA C standards). `heap_1` provides absolute physical safety, zero fragmentation, and zero memory leaks for "create-once-and-run-forever" static architectures.
* **Tomorrow's Plan**: Trace the chaotic evolution of memory management. Explore `heap_2` to witness the catastrophic fragmentation caused by introducing a basic `free()` function, OR skip directly to the industrial gold standard, `heap_4`, to master the ultimate "memory block coalescing" algorithm.

### 2026-03-27
* **Learning Content**: FreeRTOS Queue Memory Architecture, Ring Buffer Pointer Mechanics, and the IPC Union Secret.
* **Core Concepts Mastered**:
  1. **The Physical Queue Illusion**: Shattered the "magic pipe" illusion of IPC. A FreeRTOS Queue is fundamentally just a contiguous RAM array (Ring Buffer) bound by absolute physical floor (`pcHead`) and ceiling (`pcTail`) pointers.
  2. **The Asymmetric Pointer Dance**: Mastered the counter-intuitive but mathematically elegant pointer mechanics:
     - *Write Pointer (`pcWriteTo`)*: "Write, then increment." Always points to the next available empty slot.
     - *Read Pointer (`pcReadFrom`)*: "Increment, then read." Always points to the *last consumed* (abandoned) slot. 
     - *Architectural Brilliance*: This built-in offset allows the kernel to instantly determine an "Empty Queue" state simply by checking if `pcWriteTo == pcReadFrom`, bypassing complex arithmetic.
  3. **The Ultimate IPC Union**: Uncovered Richard Barry's architectural masterpiece within `QueueDefinition`. Discovered that Semaphores and Mutexes are physically identical to Queues at the core level. By using a C `union`, they recycle the exact same Task blocking/unblocking logic and data structures, simply ignoring the data buffer allocation.
* **Tomorrow's Plan**: Trace the physical execution path of a "Queue Full" scenario (investigating how a Task gets knocked out and thrown into the `xTasksWaitingToSend` list) OR transition to the ultimate FreeRTOS endgame: Memory Management (Heap_1 through Heap_5).

### 2026-03-26
* **Learning Content**: FreeRTOS Run Time Statistics (CPU Load Calculation) and High-Resolution Hardware Timer Physics.
* **Core Concepts Mastered**:
  1. **The OS Tick Blind Spot**: Shattered the illusion that the standard OS Tick (e.g., 1ms SysTick) can accurately measure CPU usage. Short-lived tasks that execute and yield entirely within a single Tick period become invisible "ghosts," leading to grossly inaccurate load calculations (0% recorded runtime).
  2. **The High-Resolution Hardware Stopwatch**: Discovered the strict architectural prerequisite for enabling Run Time Stats (`vTaskGetRunTimeStats`). The OS mandates an independent, high-frequency hardware timer (ideally 10x-100x faster than the SysTick) to act as an unblinking physical stopwatch.
  3. **Context Switch Accounting**: Mapped the exact physical moments of timekeeping. At every Context Switch boundary, the kernel records the precise timestamp when a task enters and exits the CPU state ($T_{out} - T_{in}$). This delta is instantly accumulated into the Task Control Block's (TCB) `ulRunTimeCounter`, allowing for sub-millisecond precision accounting.
* **Tomorrow's Plan**: Finalize the macro configurations for Run Time Stats (`portCONFIGURE_TIMER_FOR_RUN_TIME_STATS`) OR dive directly into the ultimate FreeRTOS battlefield: Memory Management (Analyzing the physical evolution from Heap_1 to Heap_5 and how Heap_4 prevents fragmentation).

### 2026-03-25
* **Learning Content**: FreeRTOS Concurrency Defense, Cortex-M Interrupt Priority Architecture, `configASSERT` Forensics, and Stack Overflow Detection (Watermarking).
* **Core Concepts Mastered**:
  1. **Concurrency Defense Tiers**: Clarified the physical difference between locking the scheduler (`vTaskSuspendAll` for Task vs. Task) and masking hardware interrupts (`taskENTER_CRITICAL` for Task vs. ISR). 
  2. **The Hardware Priority Red Line**: Decoded the Cortex-M priority system (lower number = higher priority). Identified `configMAX_SYSCALL_INTERRUPT_PRIORITY` (e.g., 191) as the OS boundary. Priorities 0-190 are unrestricted "god-tier" hardware interrupts (OS APIs strictly forbidden), while the OS kernel itself sits at the lowest priority (255) to guarantee real-time hardware responsiveness.
  3. **ISR Context Snapshots**: Understood why `taskENTER_CRITICAL_FROM_ISR` cannot simply hardcode the BASEPRI register to 0 upon exit. It must save and restore the exact hardware state (`uxSavedInterruptStatus`) to preserve Cortex-M interrupt nesting layers.
  4. **The OS Self-Destruct Sequence**: Demystified `configASSERT`. It is not an error handler, but a "crime-scene freezing" mechanism (`while(1)`) often enhanced with `__FILE__` and `__LINE__` for pinpoint debugging. It is physically compiled out in Release builds to save ROM and CPU cycles.
  5. **Stack Overflow Forensics (Method 2)**: Mastered the `0xA5` "Poison Pill / Watermark" technique. The OS paints the entire stack with `0xA5` upon creation and inspects the physical floor for "footprints". Decoded Richard Barry's cross-platform macro magic: `pucStackByte -= portSTACK_GROWTH`, which elegantly translates to `+= 1` (moving upwards from the floor) on ARM architectures where the stack grows downwards (`portSTACK_GROWTH = -1`).
* **Tomorrow's Plan**: Step into the ultimate FreeRTOS battlefield: Memory Management (Heap_1 through Heap_5). Analyze how the OS allocates TCBs and Queues, fights memory fragmentation, and keeps the system stable, OR write practical code to query and print a Task's High Water Mark (`uxTaskGetStackHighWaterMark`).

### 2026-03-24
* **Learning Content**: Hardware ISR Physics, Daemon Task Starvation, and FreeRTOS `FromISR` API Kernel Anatomy.
* **Core Concepts Mastered**:
  1. **The Physical Truth of ISRs**: Demystified the hardware timer. It is merely a silicon countdown mechanism that sends an IRQ electrical signal. Timers do not "run" code; rather, the IRQ forces the CPU to jump to a fixed memory address (`SysTick_Handler`). 
  2. **Daemon Task Starvation & The Polling Trap**: Conducted a hardcore code review on a flawed implementation. Proved that a high-priority `while(1)` loop lacking a blocking API (like `vTaskDelay`) will monopolize CPU time slices, completely starving the internal Timer Service Task and silently killing all software timers.
  3. **The `FromISR` API Architecture (3 Physical Rules)**: Analyzed kernel source code (`xQueueGenericSendFromISR`) to validate OS rules in hardware interrupt contexts:
     - *Zero Blocking*: Complete removal of the `xTicksToWait` parameter. ISRs must execute deterministically and cannot wait.
     - *Nested Interrupt Protection*: Utilizing `uxSavedInterruptStatus` to carefully preserve and restore hardware priority states without blinding the entire system.
     - *Deferred Context Switch*: Using the `pxHigherPriorityTaskWoken` pointer and manually calling `portYIELD_FROM_ISR()` to safely defer the preemptive context switch until the very end of the ISR routine, preventing stack corruption.
  4. **The OS Priority Boundary**: Decoded `portASSERT_IF_INTERRUPT_PRIORITY_INVALID()`. Understood that hardware interrupts configured with a priority higher than `configMAX_SYSCALL_INTERRUPT_PRIORITY` are outside the RTOS's jurisdiction and are strictly forbidden from calling any FreeRTOS APIs.
* **Tomorrow's Plan**: Shift focus from execution flow to memory physics. Explore FreeRTOS Memory Management (Heap_1 to Heap_5) to understand how the OS statically or dynamically allocates TCBs and Queues without fragmenting the microcontroller's limited RAM.

### 2026-03-23
* **Learning Content**: FreeRTOS Software Timers Internals (Daemon Task, Command Queue, and Callback Physics).
* **Core Concepts Mastered**:
  1. **The ISR vs. Task Architecture**: Shattered the illusion that software timers run inside the hardware Tick ISR. To prevent catastrophic system halts, FreeRTOS defers timer callback execution to a dedicated, high-privilege system thread called the "Timer Service Task" (or Daemon Task).
  2. **The "No-Blocking" Ironclad Rule**: Since ALL software timer callbacks execute within the context of the *single* shared Daemon Task, calling ANY blocking API (e.g., `vTaskDelay`, `xSemaphoreTake(..., portMAX_DELAY)`) inside a callback is a fatal architectural sin. It will instantly paralyze the entire OS software timer subsystem.
  3. **The Timer Command Queue Illusion**: Discovered that APIs like `xTimerStart()` and `xTimerStop()` do not directly manipulate timer hardware or lists. They are simply IPC wrappers that send instruction messages via a hidden `Timer Command Queue` to the Daemon Task. This perfectly explains why `xTimerStart` requires a `xTicksToWait` parameter (to handle scenarios where the command queue is full).
  4. **Daemon Task Priority Starvation**: Identified a critical system failure mode. If `configTIMER_TASK_PRIORITY` is set lower than heavy CPU-bound user tasks, the Daemon Task will never get CPU time to read the command queue. Commands will pile up, and timers will fail to trigger, destroying the system's real-time timeline.
* **Tomorrow's Plan**: Shift to practical application. Write a clean, non-blocking timer callback implementation to toggle LEDs/states, and inspect `FreeRTOSConfig.h` to physically tune the Daemon Task's priority and Command Queue length.

### 2026-03-21
* **Learning Content**: FreeRTOS Preemptive Scheduling, Hardware-Level Critical Sections, and IPC Consumption Physics (Mailbox vs. Task Notifications).
* **Core Concepts Mastered**:
  1. **Preemptive Scheduling (The Absolute Rule of Preemption)**: Eliminated the misconception of cooperative scheduling. Confirmed that APIs like `xTaskNotifyGive` instantly trigger a context switch (via PendSV interrupt) *before returning*. If the awakened task has a higher priority, the current task is immediately preempted, regardless of its execution state.
  2. **Critical Sections (Hardware-Level Time Stop)**: Analyzed `taskENTER_CRITICAL()` and its physical mechanism of masking hardware interrupts (e.g., via the `BASEPRI` register). Understood its necessity for preventing "Data Tearing" in multi-byte operations, while strictly acknowledging its danger: calling blocking APIs inside this section will cause catastrophic system failure.
  3. **IPC Consumption Physics (Mailbox vs. Notify Overwrite)**: Decoded the difference between non-destructive and destructive reads. A standard Mailbox (`xQueuePeek`) leaves data intact for multiple reads. Conversely, Task Notification with `eSetValueWithOverwrite` is a destructive read ("burn after reading"); calling `xTaskNotifyWait` fetches the value and physically clears the notification state, allowing only a single read.
* **Tomorrow's Plan**: Solidify this week's hardcore theory by writing a practical code snippet using Task Notifications (`eSetValueWithOverwrite`) to replace a legacy bare-metal polling loop, or lightly explore FreeRTOS Software Timers to see how the OS handles time-delayed events without blocking tasks.

### 2026-03-20
* **Learning Content**: Decoded C++ Object Model Internals (vptr, vtable, Polymorphism) and Mastered FreeRTOS Ultimate IPC Mechanisms (Barrier Synchronization & Task Notifications).
* **Core Concepts Mastered**:
  1. **The "Polymorphism Assassination" (Virtual Destructors)**: Exposed the lethal memory leak trap in C++. Understood that without a `virtual` destructor, deleting a derived object via a base pointer triggers Static Binding, silently abandoning the derived class's memory. 
  2. **C++ `vtable` & `vptr` (The 8-Byte Tax)**: Smashed the illusion of object methods. Functions live in the `.text` segment. Adding `virtual` forces the compiler to inject a hidden 8-byte `vptr` into the object, pointing to a class-level `vtable` (function pointer array) to achieve Dynamic Binding.
  3. **Overload vs. Override (Compile-time vs. Runtime)**: 
     - **Overload**: A compiler illusion. Uses Name Mangling (e.g., `_Z5Printi`) to create completely independent functions.
     - **Override**: True polymorphism. Replaces the base class's function pointer inside the `vtable` with the derived class's function pointer.
  4. **Event Group Sync (The RTOS Barrier)**: Analyzed `xEventGroupSync`. Mastered the atomic operation of "Setting my flag & Waiting for others." Proved that it instantly yields the CPU (entering Blocked state) with zero-polling overhead until the entire system reaches the synchronization barrier.
  5. **Task Notifications (The Ultimate IPC Weapon)**: Discovered the absolute performance ceiling of FreeRTOS. By writing directly to the `ulNotifiedValue` and `ucNotifyState` inside the target's Task Control Block (TCB), it eliminates all intermediate IPC objects (queues/semaphores). Yields a 45% speed boost with zero extra RAM, acting as the ultimate chameleon (Simulating Semaphores, Event Groups, and Mailboxes).
* **Tomorrow's Plan**: Shift from architectural theory to hardcore coding. Implement a bare-metal to RTOS refactoring using Task Notifications to replace legacy polling loops, proving the 45% performance gain on actual silicon.

### 2026-03-19
* **Learning Content**: Advanced FreeRTOS Synchronization Mechanisms & Code Review (Recursive Mutexes, Event Groups, and RTOS Anti-patterns).
* **Core Concepts Mastered**:
  1. **The Polling Anti-Pattern & Mutex Ownership**: Conducted a hardcore code review on a toxic `while(1)` spinlock. Reaffirmed the physical laws of Mutexes: **Never `Give` a lock you didn't `Take`**. Mastered the principle of replacing zero-timeout polling with OS-level blocking (`portMAX_DELAY`) to prevent CPU starvation.
  2. **Recursive Mutex (The Self-Deadlock Antidote)**: Decoded the internal "nesting depth counter" of `xSemaphoreTakeRecursive`. Understood its architectural necessity: allowing a task to take the same lock multiple times without permanently locking itself out (crucial for protecting nested library calls in large-scale industrial code).
  3. **Event Group (The OS-Level AND/OR Gate)**: Smashed a classic FreeRTOS trap question. Analyzed the bitmask logic of `xEventGroupWaitBits`. Verified that the RTOS natively supports "Wait for ANY bit" (OR logic) and "Wait for ALL bits" (AND logic) via the `xWaitForAllBits` parameter, but fundamentally cannot natively process "Wait for N out of M" (complex threshold logic).
* **Tomorrow's Plan**: Transition from theory and code review to actual system design. Build a multi-sensor data fusion architecture using a combination of Queues (for data) and Event Groups (for synchronization triggers).

### 2026-03-18
* **Learning Content**: Systematically decoded FreeRTOS IPC mechanisms (Queue Sets, Semaphores, Mutexes) and dived into bottom-level C++11 move semantics and CPU-level Atomic operations.
* **Core Concepts Mastered**:
  1. **Queue Set Multiplexing**: Understood `Queue Set` as the RTOS version of `epoll`. Mastered the strict "one ticket, one fetch" physical rule and the necessity of exact memory sizing to prevent event loss.
  2. **Semaphore vs. Mutex Philosophy**: Completely separated the concepts. Semaphores are for "Event Synchronization" (Wait/Trigger, no ownership, interrupt-safe), while Mutexes are for "Resource Protection" (Strict ownership, prevents Priority Inversion via Priority Inheritance, absolutely NO interrupts).
  3. **IPC Performance Trade-offs**: Weighed Message Queues (safe FIFO, block/wake OS integration, but has `memcpy` overhead) against Shared Memory + Mutex (Absolute zero-copy, but vulnerable to state overwrite and deadlocks).
  4. **Software to Silicon (std::move & Atomics)**: Demystified `std::move` as a pure compiler cast to `T&&` (Rvalue) to legally hijack memory. Pushed down to the silicon level to understand how `std::atomic` uses hardware bus-locks for 1-cycle thread safety, crushing OS-level Mutexes for simple variables.
* **Tomorrow's Plan**: Implement the ultimate "Zero-Copy + OS Notification" cheat code: using a FreeRTOS Queue to pass physical pointers of a massive shared memory block, fusing the high performance of Shared Memory with the elegant blocking mechanisms of Queues.

### 2026-03-17
* **Learning Content**: Dived deep into FreeRTOS internal memory mechanics (Ring Buffers & Unions) and transitioned to modern C++ memory management (Lvalue vs. Rvalue references).
* **Core Concepts Mastered**:
  1. **RTOS Memory Squeezing**: Decoded the `union` in FreeRTOS `QueueDefinition`. Understood that Semaphores and Mutexes are essentially "data-less queues" reusing the same memory footprint to save SRAM.
  2. **Ring Buffer Physics**: Debunked the "data shifting" illusion in Queues. Mastered the "pointer chasing" mechanism where `Read` and `Write` pointers progress from low to high addresses without moving physical data (unrelated to CPU Endianness).
  3. **Scheduling Philosophy**: Differentiated task yielding strategies: `vTaskDelay` (forces Blocked state) vs. `taskYIELD()` (PendSV-triggered cooperative sharing, returns to Ready state) vs. Priority Preemption (event-driven).
  4. **C++ Zero-Copy & Move Semantics**: Demystified references. Proved `T&` (Lvalue) is a safe, auto-dereferenced `const` pointer for zero-copy sharing, while `T&&` (Rvalue) enables "Move Semantics" to safely hijack memory from dying temporary objects.
* **Tomorrow's Plan**: Apply the zero-copy pointer philosophy to FreeRTOS Queues to efficiently pass a large `struct` without triggering catastrophic `memcpy` overhead, and explore ISR-to-Task (Interrupt Service Routine) communication.

### 2026-03-16
* **Learning Content**: Transitioned from RTOS concurrency control and IPC mechanisms to C++ Object-Oriented memory management and compiler-level name resolution rules.
* **Core Concepts Mastered**:
  1. **RTOS Concurrency & IPC**: Differentiated Mutexes (featuring Priority Inheritance to prevent Priority Inversion) from Spinlocks (fatal on single-core, mitigated via CPU Critical Sections). Evaluated IPC efficiency: Shared Memory (Zero-copy but requires polling) vs. Message Queues (Event-driven blocking) vs. Task Notifications (Highest performance).
  2. **C++ Encapsulation & Memory**: Explored `friend` functions for controlled encapsulation bypass (e.g., operator overloading). Mastered the necessity of custom Deep Copy Constructors to prevent "Double Free" segmentation faults caused by default shallow copies.
  3. **C++ Name Resolution & Macros**: Mapped the rigid boundaries between Overloading (same scope, different signatures), Overriding (vtable, `override` keyword, covariant returns), and Name Hiding (cross-scope shadowing, bypassable via `using`). Dissected `#define` macro mechanics including token pasting (`##`) and stringification (`#`).
  4. **FreeRTOS OOP Paradigm**: Decoded `xTaskCreate`'s `void *pvParameters`. Leveraged dynamic pointer casting to achieve code reuse, instantiating multiple unique task behaviors from a single generic task function.
* **Tomorrow's Plan**: Implement a generic task function passing a complex `struct` via `pvParameters`, and observe Mutex priority inheritance or Message Queue blocking states using the Keil Logic Analyzer.

### 2026-03-15
* **Learning Content**: Dived into FreeRTOS kernel configuration, exploring the Idle Task lifecycle, C-language linkage rules for debugging, and the core macros governing preemptive and time-sliced scheduling.
* **Core Concepts Mastered**:
  1. **Heap Exhaustion & `pdPASS`**: Understood that `xTaskCreate` is fundamentally a dynamic RAM allocation operation. Mastered the necessity of verifying the `pdPASS` return value to prevent silent system crashes caused by heap depletion.
  2. **Internal Linkage vs. Logic Analyzer**: Solved the `Undefined Identifier` error by recognizing that the `static` keyword applies "internal linkage," hiding symbols from external debuggers. Enforced global `volatile` variables to ensure memory visibility on the hardware bus.
  3. **The Hollywood Principle & Idle Hook**: Dissected `vApplicationIdleHook`. Realized the Idle Task automatically handles system-level garbage collection (freeing deleted TCBs/Stacks). Learned to inject user-level background tasks (e.g., watchdog feeding, low-power modes) while strictly avoiding blocking APIs (`printf`, `vTaskDelay`) to prevent kernel paralysis.
  4. **Scheduler Engine Configuration**: Decoded the ultimate RTOS macros in `FreeRTOSConfig.h`:
     * `configUSE_PREEMPTION = 1`: Enables strict, priority-based CPU hijacking (jungle rules).
     * `configUSE_TIME_SLICING = 1`: Enables SysTick-driven Round-Robin scheduling for tasks of *equal* priority, preventing CPU starvation.
* **Tomorrow's Plan**: Finalize the Logic Analyzer setup to capture the exact waveforms of `taskflagrun` variables, visually proving the Round-Robin time-slicing and Preemption mechanics in real-time. Then, advance to Inter-Process Communication (IPC).

### 2026-03-14
* **Learning Content**: Modernized the development workflow using VS Code and dissected the physical execution of FreeRTOS task creation and hardware atomicity.
* **Core Concepts Mastered**:
  1. **Cross-IDE Workflow**: Integrated VS Code with Keil Assistant. Resolved the "Silent Crash" XML error by enforcing strict ASCII-only directory paths, eliminating non-standard character parsing failures in the build toolchain.
  2. **Task Creation Physics**: Decoded `xTaskCreate` parameters. Verified that task functions decay into Flash addresses and understood the "Handle vs NULL" trade-off for task lifecycle control (TCB pointer management).
  3. **Debugging Visibility**: Identified the scope limitations of `static` and `stack-based` local variables in debugging. Mastered `volatile` global declarations to ensure memory visibility for the Keil Logic Analyzer.
  4. **RTOS Boot Hooks**: Analyzed the STM32 startup file (`.s`) to understand how FreeRTOS hijacks `SysTick` and `PendSV` interrupts to take control of the Program Counter (PC) for context switching.
* **Tomorrow's Plan**: Run the Logic Analyzer to visually verify the RTOS scheduler's time-slicing behavior and begin exploring Inter-Process Communication (IPC) using Queues.

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
