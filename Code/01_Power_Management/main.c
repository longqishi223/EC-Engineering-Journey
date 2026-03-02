/**
 * @file    main.c
 * @brief   Test bench for the EC Power Sequence State Machine
 */

#include <stdio.h>
#include "power_sequence.h"

// Declare the external variable so we can simulate hardware signal changes in main()
extern HardwareSignals_t hw_signals; 

int main() {
    printf("=========================================\n");
    printf("   EC Power Sequence Simulator Started   \n");
    printf("=========================================\n\n");

    // 1. Initialize the EC state machine
    PowerSeq_Init();

    // 2. Simulate plugging in the AC Adapter (From G3 Mechanical Off -> S5 Soft Off)
    printf("\n---> [USER ACTION] Plugging in AC Adapter...\n");
    hw_signals.ac_present = true;
    PowerSeq_Task(); // Run the state machine once
    
    // 3. Simulate pressing the power button to boot (S5 -> TRANSITION state)
    printf("\n---> [USER ACTION] Pressing Power Button...\n");
    hw_signals.pwr_btn_pressed = true;
    PowerSeq_Task(); // Run state machine; EC detects the button press and wakes the PCH
    
    // Release the power button (mechanical buttons are not held down indefinitely)
    hw_signals.pwr_btn_pressed = false; 

    // 4. Continue running the state machine to complete the power-up sequence (TRANSITION -> S0)
    printf("\n---> [SYSTEM] Continuing power sequence...\n");
    PowerSeq_Task(); // PCH asserts SLP signals, EC turns on various power rails
    
    // Check if successfully entered S0 (Working State)
    if (Get_Current_System_State() == SYS_STATE_S0) {
        printf("\n*** SUCCESS: System is fully ON and running Windows/Linux (S0 State)! ***\n");
    }

    // ---------------- ADVANCED TESTING BELOW ---------------- //

    // 5. Simulate OS commanding a Sleep (S0 -> S3)
    printf("\n---> [USER ACTION] OS commands Sleep (S3)...\n");
    hw_signals.slp_s3_n = false; // Simulate PCH dropping the SLP_S3# signal
    PowerSeq_Task(); // EC detects the signal drop, cuts CPU power but maintains RAM power

    if (Get_Current_System_State() == SYS_STATE_S3) {
        printf("\n*** SUCCESS: System is sleeping (S3). RAM is still powered. Zzz... ***\n");
    }

    // 6. Simulate waking up from sleep (S3 -> TRANSITION -> S0)
    printf("\n---> [USER ACTION] Pressing Power Button to Wake...\n");
    hw_signals.pwr_btn_pressed = true;
    PowerSeq_Task(); // EC triggers the wake event
    
    hw_signals.pwr_btn_pressed = false; // Release the button
    PowerSeq_Task(); // Complete the wake-up transition

    if (Get_Current_System_State() == SYS_STATE_S0) {
        printf("\n*** SUCCESS: System woke up and is back to S0! ***\n");
    }

    printf("\n=========================================\n");
    printf("   Simulation Complete. Exiting...       \n");
    printf("=========================================\n");

    return 0;
}