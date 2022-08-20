# EdgeUnified evaluation

[AutoConnect](https://github.com/Hieromon/AutoConnect) utilization aid to integrate individual device and custom web pages for Arduino ESP8266/ESP32.

**This project is a pilot and this repository exists to evaluate the feasibility of the goals of this project.**

## Aims of EdgeUnified

By leveraging EdgeUnified, Sketch can integrate multiple devices and their corresponding custom web pages into a single application. The library provides classes and programming models for integration and extends the functionality of [AutoConnect](https://github.com/Hieromon/AutoConnect), a WiFi connectivity aid for ESP8266/ESP32.

##

Sketches using AutoConnect, for example, owns the logic around IO, such as MQTT publishing and GPIO, and the UI sketch for manipulating it on a web page. These are formed by seemingly unrelated independent logic, but trying to integrate them into one tends to complicate the main sketch to avoid inconsistent processing order and conflicting variables.

In addition, similar processes that are normally required, such as loading custom web pages or saving configuration parameters, are scattered throughout the source code, reducing maintainability.

EdgeUnified performs these normally required routine processes on behalf of the user sketch, combining the device IO and [AutoConnect custom web page](https://hieromon.github.io/AutoConnect/acintro.html) pairs into a single event loop. It has the following features:

1. EdgeUnified controls the main event loop. In the sketch, the setup function just attaches the pair of the device IO processing and the AutoConnectAux JSON definition of the corresponding UI implementation. Then, the loop function simply calls the Edge.process function.
2. Various parameters required for device IO are saved and restored by EdgeUnified. There is no need to write those file IO operations in the user sketch.
3. Allows periodicity in device IO processing. Periodic execution operations are performed by EdgeUnified.

## To lead a project to the goal

I welcome contributions to the EdgeUnified. There are many topics to pursue availability in the EdgeUnified programming model, including functions to be implemented, use cases to be assumed, and constraints to be met. For themes suggestions, please go to the [Discussions](https://github.com/Hieromon/EdgeUnified-eval/discussions).

[Issues](https://github.com/Hieromon/EdgeUnified-eval/issues) should be presented with the minimum reproducible code and hardware conditions.

**Please also follow along with [this Discussions](https://github.com/Hieromon/AutoConnect/discussions/515).** You can see the origins of EdgeUnified there.

## EdgeUnified Programming model

Currently in writing. The draft is [here](https://github.com/Hieromon/AutoConnect/discussions/515#discussioncomment-3314411).

## EdgeUnified classes

Currently in writing. Please refer to the function header comments in the source code for now.

## Requirements

### Supported hardware

  * [X] Generic ESP8266 modules (applying the ESP8266 Community's Arduino core)
  * [X] Adafruit HUZZAH ESP8266 (ESP-12)
  * [X] ESP-WROOM-02
  * [X] Heltec WiFi Kit 8
  * [X] NodeMCU 0.9 (ESP-12) / NodeMCU 1.0 (ESP-12E)
  * [X] Olimex MOD-WIFI-ESP8266
  * [X] SparkFun Thing
  * [X] SweetPea ESP-210
  * [X] ESP32Dev Board (applying the Espressif's arduino-esp32 core)
  * [X] SparkFun ESP32 Thing
  * [X] WEMOS LOLIN D32
  * [X] Ai-Thinker NodeMCU-32S
  * [X] Heltec WiFi Kit 32
  * [X] M5Stack
  * [X] And other ESP8266/ESP32 modules supported by the Additional Board Manager URLs of the Arduino-IDE.

### Required libraries

EdgeUnified requires the following environment and libraries.

#### Arduino IDE

The current upstream at the 1.8 level or later is needed. Please install from the [official Arduino IDE download page](https://www.arduino.cc/en/Main/Software). This step is not required if you already have a modern version.

#### ESP8266 Arduino core

AutoConnect targets Sketches made on the assumption of [ESP8266 Community's Arduino core](https://github.com/esp8266/Arduino). Stable 3.0.0 or higher required and the [latest release](https://github.com/esp8266/Arduino/releases/latest) is recommended.  
Install third-party platform using the *Boards Manager* of Arduino IDE. Package URL is http://arduino.esp8266.com/stable/package_esp8266com_index.json

#### ESP32 Arduino core

Also, to apply AutoConnect to ESP32, the [arduino-esp32 core](https://github.com/espressif/arduino-esp32) provided by Espressif is needed. Stable 2.0.4 or required and the [latest release](https://github.com/espressif/arduino-esp32/releases/latest) is recommended.  
Install third-party platform using the *Boards Manager* of Arduino IDE. You can add multiple URLs into *Additional Board Manager URLs* field, separating them with commas. Package URL is https://dl.espressif.com/dl/package_esp32_index.json for ESP32.

#### Additional library (Required)

1. [AutoConnect](https://github.com/Hieromon/AutoConnect) library to handling the portal.
2. [PageBuilder](https://github.com/Hieromon/PageBuilder) library to build HTML.
3. [ArduinoJson](https://github.com/bblanchon/ArduinoJson) library to make persistence EdgeData.

## Installation

Clone or download from the [EdgeUnified-eval](https://github.com/Hieromon/EdgeUnified-eval) GitHub repository.

## License

Copyright (c) Hieromon Ikasamo. All rights reserved.

Licensed under the [MIT](https://github.com/Hieromon/EdgeUnified-eval/blob/main/LICENSE) license.

## Special thanks

This project was inspired by the inspiration of @FRANAIRBUS. I thank giving a new perspective.
