/**
 * @file    smbus_battery.h
 * @brief   Header file for SMBus Smart Battery System (SBS) communication
 */

#ifndef SMBUS_BATTERY_H
#define SMBUS_BATTERY_H

#include <stdint.h>
#include <stdbool.h>

// --- Standard SBS (Smart Battery System) Command Codes ---
#define SBS_CMD_TEMP            0x08  // Temperature (Units: 0.1 Kelvin)
#define SBS_CMD_VOLTAGE         0x09  // Voltage (Units: mV)
#define SBS_CMD_CURRENT         0x0A  // Current (Units: mA, negative means discharging)
#define SBS_CMD_RSOC            0x0D  // Relative State of Charge (Units: %)
#define SBS_CMD_DESIGN_CAPACITY 0x18  // Design Capacity (Units: mAh)

// SMBus Slave Address for Smart Battery (Standard is 0x16, 8-bit write address)
#define BATTERY_SMBUS_ADDR      0x16  

// --- Battery Information Structure ---
typedef struct {
    uint16_t voltage_mV;
    int16_t  current_mA;       // int16_t because current can be negative!
    uint8_t  rsoc_percent;     // 0-100%
    float    temperature_C;    // Converted to Celsius for the OS
    bool     is_charging;
} SmartBatteryInfo_t;

// --- Function Prototypes ---
void Battery_Init(void);
void Battery_Poll_Data(void);
SmartBatteryInfo_t Get_Battery_Info(void);

#endif // SMBUS_BATTERY_H