# COOLRF HeatStick for ESPHome

Modern ESPHome firmware for the original COOLRF HeatStick (ESP8285) used with
Ballu Digital Inverter controllers, including BCT/EVU-I.

## What changed

- Migrated from the removed `custom_component` API to a local external component.
- Added a native Home Assistant climate entity.
- Fixed the mode feedback loop that could repeatedly switch the heater after
  changing from `No frost` back to `Comfort`.
- Added a streaming UART parser that handles fragmented and consecutive packets.
- Unknown protocol values are logged and ignored instead of crashing the device.
- Added periodic state synchronization and safe command handling before the
  first valid state packet is received.
- Updated the configuration for ESPHome 2026.9 and its current OTA syntax.

## Hardware

Target board: the original COOLRF HeatStick with ESP8285.

The heater communication bus uses UART0 at 9600 baud:

- TX: GPIO1
- RX: GPIO3
- 8 data bits, no parity, 1 stop bit

Serial logging is disabled because the same UART is connected to the heater.
Runtime logs remain available through the ESPHome native API.

## Build

1. Copy `secrets.example.yaml` to `secrets.yaml`.
2. Replace all example values with your Wi-Fi credentials.
3. Validate and compile:

```powershell
.\.venv\Scripts\esphome.exe config coolrf-heatstick.yaml
.\.venv\Scripts\esphome.exe compile coolrf-heatstick.yaml
```

The initial serial image is generated as:

```text
.esphome/build/ballu-heatstick/.pioenvs/ballu-heatstick/firmware.factory.bin
```

## First installation

Disconnect the HeatStick from the heater before connecting a 3.3 V USB-to-UART
programmer. Never connect the programmer while the HeatStick is powered by the
heater. Back up the existing flash before erasing or writing it.

Do not install the current build on an unattended heater. The first hardware
test must be supervised and should verify:

1. State is received without UART checksum errors.
2. On/off and target temperature each generate one command.
3. `Comfort -> No frost -> Comfort` does not cause repeated switching.
4. Controls on the heater are reflected in Home Assistant without being echoed
   back as new commands.

## Repository layout

- `coolrf-heatstick.yaml` — current ESP8285 configuration.
- `components/heatstick/` — maintained ESPHome component.
- `coolrf-heatstick-legacy.yaml` and `coolrf-heatstick.h` — original firmware,
  retained for protocol comparison.
- `README-legacy.md` — original project documentation.

## Status

Version `0.2.2-dev` compiles successfully for ESP8285 with ESPHome 2026.9.0
and has been tested on a Ballu Digital Inverter BCT/EVU-I. Home Assistant,
the web interface, climate control, display control, operating modes and the
`Comfort -> No frost -> Comfort` feedback-loop fix have been verified.

Automatic power control works. Manual `Level 1` through `Level 5` commands are
experimental: the heater acknowledges the selection but may continue to use
its own automatic power level. Until the protocol is captured from the physical
control panel, use `Auto` for normal operation.
