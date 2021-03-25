
# elix

elix is a hobby-grade "operating system"-sh thing designed to be portable and explore various ideas out of curiousity. It is not intended to be used in production of any sort.

In line with the goals of system-level software coming out of Project Free Time, elix should be:

* Portable to very old and/or very constrained systems.
* "Interesting" from a paradigm perspective OR
* "Useful" from a personal (to indigoparadox) perspective.

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

