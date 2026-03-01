/**
 * @file    power_sequence.c
 * @brief   Implementation of the ACPI Power Sequence State Machine
 * @note    Handles transitions between S5, S3, and S0 based on hardware signals.
 */

#include "power_sequence.h"
#include <stdio.h> // For debug printing in simulation

// Global Variables
static SystemState_t current_state = SYS_STATE_G3;
HardwareSignals_t hw_signals;

/**
 * @brief Initializes the power sequence module
 */
void PowerSeq_Init(void) {
    current_state = SYS_STATE_G3;
    
    // Initialize all simulated hardware signals to default (OFF) states
    hw_signals.pwr_btn_pressed = false;         // Power Button Input
    hw_signals.ac_present = false;              // AC Adapter plugged in
    hw_signals.slp_s3_n = false;
    hw_signals.slp_s4_n = false;
    hw_signals.slp_s5_n = false;
    hw_signals.pg_3v3_alw = false;              // Power Good (PG) signals from Voltage Regulators
    hw_signals.pg_1v2_ram = false;              // Power Good (PG) signals from Voltage Regulators
    hw_signals.pg_vcore = false;                // Power Good (PG) signals from Voltage Regulators
    
    printf("[EC_PWR] Power Sequence Module Initialized.\n");
}

/**
 * @brief Main state machine task. Should be called periodically in the EC main loop.
 */
void PowerSeq_Task(void) {
    switch (current_state) {
        
        case SYS_STATE_G3:
            // Waiting for AC or Battery to be plugged in
            if (hw_signals.ac_present) {
                printf("[EC_PWR] AC attached. Turning on Always-On (ALW) power...\n");
                // In real EC: Assert EN_3V3_ALW GPIO here
                
                // Simulate waiting for Power Good
                hw_signals.pg_3v3_alw = true; 
                
                if (hw_signals.pg_3v3_alw) {
                    current_state = SYS_STATE_S5;
                    printf("[EC_PWR] Transitioned to S5 (Soft Off).\n");
                }
            }
            break;

        case SYS_STATE_S5:
            // System is off, waiting for Power Button press
            if (hw_signals.pwr_btn_pressed) {
                printf("[EC_PWR] Power Button Pressed in S5! Initiating power-up...\n");
                
                // In real EC: Assert RSMRST# (Resume Reset) to PCH here
                // Simulate PCH waking up and asserting SLP signals
                hw_signals.slp_s5_n = true;
                hw_signals.slp_s4_n = true;
                hw_signals.slp_s3_n = true;
                
                current_state = SYS_STATE_TRANSITION;
            }
            break;

        case SYS_STATE_TRANSITION:
            // Checking PCH SLP signals to determine target state
            if (hw_signals.slp_s3_n && hw_signals.slp_s4_n && hw_signals.slp_s5_n) {
                printf("[EC_PWR] PCH requests S0. Turning on RAM and CPU power...\n");
                
                // Simulate turning on RAM and getting PG
                hw_signals.pg_1v2_ram = true;
                
                // Simulate turning on CPU Vcore and getting PG
                hw_signals.pg_vcore = true;
                
                if (hw_signals.pg_1v2_ram && hw_signals.pg_vcore) {
                    current_state = SYS_STATE_S0;
                    printf("[EC_PWR] All power rails stable. Transitioned to S0 (Working).\n");
                } else {
                    printf("[EC_PWR] FATAL ERROR: Missing Power Good signal! System halt.\n");
                    // In real EC: Emergency shutdown sequence here to prevent damage
                }
            }
            break;

        case SYS_STATE_S0:
            // System is fully running. Monitor for shut down events (e.g., Thermal Trip or OS shutdown)
            if (!hw_signals.slp_s3_n) {
                printf("[EC_PWR] PCH dropped SLP_S3#. Entering S3 (Sleep)...\n");
                // In real EC: Turn off CPU Vcore, keep RAM power on
                hw_signals.pg_vcore = false;
                current_state = SYS_STATE_S3;
            }
            break;

        case SYS_STATE_S3:
            // Monitor for wake events (Power button, Keyboard, etc.)
            if (hw_signals.pwr_btn_pressed) {
                 printf("[EC_PWR] Wake event in S3! Transitioning back to S0...\n");
                 // Simulate PCH asserting SLP_S3# again
                 hw_signals.slp_s3_n = true;
                 current_state = SYS_STATE_TRANSITION;
            }
            break;

        default:
            printf("[EC_PWR] Unknown State! Forcing S5.\n");
            current_state = SYS_STATE_S5;
            break;
    }
}

SystemState_t Get_Current_System_State(void) {
    return current_state;
}
