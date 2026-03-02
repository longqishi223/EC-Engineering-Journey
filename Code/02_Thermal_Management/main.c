/**
 * @file    main.c
 * @brief   Test bench for EC Thermal Management
 */

#include <stdio.h>
#include <unistd.h>
#include "thermal_control.h"

int main() {
    printf("=========================================\n");
    printf("     EC Thermal Simulator Started        \n");
    printf("=========================================\n\n");

    Thermal_Init();

    // Array of simulated CPU temperatures over time
    // Scenario: Idle -> Light load -> Gaming -> Cooling issue -> Catastrophic failure
    uint8_t simulated_temps[] = {35, 42, 58, 65, 72, 86, 92, 98, 110, 80};
    int num_samples = sizeof(simulated_temps) / sizeof(simulated_temps[0]);

    for (int i = 0; i < num_samples; i++) {
        uint8_t temp = simulated_temps[i];
        
        printf("\n---> [SENSOR] CPU PECI reading: %d C\n", temp);
        
        // Feed the temperature into the EC Thermal Task
        Thermal_Task(temp);
        
        // Stop the simulation if the EC triggered a hardware shutdown
        if (Is_System_Shutdown_Triggered()) {
            printf("\n*** SIMULATION HALTED: Motherboard power was cut by EC to save the CPU! ***\n");
            break;
        }
        
        // Sleep for a short time to make the console output readable
        sleep(1); 
    }

    printf("\n=========================================\n");
    printf("     Simulation Complete. Exiting...     \n");
    printf("=========================================\n");

    return 0;
}