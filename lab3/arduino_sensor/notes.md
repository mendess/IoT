# Best version of temperature calculation

## Method

Every 10000 iterations write 'r\n' to the serial port. Run the program for 5
minutes and gather how many 'r\n' are written to the serial port.

| version       | calculation                                   | iter/s |
| ---           | ---                                           | ---    |
| float 1       | `(((voltage / 1024.0) * 5.0) - 0.5) * 100.0;` | 12400  |
| float 2       | `((500.0 * voltage) / 1024.0) - 50.0;`        | 13066  |
| short (16bit) | `((50 * voltage) / 102) - 50;`                | 14600  |
| long  (32bit) | `((500 * (u32) voltage) >> 10) - 50;`         | 15533  |

For the `short` version, the formula had to be adapted to avoid overflows, hence
the `50` instead of `500` and the `102` instead of the `1024`.
