# 📅 My EC Learning and Development Log

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
