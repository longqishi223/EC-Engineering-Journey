/**
 * @file    thermal_control.h
 * @brief   Header file for EC Thermal Management and Fan Speed Control (FSC)
 */

#ifndef THERMAL_CONTROL_H
#define THERMAL_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// --- Critical Thermal Thresholds (in Celsius) ---
#define PROCHOT_TEMP_C      90  // CPU throttles itself at this temp
#define THERM_TRIP_TEMP_C   105 // Critical hardware shutdown temp

// --- Fan Control Structure ---
typedef struct {
    uint8_t temp_threshold; // Temperature in Celsius
    uint8_t pwm_duty_cycle; // Fan speed percentage (0-100%)
} ThermalTableStep_t;

// --- Function Prototypes ---
void Thermal_Init(void);
void Thermal_Task(uint8_t current_cpu_temp);
uint8_t Get_Current_Fan_PWM(void);
bool Is_Prochot_Asserted(void);
bool Is_System_Shutdown_Triggered(void);

#endif // THERMAL_CONTROL_H