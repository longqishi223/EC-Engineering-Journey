#ifndef __HID_MOUSE_H
#define __HID_MOUSE_H

#include <stdint.h>

/* * 💡 Architect-Level Warning:
 * By default, C compilers (MDK/GCC) pad structures to 4-byte or 8-byte boundaries to optimize CPU access speed.
 * However, in the HID protocol, even a single byte of unexpected padding will cause Windows to reject the report and throw an error!
 * Therefore, we MUST use #pragma pack(1) to force the compiler: NO PADDING ALLOWED. Every byte must be strictly contiguous!
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t buttons;  // Byte 0: Button status (Bit 0: Left, Bit 1: Right, Bit 2: Middle, remaining 5 bits padded with 0)
    int8_t  x;        // Byte 1: X-axis relative movement (-127 to +127)
    int8_t  y;        // Byte 2: Y-axis relative movement (-127 to +127)
} HID_MouseReport_TypeDef;
#pragma pack(pop)

// Expose the report descriptor to the outside (usually sent to the host during USB enumeration or I2C-HID initialization)
extern const uint8_t HID_Mouse_ReportDescriptor[];
extern const uint32_t HID_Mouse_ReportDescSize;

// API to simulate sending mouse movements and clicks
void HID_Mouse_SendEvent(int8_t delta_x, int8_t delta_y, uint8_t left_click);

#endif