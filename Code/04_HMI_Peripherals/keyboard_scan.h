/**
 * @file    keyboard_scan.h
 * @brief   Keyboard Matrix Scanning and Debouncing logic definitions
 */

#ifndef KEYBOARD_SCAN_H
#define KEYBOARD_SCAN_H

#include <stdint.h>
#include <stdbool.h>

/* --- Matrix Configuration --- */
/* Laptop keyboard matrix usually consists of 8 rows (KSI) and 16 columns (KSO) */
#define KSI_COUNT    8
#define KSO_COUNT    16

/* Debounce threshold: Signal must be stable for 20ms to be validated */
#define DEBOUNCE_THRESHOLD    20 

typedef struct {
    uint8_t  debounce_counters[KSI_COUNT][KSO_COUNT]; // Independent counter for each key
    bool     current_raw_state[KSI_COUNT][KSO_COUNT]; // Real-time physical state (with noise)
    bool     debounced_state[KSI_COUNT][KSO_COUNT];   // Final validated state for OS
} KeyboardMatrix_t;

/* --- Function Prototypes --- */
void Keyboard_Init(void);
void Keyboard_Scan_Task(void);
bool Is_Key_Pressed(uint8_t row, uint8_t col);
void Keyboard_Power_Management_Sim(bool sleep_mode);

#endif // KEYBOARD_SCAN_H