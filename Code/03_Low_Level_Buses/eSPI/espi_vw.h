/**
 * @file    espi_vw.h
 * @brief   Header file for eSPI Virtual Wire (VW) Channel simulation
 */

#ifndef ESPI_VW_H
#define ESPI_VW_H

#include <stdint.h>
#include <stdbool.h>

// --- eSPI Virtual Wire Index Definitions (Standardized by Intel) ---
// Index 40h contains System Management Interrupts
#define VW_INDEX_SYSTEM_EVENT  0x40 

// Bit definitions within Index 40h
#define VW_BIT_SMI_N           (1 << 0) // System Management Interrupt
#define VW_BIT_SCI_N           (1 << 1) // System Control Interrupt (ACPI)
#define VW_BIT_RCIN_N          (1 << 2) // Reset CPU

// --- Function Prototypes ---
void eSPI_Init(void);
bool eSPI_VW_SetWireState(uint8_t index, uint8_t bit_mask, bool assert_low);
void EC_Trigger_ACPI_Event(void);

#endif // ESPI_VW_H