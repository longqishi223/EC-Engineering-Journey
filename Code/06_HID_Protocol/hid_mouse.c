#include "hid_mouse.h"

/* * 📜 Report Descriptor
 * This is the "Instruction Manual" the MCU sends to Windows to introduce itself.
 * It perfectly and strictly describes the meaning of every single bit in the HID_MouseReport_TypeDef struct above!
 */
const uint8_t HID_Mouse_ReportDescriptor[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)   -> Tell OS: I am a generic desktop device
    0x09, 0x02,        // USAGE (Mouse)                  -> Tell OS: My specific identity is a Mouse
    0xA1, 0x01,        // COLLECTION (Application)       -> ==== Start of Application Collection ====
    0x09, 0x01,        //   USAGE (Pointer)              -> My function is to control the cursor/pointer
    0xA1, 0x00,        //   COLLECTION (Physical)        -> ---- Start of Physical Data Collection ----
    
    // ---------------- Step 1: Define 3 Buttons (Consumes 3 Bits) ----------------
    0x05, 0x09,        //     USAGE_PAGE (Button)        -> I am about to send button data
    0x19, 0x01,        //     USAGE_MINIMUM (Button 1)   -> Minimum is Button 1 (Left Click)
    0x29, 0x03,        //     USAGE_MAXIMUM (Button 3)   -> Maximum is Button 3 (Middle Click)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)        -> Physical min value is 0 (Unpressed)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)        -> Physical max value is 1 (Pressed)
    0x95, 0x03,        //     REPORT_COUNT (3)           -> There are 3 data fields like this
    0x75, 0x01,        //     REPORT_SIZE (1)            -> Each data field occupies 1 Bit
    0x81, 0x02,        //     INPUT (Data,Var,Abs)       -> Generate Input Report (These 3 bits are packed)
    
    // ---------------- Step 2: Bit Padding (Consumes 5 Bits) ----------------
    // The minimum memory unit is a Byte (8 bits). We used 3 bits, so 5 bits must be padded with 0 to complete 1 Byte.
    0x95, 0x01,        //     REPORT_COUNT (1)           -> 1 data field
    0x75, 0x05,        //     REPORT_SIZE (5)            -> Occupies 5 Bits
    0x81, 0x03,        //     INPUT (Cnst,Var,Abs)       -> This is a constant (Cnst), OS will ignore it
    // [At this point, the 1st byte (buttons) in the struct is fully described!]

    // ---------------- Step 3: Define X and Y axes (Consumes 2 Bytes) ----------------
    0x05, 0x01,        //     USAGE_PAGE (Generic Desktop)-> Switch back to Generic Desktop page
    0x09, 0x30,        //     USAGE (X)                  -> This is the X axis
    0x09, 0x31,        //     USAGE (Y)                  -> This is the Y axis
    0x15, 0x81,        //     LOGICAL_MINIMUM (-127)     -> Minimum value is -127 (Max left/up)
    0x25, 0x7F,        //     LOGICAL_MAXIMUM (127)      -> Maximum value is +127 (Max right/down)
    0x75, 0x08,        //     REPORT_SIZE (8)            -> Each data field occupies 8 Bits (1 Byte)
    0x95, 0x02,        //     REPORT_COUNT (2)           -> There are 2 data fields like this (X and Y)
    0x81, 0x06,        //     INPUT (Data,Var,Rel)       -> Generate Input Report (Rel = Relative movement)
    // [At this point, the 2nd and 3rd bytes (x, y) in the struct are fully described!]

    0xC0,              //   END_COLLECTION               -> ---- End of Physical Collection ----
    0xC0               // END_COLLECTION                 -> ==== End of Application Collection ====
};

// Calculate the total length of this descriptor array (Must be reported to the host during transmission)
const uint32_t HID_Mouse_ReportDescSize = sizeof(HID_Mouse_ReportDescriptor);


// Assume this is a low-level physical transport function (e.g., USB_Send_EP1() or I2C_Write())
extern void HW_Transport_SendPayload(uint8_t* data, uint16_t length);

/*
 * @brief Application function to be called in the MCU main loop
 */
void HID_Mouse_SendEvent(int8_t delta_x, int8_t delta_y, uint8_t left_click) {
    // 1. Instantiate the payload structure we defined
    HID_MouseReport_TypeDef report;
    
    // 2. Fill the data strictly according to the contract in the descriptor
    // Forcefully clear unused high bits, reserving only the lowest bit for the left click
    report.buttons = left_click ? 0x01 : 0x00; 
    report.x = delta_x;
    report.y = delta_y;
    
    // 3. Cast these 3 bytes into a uint8_t* array and throw it to Windows via the underlying bus (USB/I2C)!
    HW_Transport_SendPayload((uint8_t*)&report, sizeof(report));
}