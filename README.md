# Relay to RS485

Mirror a relay state with RS485 and the ModBus protocol.

This project was developed for use in amateur radio. A zero-voltage relay state is measured by an ESP32 input pin and this is reflected across using the ModBus protocol. It includes firmware and a PCB board for use.

Use-cases include retrofitting into an existing, proprietary system to allow coil switching to move directors or change bands. Uses commonly available RS485-enabled relay boards available on Aliexpress/eBay and the like.

It uses the ModBusMaster library and ESP32 glitch filters.

## Variables

This build uses five ESP32 input pins in the code but the board has eight input pins hardwired. This is to allow future addition of relays as this was designed for use with an 8-relay board.

## Built With

* [VS Code and PlatformIO) - Arduino framework
  
## Versioning

We use [SemVer](http://semver.org/) for versioning.

## Authors

* **de_light** - *Initial work* - [de_light]([https://github.com/de_light](https://github.com/delight-f))

## License

This project is licensed under the Creative Commons Attribution Share Alike 4.0 International License.

## Acknowledgments

* ModBusMaster library
