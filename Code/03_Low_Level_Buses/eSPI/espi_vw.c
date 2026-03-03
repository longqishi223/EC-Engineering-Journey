/**
 * @file    espi_vw.c
 * @brief   Implementation of eSPI Virtual Wire communication
 */

#include "espi_vw.h"
#include <stdio.h>

// Simulated hardware registers for the eSPI Controller inside the EC
static uint8_t ESPI_VW_INDEX_REG = 0x00;
static uint8_t ESPI_VW_DATA_REG  = 0x00;
static uint8_t ESPI_VW_CTRL_REG  = 0x00;

#define ESPI_CTRL_SEND_TRIGGER   0x01 // Bit to tell hardware to send the packet

void eSPI_Init(void) {
    printf("[EC_eSPI] Initializing eSPI Controller (1.8V, 66MHz)...\n");
    // In reality, this involves negotiating channel readiness with the PCH
    printf("[EC_eSPI] Virtual Wire Channel: Ready.\n");
}

/**
 * @brief Simulates sending a Virtual Wire packet to the PCH
 * @param index The Virtual Wire group index (e.g., 0x40 for System Events)
 * @param bit_mask The specific virtual wire to modify (e.g., SCI#)
 * @param assert_low True to assert (pull low), False to de-assert (pull high)
 */
bool eSPI_VW_SetWireState(uint8_t index, uint8_t bit_mask, bool assert_low) {
    
    // 1. Read the current state of this virtual wire group (simulated)
    // Normally, the EC maintains a shadow copy of the VW states.
    uint8_t current_data = ESPI_VW_DATA_REG; 
    
    // 2. Modify the specific bit. 
    // Note: These signals are active-low (suffixed with _N), so 'asserting' means clearing the bit to 0.
    if (assert_low) {
        current_data &= ~bit_mask; // Clear bit to 0 (Assert)
    } else {
        current_data |= bit_mask;  // Set bit to 1 (De-assert)
    }

    // 3. Write to the hardware transmission registers
    ESPI_VW_INDEX_REG = index;
    ESPI_VW_DATA_REG  = current_data;
    
    printf("\n[eSPI BUS] Constructing Virtual Wire Packet -> Index: 0x%02X, Data: 0x%02X\n", 
           ESPI_VW_INDEX_REG, ESPI_VW_DATA_REG);

    // 4. Trigger the hardware to send the packet across the physical eSPI bus
    ESPI_VW_CTRL_REG |= ESPI_CTRL_SEND_TRIGGER;
    printf("[eSPI BUS] Transmitting packet over eSPI IO lines...\n");

    // Clear trigger bit (simulating hardware auto-clear)
    ESPI_VW_CTRL_REG &= ~ESPI_CTRL_SEND_TRIGGER;

    return true;
}

/**
 * @brief High-level application function to trigger an OS ACPI event via SCI
 */
void EC_Trigger_ACPI_Event(void) {
    printf("[EC_APP] Urgency: Triggering ACPI SCI# to wake/notify OS driver.\n");
    
    // Assert SCI# (Virtual pull down to 0V)
    eSPI_VW_SetWireState(VW_INDEX_SYSTEM_EVENT, VW_BIT_SCI_N, true);
    
    // De-assert SCI# (Virtual release to 1.8V) - It's usually a pulse!
    printf("[EC_APP] Releasing SCI# after pulse.\n");
    eSPI_VW_SetWireState(VW_INDEX_SYSTEM_EVENT, VW_BIT_SCI_N, false);
}