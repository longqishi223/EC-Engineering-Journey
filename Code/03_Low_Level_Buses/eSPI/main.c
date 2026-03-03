/**
 * @file    main.c
 * @brief   Test bench for eSPI Virtual Wire Channel
 */

#include <stdio.h>
#include <unistd.h>
#include "espi_vw.h"

int main() {
    printf("==================================================\n");
    printf("        EC eSPI Bus Simulator Started             \n");
    printf("==================================================\n\n");

    eSPI_Init();

    // Simulate system running normally
    printf("\n---> [SYSTEM] OS is running normally...\n");
    sleep(1);

    // Simulate a hardware sensor event detected by EC (e.g., Battery drops below 10%)
    printf("---> [SYSTEM] SENSOR ALERT: Battery capacity dropped to 9%%!\n");
    printf("---> [SYSTEM] EC must notify the OS immediately.\n");
    sleep(1);

    // EC triggers the ACPI event via eSPI Virtual Wire
    EC_Trigger_ACPI_Event();

    printf("\n---> [SYSTEM] Host PCH received eSPI Virtual SCI# pulse.\n");
    printf("---> [SYSTEM] OS ACPI driver wakes up, queries EC via eSPI Peripheral Channel, and shows 'Low Battery' warning to user.\n");

    printf("\n==================================================\n");
    printf("   Simulation Complete. Exiting...                \n");
    printf("==================================================\n");

    return 0;
}