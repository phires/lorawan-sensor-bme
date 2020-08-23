# LoRaWAN sensor with BME280
## Hardware
I'm using the LoRa Radio Node v1.0, see here: https://github.com/IOT-MCU/LoRa-Radio-Node-v1.0

As sensor I'm using the BME280.

## Software
The project is created with PlatformIO in VSCode.

The code is fetched together from the internet, primarily here: https://www.disk91.com/2019/technology/lora/first-steps-with-lora-radio-node-arduino/

## Config

You need to copy the file lib/loraWan/config.h.template to lib/loraWan/config.h and enter your data retrieved from the TTN console after registering a new device (OTAA)