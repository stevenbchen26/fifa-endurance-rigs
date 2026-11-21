# Solenoid Endurance Test Rigs

## Overview

Endurance test rigs built to durability-test the solenoid kicker used in a robot soccer project at Humber Polytechnic. The main rig fires a solenoid at a ball on a fixed cycle and automatically verifies, after every kick, whether the ball reached its target height. A second, stripped-down rig runs the same firing cycle in parallel with no sensing, for hammer-style solenoid testing. The point of the rigs is unattended, repeatable testing: the rig counts successful hits, detects failures on its own, and halts when the failure criterion is met.

## Features

- Fixed firing cycle: 50 ms ON / 950 ms OFF (1 Hz, 5% duty cycle, well under the solenoid's 30% limit)
- Automatic hit verification with an E18-D80NK proximity sensor after each kick
- Failure detection: 5 consecutive missed detections halt the test and print the final hit count to Serial
- Hit count reported every 10 hits over Serial at 115200 baud
- Solenoid driven through a MOSFET driver board from a LoR Core v3 output

## Hardware

- LoR Core v3 board (ESP32)
- Solenoid on a vertical mount with a ball guide (main rig)
- D4184-class MOSFET driver boards (see the failure notes below)
- E18-D80NK proximity sensor: IO slot 10 / GPIO4 on the main rig
- MOSFET signal: IO slot 1 / GPIO32 on `kick_to_death_v6`; IO slot 12 / GPIO21 on `solenoid_test`

## Software and architecture notes

- Developed in PlatformIO with VS Code (Arduino framework for ESP32)
- Serial monitor is the data channel. MicroSD logging was wired and debugged, then scrapped at the project manager's direction, so no logging code ships in these builds
- The main rig's firmware iterated v3 → v6; v6 is the final version

## File and version guide

| File | Description |
|---|---|
| `kick_to_death_v6.ino` | Main ball rig, final version: 1 Hz firing cycle, E18-D80NK hit verification, 5-consecutive-miss halt, Serial reporting |
| `solenoid_test.ino` | Second rig, stripped build: same 50/950 ms cycle, solenoid only, no sensor and no SD; reports hit count every 10 kicks |

## Engineering notes

- **MOSFET driver failures, root-caused:** two D4184-class driver boards failed during the testing program. The first failure was a miswiring incident that shorted the MOSFET gate, causing thermal runaway and smoke. The second failure, during remounting, was traced to the same root cause; the board was confirmed failed-shorted, holding the solenoid extended at full 12 V with no control signal. A replacement board worked once the wiring was verified clean. The lesson recorded at the time: verify wiring before power, every time, especially after any remount.
- **Power supply masquerading as a sensor fault:** an early "sensor not detecting" problem was actually insufficient AA battery power. Switching to a proper battery/external supply fixed it, and confirmed the LoR Core v3 can run indefinitely from a power supply.
- **Connector mix-up:** the MicroSD debugging identified a wrong-connector error (IO port vs AUX port) and the correct pin order for the adapter before logging was descoped.
- **Open item:** a formal failure criterion (visible cracking, permanent deformation, or consistent failure to reach sensor height) was still to be documented, with findings to be reported at a statistically meaningful hit count.
