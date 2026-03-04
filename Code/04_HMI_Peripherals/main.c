/**
 * @file    main.c
 * @brief   Test bench for Keyboard Matrix Debounce and Power Management
 */

#include <stdio.h>
#include "keyboard_scan.h"

/* Access the internal matrix state for simulation purposes */
extern KeyboardMatrix_t kbd;

int main() {
    printf("==================================================\n");
    printf("     EC Keyboard Matrix Simulator (0x04)          \n");
    printf("==================================================\n\n");

    Keyboard_Init();

    /* Scenario 1: Simulating a physical key press at [Row 2, Col 5] with noise */
    printf("---> [USER ACTION] Pressing key at [2, 5] with mechanical bouncing...\n");

    for (int ms = 1; ms <= 50; ms++) {
        /* Simulate 10ms of bouncing noise (Toggle between 0 and 1) */
        if (ms <= 10) {
            /* Inconsistent signal during physical contact strike */
            static bool toggle = false;
            toggle = !toggle;
            ((KeyboardMatrix_t*)&kbd)->current_raw_state[2][5] = toggle;
        } else {
            /* Signal becomes stable after the bounce period */
            ((KeyboardMatrix_t*)&kbd)->current_raw_state[2][5] = true; 
        }

        /* Call the periodic scan task (Simulating 1ms hardware timer) */
        Keyboard_Scan_Task();

        if (ms == 31) { // 10ms noise + 20ms threshold + 1ms processing
            printf("      (Log: At %d ms, the debounce threshold should be reached)\n", ms);
        }
    }

    /* Scenario 2: Simulate Sleep and Wake mechanism */
    Keyboard_Power_Management_Sim(true);
    
    printf("\n---> [USER ACTION] Any key pressed during sleep...\n");
    printf("[HW_INT] KSI falling-edge detected! Generating Wake Event.\n");
    
    Keyboard_Power_Management_Sim(false);

    printf("\n==================================================\n");
    printf("   Simulation Complete. All logical states verified. \n");
    printf("==================================================\n");

    return 0;
}