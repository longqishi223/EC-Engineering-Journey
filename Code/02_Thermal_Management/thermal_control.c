/**
 * @file    thermal_control.c
 * @brief   Implementation of EC Thermal Management
 */

#include "thermal_control.h"
#include <stdio.h>

// --- Global State Variables ---
static uint8_t current_pwm = 0;
static bool prochot_asserted = false;
static bool shutdown_triggered = false;

// --- EC Step-based Thermal Table ---
// In a real laptop, this table is usually tuned by acoustic engineers 
// to balance cooling performance and fan noise.
static const ThermalTableStep_t fan_table[] = {
    {0,   0},   // Below 40C: Fan is OFF (Passive cooling)
    {40,  30},  // 40C+ : 30% PWM (Low speed, quiet)
    {55,  50},  // 55C+ : 50% PWM (Medium speed)
    {70,  75},  // 70C+ : 75% PWM (High speed, audible)
    {85,  100}  // 85C+ : 100% PWM (Max cooling, loud)
};
#define TABLE_SIZE (sizeof(fan_table) / sizeof(fan_table[0]))

void Thermal_Init(void) {
    current_pwm = 0;
    prochot_asserted = false;
    shutdown_triggered = false;
    printf("[EC_THERM] Thermal Management Initialized. Fan is OFF.\n");
}

void Thermal_Task(uint8_t current_cpu_temp) {
    if (shutdown_triggered) {
        return; // System is dead, ignore further thermal tasks
    }

    // ==========================================
    // 1. Hardware-Level Safety Protections
    // ==========================================
    
    // Check for catastrophic overheating (THERMTRIP#)
    if (current_cpu_temp >= THERM_TRIP_TEMP_C) {
        printf("[EC_THERM] [!!! FATAL !!!] Temp %dC exceeds THERMTRIP (%dC)!\n", current_cpu_temp, THERM_TRIP_TEMP_C);
        printf("[EC_THERM] Asserting hardware shutdown (S5/G3) to prevent silicon melting!\n");
        shutdown_triggered = true;
        // In real EC: Pull SYS_SHDN# or similar GPIO low immediately
        return; 
    }

    // Check for heavy load throttling (PROCHOT#)
    if (current_cpu_temp >= PROCHOT_TEMP_C) {
        if (!prochot_asserted) {
            printf("[EC_THERM] [WARNING] Temp %dC exceeds PROCHOT (%dC). Asserting PROCHOT# to throttle CPU.\n", current_cpu_temp, PROCHOT_TEMP_C);
            prochot_asserted = true;
        }
    } else {
        if (prochot_asserted) {
            printf("[EC_THERM] [INFO] Temp %dC recovered below PROCHOT. De-asserting PROCHOT#.\n", current_cpu_temp);
            prochot_asserted = false;
        }
    }

    // ==========================================
    // 2. Fan Speed Control (FSC) Logic
    // ==========================================
    uint8_t target_pwm = 0;
    
    // Iterate through the thermal table to find the appropriate PWM step
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (current_cpu_temp >= fan_table[i].temp_threshold) {
            target_pwm = fan_table[i].pwm_duty_cycle;
        }
    }

    // Apply the new PWM only if it has changed
    if (target_pwm != current_pwm) {
        printf("[EC_THERM] Fan PWM updated: %3d%% -> %3d%% (Current Temp: %dC)\n", current_pwm, target_pwm, current_cpu_temp);
        current_pwm = target_pwm;
        // In real EC: Write 'target_pwm' to the Fan Controller IC register here
    }
}

uint8_t Get_Current_Fan_PWM(void) { return current_pwm; }
bool Is_Prochot_Asserted(void) { return prochot_asserted; }
bool Is_System_Shutdown_Triggered(void) { return shutdown_triggered; }