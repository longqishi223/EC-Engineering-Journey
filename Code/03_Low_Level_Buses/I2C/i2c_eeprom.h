/**
 * @file    i2c_eeprom.h
 * @brief   Header file for I2C EEPROM (e.g., AT24C02) driver
 */

#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

#include <stdint.h>
#include <stdbool.h>

// Standard 7-bit I2C address for an EEPROM (typically 0x50 to 0x57 depending on hardware pins)
#define EEPROM_SLAVE_ADDR  0x50 

// Function Prototypes
void I2C_Init(void);
bool I2C_EEPROM_WriteByte(uint8_t mem_addr, uint8_t data);
uint8_t I2C_EEPROM_ReadByte(uint8_t mem_addr);

#endif // I2C_EEPROM_H