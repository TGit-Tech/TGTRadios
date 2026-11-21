# Ebyte
### Arduino Radio Communication Driver

Ebyte is a radio communication driver for Ebyte LoRa radio modules. It provides a common `RadioI` interface for identifying supported radio modules, reading and configuring radio parameters, managing operating modes, and transmitting and receiving RF packets.

The driver handles the differences between Ebyte radio families internally, allowing the rest of the communication system to work through a common interface.

## Features

- Automatic radio model identification.
- Support for multiple Ebyte radio families.
- Common `RadioI` interface.
- Read and write radio configuration parameters.
- Automatic switching between normal and configuration modes.
- RF address management.
- Frequency and channel configuration.
- UART and air-speed configuration.
- Transmit power configuration.
- Fixed transmission support.
- RSSI, LBT, WOR and FEC settings where supported.
- Packet transmission and reception through `TxPacket` and `RxPacket`.
- Packet validation before reporting received packets.
- AUX pin monitoring for transmission readiness.
- Optional forced radio model selection.
- TGTDB-compatible debug instrumentation.

## Supported Radio Modules

The driver currently declares the following radio models:

| Model | Series | Frequency |
|---|---|---|
| E22-400T30D | E22 | 400 MHz |
| E22-400T33D | E22 | 400 MHz |
| E22-900T30D | E22 | 900 MHz |
| E32-433T20D | E32 | 433 MHz |
| E32-433T30D | E32 | 433 MHz |
| E220-400T30D | E220 | 400 MHz |

**Automatic identification:** The current discovery implementation recognizes E22-400T33D, E22-900T30D, E32-433T20D, E32-433T30D and E220-400T30D. E22-400T30D is declared in the model definitions but is not currently assigned by the automatic discovery routine.

## Hardware Connections

The Ebyte constructor accepts five hardware pin assignments:

```cpp
Ebyte(byte _PinM0, byte _PinM1,
      byte _PinTX, byte _PinRX, byte _PinAUX);
```

| Pin | Function |
|---|---|
| M0 | Radio operating mode control |
| M1 | Radio operating mode control |
| TX | UART transmit |
| RX | UART receive |
| AUX | Radio busy/ready indication |

Pins assigned a value of `0` are excluded from the constructor's initial `pinMode()` configuration. Supply valid pin assignments for the connections used by the driver.

The UART itself is provided through the `RFSERIAL` interface defined by the supporting RFC infrastructure. Serial initialization must be handled by the surrounding system.

## Radio Configuration

The driver expects the radio configuration to use the following communication settings:

| Parameter | Required setting |
|---|---|
| UART baud rate | 9600 bps |
| UART format | 8N1 |
| Air data rate | 2.4 kbps |
| FEC | Enabled |
| Fixed transmission | Enabled |
| WOR timing | 250 ms |
| IO drive | Push-Pull |
| RF address | Configured through radio menu |
| Channel | Configured through radio menu |
| Transmit power | Configured through radio menu |

**Important:** New Ebyte radio installations must be configured for Fixed Transmission mode. This was an important discovery during initial testing.

Use the Ebyte Radio Configuration PC software to verify or change radio settings.

Before connecting with the configuration software:
1. Disable radio DEBUG output.
2. Configure both serial connections for 9600 baud.
3. Connect to the radio using the PC configuration interface.
4. Verify the radio settings on both communicating modules.

## Installation

Include the driver in your Arduino project:

```cpp
#include "Ebyte.h"
```

The driver depends on the supporting RFC communication infrastructure and its associated packet definitions.

Dependencies include:
- `RFC.h` — Radio interface and packet infrastructure.
- `ArduinoQueue` — Transmission queue handling.
- Arduino hardware interfaces — GPIO, UART and timing.
- EEPROM — Persistent RF address storage.
- TGTDB — Optional debug instrumentation.

## Creating an Ebyte Radio

Instantiate the driver with the five hardware pin assignments:

```cpp
Ebyte RF(6, 7, 18, 19, 20);
```

The constructor initializes the assigned pins and immediately attempts to identify the connected radio.

The discovered radio model is available through:

```cpp
byte Model = RF.RadioID();
```

The returned value corresponds to one of the `EM_` model definitions.

## Operating Modes

Radio operating modes are defined as:

| Constant | Mode |
|---|---|
| `ERFNORMMODE` | Normal operation |
| `ERFWAKEMODE` | Wake-up mode |
| `ERFPWRSAVE` | Power-saving mode |
| `ERFSETMODE` | Configuration mode |

Set or retrieve the current radio mode:

```cpp
RF.Mode(ERFNORMMODE);

byte CurrentMode = RF.Mode();
```

The driver handles the M0/M1 pin combinations according to the identified radio family.

## Radio Configuration API

The driver provides matching SET and GET methods for most radio configuration parameters.

| Method | Description |
|---|---|
| `RadioID()` | Retrieve identified radio model |
| `Address(value)` | Set RF address |
| `Address()` | Get RF address |
| `AirSpeed(value)` | Set air data rate |
| `AirSpeed()` | Get air data rate |
| `UARTSpeed(value)` | Set UART speed and parity |
| `UARTSpeed()` | Get UART speed and parity |
| `Frequency(value)` | Set operating frequency |
| `Frequency()` | Get operating frequency |
| `Channel()` | Get radio channel |
| `TxPower(value)` | Set transmit power |
| `TxPower()` | Get transmit power |
| `NetID(value)` | Set network ID (supported E22 models) |
| `NetID()` | Get network ID |

### Additional Configuration

| Method | Description |
|---|---|
| `FixedTransmission(value)` | Enable or disable fixed transmission |
| `FixedTransmission()` | Get fixed transmission setting |
| `E32IODrive(value)` | Configure E32 IO drive |
| `E32IODrive()` | Get E32 IO drive setting |
| `RSSIOn(value)` | Enable or disable RSSI |
| `RSSIOn()` | Get RSSI setting |
| `LBTOn(value)` | Enable or disable Listen Before Talk |
| `LBTOn()` | Get LBT setting |
| `WUORTime(value)` | Set WOR timing |
| `WUORTime()` | Get WOR timing |
| `E32FECOn(value)` | Enable or disable E32 FEC |
| `E32FECOn()` | Get E32 FEC setting |
| `E22WOROn(value)` | Enable or disable E22 WOR |
| `E22WOROn()` | Get E22 WOR setting |

Configuration methods switch the radio into configuration mode as necessary, write the parameters, reload the radio configuration, and return the radio to its previous operating mode.

Some settings are specific to particular Ebyte families. Unsupported settings are ignored or are not available for meaningful retrieval.

## RF Address and EEPROM

The RF address is a 16-bit value.

```cpp
RF.Address(0x0001);

unsigned int MyAddress = RF.Address();
```

The driver also maintains the RF address in EEPROM at address `0`.

During radio initialization, the driver reads the radio's current RF address and compares it with the EEPROM value. If they differ, the radio's address is copied into EEPROM.

The RF address therefore remains associated with the installed radio configuration.

**Note:** EEPROM address `0` is reserved by this implementation for the RF address.

## Packet Transmission

Transmission uses the RFC packet infrastructure.

```cpp
RF.Send(TxPacket* Tx);
```

The driver handles the radio-specific transmission process, including:

- Checking radio operating mode.
- Waiting for the AUX pin to indicate readiness.
- Applying the transmitting radio address.
- Inserting the current channel.
- Securing the packet.
- Preparing the transmission queue.
- Transmitting packet data through `RFSERIAL`.

The driver also handles extended packet data using `TxPacket::ExtraBytes` when required.

The transmission process uses the radio's AUX signal to determine when the module is ready for additional data.

## Packet Reception

Use `PacketAvailable()` to check for incoming packets:

```cpp
if (RF.PacketAvailable()) {
    // A complete, valid packet has been received.
}
```

The driver collects incoming UART bytes and builds an `RxPacket`.

Once a complete packet is received, the driver validates it using `RxPacket::IsValid()`.

Only valid, complete packets cause `PacketAvailable()` to return `true`.

The received packet is exposed through the public pointer:

```cpp
RxPacket* Packet = RF.Packet;
```

Packet assembly is handled internally, including incomplete packet collection and expiration of stale packet data.

## Debugging

The implementation includes TGTDB debug instrumentation.

Debug output can help identify:
- Radio discovery and identification.
- Radio parameter reads and writes.
- Operating mode changes.
- UART communication.
- Packet reception and validation.
- Packet transmission and queue handling.

Debug instrumentation is controlled through the TGTDB configuration macros.

For radio configuration using the Ebyte PC software, disable DEBUG output and ensure the UART is operating at 9600 baud.

## Forced Radio Model

For testing or installations where automatic identification is not desired, a radio model can be specified at compile time:

```cpp
#define FORCERADIOMODEL EM_E22_400T33D
```

Place the definition before compiling the driver.

When defined, the specified model overrides the automatically discovered model.

## Design

`Ebyte` inherits from `RadioI`, providing a common radio interface to the rest of the communication system.

Radio-specific configuration registers, command sequences, operating modes and UART communication remain encapsulated inside the Ebyte implementation.

The intention is to allow the communication engine to work with different radio hardware without requiring radio-specific handling throughout the application.

---

**Ebyte — TGit-Tech**

Radio communication infrastructure for Ebyte LoRa modules.