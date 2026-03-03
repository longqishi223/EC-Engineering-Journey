# 🚀 Deep Dive: eSPI (Enhanced Serial Peripheral Interface)

This folder explores eSPI, the modern, high-speed, low-voltage (1.8V) bus that replaced the legacy LPC (Low Pin Count) bus for communication between the Embedded Controller (EC) and the Host (Intel/AMD PCH).

## 1. The Four Channels of eSPI
Unlike traditional SPI, eSPI multiplexes four distinct logical channels over the same physical pins:
1. **Peripheral Channel (PC):** Replaces legacy LPC I/O and memory cycles (e.g., KBC ports 60h/64h).
2. **Virtual Wire Channel (VW):** The most revolutionary feature. It serializes physical sideband pins (like `SMI#`, `SCI#`, `RCIN#`, `SLP_S3#`) into digital packets.
3. **Out-of-Band Channel (OOB):** Used for management data (like SMBus tunneling) without waking up the main CPU.
4. **Flash Access Channel (FAC):** Allows the EC to share the main SPI BIOS flash with the PCH.

## 2. The Project: Virtual Wire (VW) Simulation
In the legacy days, if the EC wanted to trigger an ACPI System Control Interrupt (SCI) to tell the OS about a low battery, it would pull a dedicated physical `SCI#` copper trace low. 

With eSPI, that physical wire no longer exists. Instead, the EC constructs a **Virtual Wire Packet** containing an Index (identifying the wire group) and Data (the state of the wires), and transmits it to the PCH.

**This project simulates:**
1. The EC detecting a critical hardware event.
2. The EC constructing an eSPI Virtual Wire packet to assert `SCI#`.
3. The hardware registers involved in transmitting this packet.