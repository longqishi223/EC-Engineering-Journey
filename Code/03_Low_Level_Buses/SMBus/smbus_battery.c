/**
 * @file    smbus_battery.c
 * @brief   Implementation of SMBus data parsing for Smart Battery
 */

#include "smbus_battery.h"
#include <stdio.h>

// Global struct to hold the latest battery data
static SmartBatteryInfo_t bat_info;

/**
 * @brief  MOCK FUNCTION: Simulates reading a 16-bit word from the SMBus.
 * @note   In a real EC, this function configures the hardware I2C/SMBus controller,
 * sends the START condition, Address, Command, reads data, and checks the PEC byte.
 */
static uint16_t SMBus_Mock_ReadWord(uint8_t slave_addr, uint8_t command) {
    uint8_t lsb = 0;
    uint8_t msb = 0;

    // Simulate different battery responses based on the SBS command
    switch (command) {
        case SBS_CMD_VOLTAGE:
            // Simulate 11400 mV (11.4V) -> 0x2C88
            lsb = 0x88; msb = 0x2C; 
            break;
        case SBS_CMD_CURRENT:
            // Simulate discharging at -1500 mA -> 0xFA24 (Two's complement)
            lsb = 0x24; msb = 0xFA;
            break;
        case SBS_CMD_RSOC:
            // Simulate 85% charge -> 0x0055
            lsb = 0x55; msb = 0x00;
            break;
        case SBS_CMD_TEMP:
            // Simulate 30.0 Celsius = 303.15 Kelvin = 3031.5 (0.1K units) -> ~3032 -> 0x0BD8
            lsb = 0xD8; msb = 0x0B;
            break;
        default:
            return 0xFFFF; // Error/Unknown command
    }

    // Combine LSB and MSB. SMBus sends LSB first (Little-Endian).
    // This bitwise shift is a classic embedded C operation!
    return (uint16_t)((msb << 8) | lsb);
}

void Battery_Init(void) {
    printf("[EC_SMBUS] Initializing Smart Battery interface...\n");
    bat_info.voltage_mV = 0;
    bat_info.current_mA = 0;
    bat_info.rsoc_percent = 0;
    bat_info.temperature_C = 0.0f;
    bat_info.is_charging = false;
}

/**
 * @brief Periodically polls the battery via SMBus and updates the struct.
 */
void Battery_Poll_Data(void) {
    uint16_t raw_data;

    // 1. Read Voltage
    bat_info.voltage_mV = SMBus_Mock_ReadWord(BATTERY_SMBUS_ADDR, SBS_CMD_VOLTAGE);

    // 2. Read Current (Cast to int16_t because it can be negative)
    bat_info.current_mA = (int16_t)SMBus_Mock_ReadWord(BATTERY_SMBUS_ADDR, SBS_CMD_CURRENT);
    bat_info.is_charging = (bat_info.current_mA > 0);

    // 3. Read RSOC (Relative State of Charge)
    raw_data = SMBus_Mock_ReadWord(BATTERY_SMBUS_ADDR, SBS_CMD_RSOC);
    bat_info.rsoc_percent = (uint8_t)(raw_data & 0x00FF); // Only need the lower byte

    // 4. Read Temperature and convert from 0.1K to Celsius
    // Formula: Celsius = (Kelvin / 10) - 273.15
    raw_data = SMBus_Mock_ReadWord(BATTERY_SMBUS_ADDR, SBS_CMD_TEMP);
    bat_info.temperature_C = ((float)raw_data / 10.0f) - 273.15f;

    printf("[EC_SMBUS] Battery Polling Complete.\n");
}

SmartBatteryInfo_t Get_Battery_Info(void) {
    return bat_info;
}