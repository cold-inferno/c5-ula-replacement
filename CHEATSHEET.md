# C5 ULA replacement — cheatsheet (firmware V1203)

Everything the pod, the buzzer, the serial port and the ride log can tell you.
LED1 is the **bottom** LED of a bar, LED5 the **top** one.

## Pod: right bar = battery

Uses the *resting* voltage: measured when the motor has been off for about 3 seconds
(button released, or relay off). It does not sag while you accelerate.

| LEDs lit | Resting voltage | Charge (approx.) |
|---|---|---|
| 5 | 13.30 V or more | 90 % or more |
| 4 | 13.15 V | 55 % |
| 3 | 13.05 V | 35 % |
| 2 | 12.90 V | 20 % |
| 1 | 12.50 V | 14 % |
| 0 | below 12.50 V | nearly empty |
| all 5 flashing fast | battery error, motor is cut | see fault codes |

## Pod: left bar, no fault

| Situation | The bar shows | Scale |
|---|---|---|
| Button released | speed | 1 LED per 5 km/h: 5, 10, 15, 20, 25 km/h. Above 30 km/h the top LED flashes. |
| Button pressed | motor current | LED1 any current, LED2 5 A, LED3 10 A, LED4 15 A, LED5 20 A or more |

## Pod: left bar, fault codes

While any fault is active the left bar shows only the fault: one LED per kind of fault,
all other LEDs off. **Slow flash (1 × per second) = warning. Fast flash (4 × per second) = error.**
The buzzer beeps in the same rhythm. Several faults can show at once.

| Left bar | Buzzer | Fault | Triggered by | Motor | Clears when |
|---|---|---|---|---|---|
| LED1 slow | silent | low battery warning | resting voltage below 12.8 V (about 15 %) | runs | resting voltage 12.9 V or more |
| LED1 fast, right bar all fast | silent | battery error | resting voltage below 12.0 V, or 20-second average below 11.0 V under load | **cut** | resting voltage 12.3 V or more |
| LED3 slow | slow beeps | current warning | motor current above 140 A right now | **cut** while it lasts | current drops (retries after about 0.5 s) |
| LED3 fast | fast beeps | current error: **stall protection** | motor current above about 80 A for 3 seconds while the wheel sensor gives no pulse, or the third 140 A cut in a row | **cut** | **release the button**, then press again |
| LED3 fast | fast beeps | current error: overload | average current above 140 A for 10 seconds | **cut** | the 10-second average drops |
| LED5 slow | slow beeps | temperature warning | motor at 70 °C or more | runs | below 70 °C (counts down, up to 20 s) |
| LED5 fast | fast beeps | temperature error | motor at 80 °C or more, or at 70–79 °C for 20 seconds; also a disconnected sensor | **cut**, fan on | below 70 °C |

Other things the pod can show:

| What you see | Meaning |
|---|---|
| LED3 lit on both bars for 2 seconds after switching on | bootloader waiting for the configurator; normal |
| a single LED running up a bar | firmware upload in progress |
| right bar LED3 flickering, nothing else | configuration mode (bootloader); click *Start C5* or switch off and on |
| display frozen, motor dead, fan on, no serial data | firmware self-test failed (memory checksum): upload the firmware again |
| nothing at all | no 5 V supply, or (ATmega328P) supply below 4.3 V |

Stall protection notes: the 3-second rule only works once the wheel sensor has given a pulse
since switching on (a C5 without a wheel sensor is only protected by the "third 140 A cut" rule).
One wheel turn every 3 seconds is about 1.5 km/h: crawling slower than that at high current counts
as a stall.

## Motor relay: when does the motor run?

The handlebar button powers the relay; the firmware can only *allow* the motor.

| Pedal assist setting | Motor runs when |
|---|---|
| off | button pressed, no error |
| on | button pressed, no error, pedalling (a pedal pulse within the last 2 s), speed at or above the minimum, and below the maximum (if a maximum is set) |

## Serial status line (9600 baud, 4 × per second)

`#DIST|SPEED|RPM|TIMEOUT|SWITCH|TEMP|VOLT|VREF|CURR|ENA|COND|TIME|REST|`

| Field | Meaning | Conversion |
|---|---|---|
| DIST | trip distance | mm |
| SPEED | speed | 0.1 km/h |
| RPM | pedal cadence | rpm |
| TIMEOUT | time since the last pedal pulse | ms; below 2000 = pedalling |
| SWITCH | handlebar button | raw; above 300 = pressed |
| TEMP | motor temperature sensor | raw; the configurator converts it |
| VOLT | battery voltage now | volts ≈ (VOLT + 32) / 48 |
| VREF | pod potentiometer | only used by the lead-acid build |
| CURR | motor current | amps ≈ CURR × 140 / 523 |
| ENA | relay allowed | 1 = motor allowed |
| COND | fault flags, added up | see below |
| TIME | uptime | ms |
| REST | resting battery voltage | same conversion as VOLT |

**COND** is the sum of the active flags (0 = all OK). Example: 37 = 32 + 4 + 1.

| Value | Flag |
|---|---|
| 1 | low battery warning |
| 2 | current warning |
| 4 | temperature warning |
| 8 | battery error |
| 16 | current error |
| 32 | temperature error |

Start-up lines: `ULA replacement V…` (version), `CRC:` (checksum), `BAT:` (battery profile).
Firmware up to V1202 also printed the settings here (`PM:`, `WS:`, `LMX:`, `LMN:`, `FT:`, `NMT:`,
`NMP:`, `LOG:`); from V1203 on, read them in configuration mode.

## Ride log (optional, off by default)

| Code | Event | Note |
|---|---|---|
| 0x01 | start: power-on | normal |
| 0x02 | start: brown-out | supply dipped; a quick off-and-on is also recorded like this |
| 0x03 | start: watchdog | firmware hung; the bootloader's reset command also shows as this |
| 0x04 | start: external reset | reset pin, e.g. a USBasp programmer |
| 0x05 | start: cause unknown | bootloader older than V1044 |
| 0x10 | low battery warning | |
| 0x11 | battery error | motor cut |
| 0x12 | current warning | motor cut for a moment |
| 0x13 | current error | motor cut |
| 0x14 | temperature warning | |
| 0x15 | temperature error | motor cut |
| 0x16 | temperature sensor reads about 0 | sensor disconnected? |
| 0x20 | black box saved | the last 60 seconds before an unexpected restart were kept |

Black box flags per second: relay, button, pedal, VOLT, CURR, TEMP (warning or error of that
kind), THERM (sensor reading implausibly low).

## Bootloader answers

| Byte | Meaning |
|---|---|
| `B` | bootloader active (configuration mode) |
| `O` | upload block accepted |
| `E` | upload block had a wrong checksum |
| `D` | done (setting written, or upload finished) |
| `S` | starting the firmware |
| `?` | unknown command |
