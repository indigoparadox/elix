
# elix

elix is a hobby-grade "operating system"-sh thing designed to be portable and explore various ideas out of curiousity. It is not intended to be used in production of any sort.

elix initially started as MiSPOS, a relatively high-level "framework" with hardware drivers, intended for the rapid development of MSP430 software. elix has deviated from this quite a bit, and now has the following aims:

 * Cross-platform source compatibility for software written to run under elix.
 * Ability to be ported (relatively) easily to new platforms.
 * A layer of "drivers" that sit between the common elix core and hardware.
 * For these reasons, a core written in entirely in C.

## Virtual Macine

The elix virtual machine is stack-based, and provides a limited number of opcodes. These can be executed continuously in a cross-platform, multitasking environment. The opcodes are continually streamed from the disk layer.

This approach provides the following benefits, pursuant to elix's purpose:

 * Flexibility: Streaming from disk allows functioning in extremely RAM-limited environments.
 * Portability: This approach can be binary-compatible, as well as source-compatible.
 * Security: With proper setup, the VM can isolate tasks from each other.
 * Multitasking: This avoids setjmp/longjmp, for which extremely low-resource environments lack available memory.

the primary drawbacks at the moment are:

 * Code Size: The opcodes take up space in the core.
 * Speed: Streaming from the disk severely limits performance.
 * Hardware: Low-resource environments may require hardware they would not ordinarily need (extra RAM, a disk interface).

## Status

![Code Grade](https://www.code-inspector.com/project/579/status/svg)

Code inspector static analysis results available at: https://www.code-inspector.com/project/dashboard/579

Current documentation is available at: https://indigoparadox.github.io/elix/

## Defines

| Definition      | Type    | Notes                                  |
|-----------------|---------|----------------------------------------|
| QD\_CONSOLE\_IN   | String  | Device to input from (uart/keyboard)   |
| QD\_CONSOLE\_OUT  | String  | Device to output on (uart/display)     |
| QD\_SPI          | On/Off  | SPI enabled (where supported)          |
| QD\_CPU\_MHZ      | Integer | CPU clock speed (where supported)      |
| QD\_UART\_SW      | On/Off  | Enable software UART                   |
| QD\_UART\_HW      | On/Off  | Enable hardware UART                   |

## Platform-Specific Notes

While this project is intended to be cross-platform, there will be differences
between platfornms. Those differences should be noted here.

### MSP430

#### Serial Ports

Serial ports may be enabled or disabled, but if they are enabled, then certain
ports will always have the same ID.

| UART Index | Type     | Pins
|------------|----------|
| 0          | Software | 
| 1          | Hardware |
| 2          | Hardware |
| 3          | N/A      |

