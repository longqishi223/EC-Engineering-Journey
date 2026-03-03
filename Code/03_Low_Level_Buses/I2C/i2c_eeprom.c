/**
 * @file    i2c_eeprom.c
 * @brief   Implementation of I2C read/write sequences for an EEPROM
 */

#include "i2c_eeprom.h"
#include <stdio.h>

// Simulate the internal memory array of a 256-byte EEPROM (like AT24C02)
static uint8_t simulated_eeprom_memory[256];

/**
 * @brief Initializes the I2C controller and formats the simulated EEPROM
 */
void I2C_Init(void) {
    printf("[EC_I2C] Initializing I2C Controller...\n");
    // In real hardware, an erased EEPROM is filled with 0xFF
    for (int i = 0; i < 256; i++) {
        simulated_eeprom_memory[i] = 0xFF;
    }
}

/**
 * @brief Writes a single byte to a specific address in the EEPROM
 * @param mem_addr The internal address of the EEPROM to write to
 * @param data The byte to write
 */
bool I2C_EEPROM_WriteByte(uint8_t mem_addr, uint8_t data) {
    // 1. Shift slave address and append WRITE bit (0)
    uint8_t slave_write_addr = (EEPROM_SLAVE_ADDR << 1) | 0x00;

    // Simulate the physical I2C waveform being driven on the bus
    printf("\n[I2C BUS]  [START] -> [0x%02X(W)]-[ACK] -> [MemAddr:0x%02X]-[ACK] -> [Data:0x%02X]-[ACK] -> [STOP]\n", 
           slave_write_addr, mem_addr, data);

    /* * In a real EC, you would configure hardware registers here.
     * I2C_DATA_REG = mem_addr; I2C_CTRL = SEND_BYTE; wait_for_ack();
     * I2C_DATA_REG = data;     I2C_CTRL = SEND_BYTE_STOP; wait_for_ack();
     */

    // Write the data into our simulated memory
    simulated_eeprom_memory[mem_addr] = data;
    
    // EEPROMs require a 5ms internal write cycle time. 
    // The EC must not talk to it during this time.
    printf("[EC_I2C] Delaying 5ms for EEPROM internal write cycle...\n");

    return true;
}

/**
 * @brief Reads a single byte from a specific address in the EEPROM
 * @note  This uses the "Dummy Write" / Repeated Start sequence!
 * @param mem_addr The internal address of the EEPROM to read from
 */
uint8_t I2C_EEPROM_ReadByte(uint8_t mem_addr) {
    uint8_t slave_write_addr = (EEPROM_SLAVE_ADDR << 1) | 0x00; // Write bit is 0
    uint8_t slave_read_addr  = (EEPROM_SLAVE_ADDR << 1) | 0x01; // Read bit is 1

    // Simulate the complex "Random Read" I2C waveform
    printf("\n[I2C BUS]  [START] -> [0x%02X(W)]-[ACK] -> [MemAddr:0x%02X]-[ACK] ... (Dummy Write completed)\n", 
           slave_write_addr, mem_addr);
           
    printf("[I2C BUS]  ... [REPEATED START] -> [0x%02X(R)]-[ACK] -> [Data Read]-[NACK] -> [STOP]\n", 
           slave_read_addr);

    /* * In real EC:
     * 1. Send Write Addr + Mem Addr (Dummy Write to set internal pointer)
     * 2. Issue RESTART condition
     * 3. Send Read Addr
     * 4. Read data buffer, send NACK to terminate, generate STOP.
     */

    return simulated_eeprom_memory[mem_addr];
}