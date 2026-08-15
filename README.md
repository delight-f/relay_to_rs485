# Relay to RS485

Mirror relay states over an RS-485 bus using the Modbus protocol.

![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)
![Platform: ESP32-S2](https://img.shields.io/badge/platform-ESP32--S2-blue.svg)
![Framework: Arduino](https://img.shields.io/badge/framework-Arduino-00979D.svg)
![Build: arduino-cli](https://img.shields.io/badge/build-arduino--cli-00979D.svg)

## Overview

An ESP32-S2 reads up to five zero-voltage relay contacts and mirrors their
state onto an RS-485 bus as Modbus coils, so an external master can switch
the corresponding output relays. Developed for amateur radio: retrofitting
an existing proprietary system to allow coil switching — moving antenna
directors or changing bands. Designed to work with commonly available
RS-485 relay boards (AliExpress, eBay, etc.).

The repository contains both the firmware and the KiCad PCB design.

## Repository layout

```
.
├── relay_to_rs485/          # Arduino sketch (firmware)
│   ├── relay_to_rs485.ino   # entry point, setup()/loop()
│   └── debounce.h           # host-testable input debouncer
├── test/
│   └── test_debounce.cpp    # host unit tests for the debouncer
├── Hardware/
│   └── relay_to_rs485/      # KiCad 7 project + gerbers (REV_B)
├── Makefile                 # build/test entry point (arduino-cli)
├── sketch.yaml              # arduino-cli sketch configuration
└── .github/workflows/ci.yml # host tests + firmware build
```

## Hardware

The PCB has eight input pins hardwired; the current firmware uses five,
leaving room for future relay expansion. It was designed around an 8-relay
board.

- KiCad project: [`Hardware/relay_to_rs485/`](Hardware/relay_to_rs485/)
- Manufacturing files (REV_B gerbers):
  [`Hardware/relay_to_rs485/gerbers/`](Hardware/relay_to_rs485/gerbers/)

## Firmware

- Board: LOLIN S2 Mini (ESP32-S2)
- Framework: Arduino, built with [arduino-cli](https://arduino.github.io/arduino-cli/)
- Modbus: [ModbusMaster](https://github.com/4-20ma/ModbusMaster) 2.0.1
- Input filtering: ESP32 hardware glitch filter + software debounce
  (`debounce.h`, host-testable)

### Prerequisites

- [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/) ≥ 1.0
- ESP32 core and ModbusMaster library:

```bash
arduino-cli config add board_manager.additional_urls \
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install ModbusMaster@2.0.1
```

### Build and flash

```bash
git clone git@github.com:delight-f/relay_to_rs485.git
cd relay_to_rs485
make            # build firmware
make upload     # build and flash over USB
```

Or without make:

```bash
arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini relay_to_rs485
arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini relay_to_rs485 --upload
```

### Test

The debounce logic is pure C++ and unit-tested on the host — no Arduino
required:

```bash
make test
```

### Configuration

Pin assignments, Modbus slave ID, and channel count are named `constexpr`
constants at the top of `relay_to_rs485/relay_to_rs485.ino`. Adjust these
to match your wiring before flashing.

| Constant | Default | Meaning |
| --- | --- | --- |
| `kModbusSlaveId` | `255` | Modbus slave ID |
| `kModbusBaud` | `9600` | RS-485 baud rate (8E1) |
| `kModbusRxPin` / `kModbusTxPin` | `31` / `35` | RS-485 UART pins |
| `kNumChannels` | `5` | Number of input/coil pairs |
| `kInputPins` | `15, 17, 18, 19, 20` | Input pins (active-low, pull-up) |
| `kCoilAddresses` | `0x0000…0x0004` | Modbus coil addresses |
| `kPollIntervalMs` | `50` | Loop poll interval |
| `kDebounceMs` | `30` | Software debounce window |
| `kMaxWriteRetries` | `3` | Modbus write retry count |

Inputs are active-low: pull-up means HIGH = relay not active (`false`),
LOW = relay active (`true`).

## Versioning

This project uses [SemVer](https://semver.org/).

## Contributing

Issues and pull requests are welcome. Please open an issue to discuss
significant changes before submitting a PR.

## License

Creative Commons Attribution-ShareAlike 4.0 International License.
See [LICENSE](./LICENSE) for details.

## Acknowledgments

- [ModbusMaster](https://github.com/4-20ma/ModbusMaster) library
- [arduino-cli](https://arduino.github.io/arduino-cli/) toolchain

## Authors

- **de_light** — initial work
