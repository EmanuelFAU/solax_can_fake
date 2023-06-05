# solax_can_fake

based on ESP32 (Sparkfun thing plus) and two mcp2515 breakout boards, this code can change the can watchdogmessage (0x1877), which includes the battery manufacturer code.
Whith this, any sutable battery can be used with the solax x3 hybrid G4, as this inverter checks, if there is a solax battery connected.

The mcp2515 is connected to spi and the following pins as CS:
- SPI_CS_CAN0 14
- SPI_CS_CAN1 21

The direct connection between the mcp2515 and esp32 might be out of voltage specification range of the mcp2515 input, but so far works flawless.

This code is just a quickfix and by far not written well.
