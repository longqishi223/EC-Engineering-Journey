/**
 * @file    keyboard_scan.c
 * @brief   Implementation of Matrix Scanning and Per-key Debouncing
 */

#include "keyboard_scan.h"
#include <stdio.h>

static KeyboardMatrix_t kbd;

/**
 * @brief Initializes the keyboard matrix and clear all states
 */
void Keyboard_Init(void) {
    for (int r = 0; r < KSI_COUNT; r++) {
        for (int c = 0; c < KSO_COUNT; c++) {
            kbd.debounce_counters[r][c] = 0;
            kbd.current_raw_state[r][c] = false;
            kbd.debounced_state[r][c] = false;
        }
    }
    printf("[EC_KBD] Matrix Scanner Initialized (8x16 mode).\n");
}

/**
 * @brief Periodic Keyboard Scanning Task (Simulating 1ms timer interrupt)
 * The EC drives KSO lines LOW one by one and reads the state of KSI lines.
 */
void Keyboard_Scan_Task(void) {
    for (uint8_t c = 0; c < KSO_COUNT; c++) {
        /* * Logic: In real hardware, we drive KSO[c] LOW and check all KSI rows.
         * If a switch is closed, the LOW signal is passed to the KSI line.
         */
        for (uint8_t r = 0; r < KSI_COUNT; r++) {
            bool physical_input = kbd.current_raw_state[r][c];

            /* --- Debounce State Machine --- */
            /* Check if the current physical state differs from our confirmed state */
            if (physical_input != kbd.debounced_state[r][c]) {
                kbd.debounce_counters[r][c]++;
                
                /* If signal is stable for enough time, update the debounced state */
                if (kbd.debounce_counters[r][c] >= DEBOUNCE_THRESHOLD) {
                    kbd.debounced_state[r][c] = physical_input;
                    kbd.debounce_counters[r][c] = 0; // Reset counter
                    
                    if (physical_input) {
                        printf("[EC_KBD] STABLE EVENT: Key [%d, %d] PRESSED\n", r, c);
                    } else {
                        printf("[EC_KBD] STABLE EVENT: Key [%d, %d] RELEASED\n", r, c);
                    }
                }
            } else {
                /* Signal matches confirmed state or is just a noise spike; reset counter */
                kbd.debounce_counters[r][c] = 0;
            }
        }
    }
}

/**
 * @brief Check if a specific key is in a stable 'pressed' state
 */
bool Is_Key_Pressed(uint8_t row, uint8_t col) {
    return kbd.debounced_state[row][col];
}

/**
 * @brief Simulate the power management transitions described in README
 */
void Keyboard_Power_Management_Sim(bool sleep_mode) {
    if (sleep_mode) {
        printf("\n[EC_PWR] System entering Sleep (S3/S0ix). Halting active scanning loop.\n");
        printf("[EC_PWR] Driving ALL KSOs LOW to enable asynchronous wake interrupts.\n");
    } else {
        printf("[EC_PWR] System Waking Up! Resuming high-speed matrix scanning task.\n");
    }
}