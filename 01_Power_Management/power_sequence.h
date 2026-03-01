/**
 * @file    power_sequence.h
 * @brief   Header file for the ACPI Power Sequence State Machine
 * @note    This is a simplified simulation for EC firmware learning.
 */

#ifndef POWER_SEQUENCE_H
#define POWER_SEQUENCE_H

#include <stdint.h>
#include <stdbool.h>

// Define ACPI System Power States
typedef enum {
    SYS_STATE_G3 = 0,   // Mechanical Off (No AC, No Battery)
    SYS_STATE_S5,       // Soft Off (Standby power available)
    SYS_STATE_S4,       // Suspend to Disk (Hibernate)
    SYS_STATE_S3,       // Suspend to RAM (Sleep)
    SYS_STATE_S0,       // Full On (Working)
    SYS_STATE_TRANSITION // Intermediate state during power up/down
} SystemState_t;

// Define Hardware Signals (Simulated GPIOs)
typedef struct {
    bool pwr_btn_pressed;   // Power Button Input
    bool ac_present;        // AC Adapter plugged in
    bool slp_s3_n;          // PCH SLP_S3# signal (Active Low)
    bool slp_s4_n;          // PCH SLP_S4# signal (Active Low)
    bool slp_s5_n;          // PCH SLP_S5# signal (Active Low)
    
    // Power Good (PG) signals from Voltage Regulators
    bool pg_3v3_alw;
    bool pg_1v2_ram;
    bool pg_vcore;
} HardwareSignals_t;

// Function Prototypes
void PowerSeq_Init(void);
void PowerSeq_Task(void);
SystemState_t Get_Current_System_State(void);

#endif // POWER_SEQUENCE_H