
# Stdlib

## tprintf()


| Token  | Type               | Formatted As       | Example   |
|--------|--------------------|--------------------|-----------|
| %a     | struct astring\*   | String             | Test      |
| %c     | char               | Single character   | T         |
| %d     | int                | Decimal number     | 16        |
| %s     | char\*             | String             | Test      |
| %x     | int                | Hexidecimal number | 0xf       |

# Defines

| Definition      | Type    | Notes                                  |
|-----------------|---------|----------------------------------------|
| QD\_CONSOLE\_IN   | String  | Device to input from (uart/keyboard)   |
| QD\_CONSOLE\_OUT  | String  | Device to output on (uart/display)     |
| QD\_SPI          | On/Off  | SPI enabled (where supported)          |
| QD\_CPU\_MHZ      | Integer | CPU clock speed (where supported)      |
| QD\_UART\_SW      | On/Off  | Enable software UART                   |
| QD\_UART\_HW      | On/Off  | Enable hardware UART                   |

# Platform-Specific Notes

While this project is intended to be cross-platform, there will be differences
between platfornms. Those differences should be noted here.

## MSP430

### Serial Ports

Serial ports may be enabled or disabled, but if they are enabled, then certain
ports will always have the same ID.

| UART Index | Type     | Pins
|------------|----------|
| 0          | Software | 
| 1          | Hardware |
| 2          | Hardware |
| 3          | N/A      |

