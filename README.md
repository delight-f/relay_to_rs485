# Relay to RS485

Mirror a relay state over RS485 using the Modbus protocol.

![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)
![Platform: ESP32](https://img.shields.io/badge/platform-ESP32-blue.svg)
![Framework: Arduino](https://img.shields.io/badge/framework-Arduino-00979D.svg)

## Overview

Developed for amateur radio use. An ESP32 input pin measures a zero-voltage relay state and reflects it across an RS485 bus using Modbus. The repository includes firmware and a PCB design.

**Use cases:** retrofitting into an existing proprietary system to allow coil switching — for example, moving antenna directors or changing bands. Designed to work with commonly available RS485 relay boards (AliExpress, eBay, etc.).

## Hardware

The PCB has eight input pins hardwired; the current firmware uses five, leaving room for future relay expansion. It was designed around an 8-relay board.

PCB design files are in [`/hardware`](./hardware).

## Firmware

- Framework: Arduino, built with [PlatformIO](https://platformio.org/)
- Modbus: [ModbusMaster](https://github.com/4-20ma/ModbusMaster)
- Debounced input handling and ESP32 glitch filters on all relay input pins

### Prerequisites

- [PlatformIO Core](https://platformio.org/install) or the PlatformIO VS Code extension
- USB-to-serial access to the target ESP32 board

### Build and flash

```bash
git clone https://github.com/de-light/relay-to-rs485.git
cd relay-to-rs485
pio run -t upload
```

### Configuration

Pin assignments, Modbus slave ID, and channel count are set as named constants at the top of `src/main.cpp`. Adjust these to match your wiring before flashing.

## Versioning

This project uses [SemVer](https://semver.org/).

## Contributing

Issues and pull requests are welcome. Please open an issue to discuss significant changes before submitting a PR.

## License

Creative Commons Attribution-ShareAlike 4.0 International License. See [LICENSE](./LICENSE) for details.

## Acknowledgments

- [ModbusMaster](https://github.com/4-20ma/ModbusMaster) library

## Authors

- **de_light** — initial work
