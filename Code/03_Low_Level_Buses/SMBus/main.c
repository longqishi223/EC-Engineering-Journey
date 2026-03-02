/**
 * @file    main.c
 * @brief   Test bench for EC SMBus Smart Battery polling
 */

#include <stdio.h>
#include "smbus_battery.h"

int main() {
    printf("=========================================\n");
    printf("   EC Smart Battery Simulator Started    \n");
    printf("=========================================\n\n");

    // 1. Initialize the battery module
    Battery_Init();

    // 2. EC polls the battery data (usually done every 1 second via a timer interrupt)
    printf("\n---> [SYSTEM] Triggering 1-second timer to read battery data...\n");
    Battery_Poll_Data();

    // 3. Retrieve and print the parsed data to the OS
    SmartBatteryInfo_t current_bat = Get_Battery_Info();

    printf("\n=========================================\n");
    printf("   Battery Status Report to OS (ACPI)    \n");
    printf("=========================================\n");
    printf("   Voltage   : %d mV (%.2f V)\n", current_bat.voltage_mV, current_bat.voltage_mV / 1000.0f);
    printf("   Current   : %d mA\n", current_bat.current_mA);
    
    // Check discharging/charging state
    if (current_bat.current_mA < 0) {
        printf("   Status    : Discharging\n");
    } else if (current_bat.current_mA > 0) {
        printf("   Status    : Charging\n");
    } else {
        printf("   Status    : Idle (Fully Charged or AC only)\n");
    }

    printf("   Capacity  : %d %%\n", current_bat.rsoc_percent);
    printf("   Temp      : %.1f °C\n", current_bat.temperature_C);
    printf("=========================================\n\n");

    return 0;
}