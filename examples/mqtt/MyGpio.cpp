/*
  EgeDriver handling GPIO as an example of combining to EdgeUnified.
  MyGpio.cpp is an implementation of GPIO EdgeDriver. EdgeDriver basically
  consists of the following elements:
  1. AutoConnectAux JSON descriptions.
  2. An instance of EdgeDriver.  It involves a EdgeData struct type.
  3. AutoConnectAux custom web page request handlers.
  4. On-demand callback functions.
  5. EdgeData serializer and deserializer in optionally.
  Copyright (c) 2022 Hieromon Ikasamo.
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#include <Arduino.h>
#include <AutoConnect.h>
#include "EdgeUnified.h"
#include "MyGpio.h"

/**
 * GPIO custom Web page descriptions.
 */
const char SETTINGS_GPIO[] PROGMEM = R"*(
{
  "title": "LED",
  "uri": "/gpio_led",
  "menu": true,
  "element": [
    {
      "name": "pin",
      "type": "ACInput",
      "label": "LED Pin",
      "global": true
    },
    {
      "name": "cycle",
      "type": "ACInput",
      "label": "Blinking Cycle [ms]",
      "global": true
    },
    {
      "name": "Apply",
      "type": "ACSubmit",
      "value": "Apply",
      "uri": "/gpio_blink"
    }
  ]
}
)*";

const char BLINKING_GPIO[] PROGMEM = R"*(
{
  "title": "LED",
  "uri": "/gpio_blink",
  "menu": false,
  "element": [
    {
      "name": "pin",
      "type": "ACText",
      "format": "GPIO #%s Blinking",
      "posterior": "div",
      "global": true
    },
    {
      "name": "cycle",
      "type": "ACText",
      "format": "Cycle %s ms",
      "posterior": "div",
      "global": true
    }
  ]
}
)*";

/**
 * Edge entities
 */
EdgeDriver<GPIO_t>  gpio(startGPIO, processGPIO, endGPIO);

/**
 * AutoConnectAux custom web page request handlers.
 */
// AutoConnectAux handler. Transfer EdgeData to AutoConnectElement values
String auxGPIOSetting(AutoConnectAux& aux, PageArgument& args) {
  aux["pin"].as<AutoConnectInput>().value = String(gpio.data.pin);
  aux["cycle"].as<AutoConnectInput>().value = String(gpio.data.cycle);
  return String();
}

// AutoConnectAux handler. Obtain AutoConnectElement values, copy to EdgeData
String auxGPIOApply(AutoConnectAux& aux, PageArgument& args) {
  gpio.data.pin = aux["pin"].as<AutoConnectText>().value.toInt();
  gpio.data.cycle = aux["cycle"].as<AutoConnectText>().value.toInt();
  gpio.setEdgeInterval(gpio.data.cycle);
  gpio.save();
  return String();
}

/**
 * GPIO start callback
 */
void startGPIO() {
  Serial.println("Starting GPIO");
  pinMode(gpio.data.pin, OUTPUT);
  digitalWrite(gpio.data.pin, !LED_ACTIVE);
  gpio.setEdgeInterval(gpio.data.cycle);
}

/**
 * GPIO process callback
 */
void processGPIO() {
  digitalWrite(gpio.data.pin, !digitalRead(gpio.data.pin));
}

/**
 * GPIO end callback
 */
void endGPIO() {
  digitalWrite(gpio.data.pin, !LED_ACTIVE);
}
