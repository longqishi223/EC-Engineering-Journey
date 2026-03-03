/**
 * @file    main.c
 * @brief   Test bench for I2C EEPROM Read/Write operations
 */

#include <stdio.h>
#include "i2c_eeprom.h"

int main() {
    printf("==================================================\n");
    printf("      EC I2C EEPROM Simulator Started             \n");
    printf("==================================================\n\n");

    I2C_Init();

    // The data we want to write (e.g., a motherboard hardware revision ID)
    uint8_t target_memory_address = 0x4A;
    uint8_t motherboard_rev_id    = 0x88;

    printf("\n---> [SYSTEM] Attempting to write Motherboard Rev ID (0x%02X) to EEPROM Address 0x%02X...\n", 
           motherboard_rev_id, target_memory_address);
           
    // Perform the I2C Write
    I2C_EEPROM_WriteByte(target_memory_address, motherboard_rev_id);

    printf("\n---> [SYSTEM] Write complete. Now attempting to read back from Address 0x%02X for verification...\n", 
           target_memory_address);

    // Perform the I2C Read (Dummy Write + Repeated Start)
    uint8_t read_back_data = I2C_EEPROM_ReadByte(target_memory_address);

    printf("\n==================================================\n");
    printf("   Verification Results: \n");
    printf("   Expected Data : 0x%02X\n", motherboard_rev_id);
    printf("   Read Data     : 0x%02X\n", read_back_data);
    
    if (read_back_data == motherboard_rev_id) {
        printf("   STATUS        : [PASS] I2C Read/Write Successful!\n");
    } else {
        printf("   STATUS        : [FAIL] Data Mismatch!\n");
    }
    printf("==================================================\n\n");

    return 0;
}