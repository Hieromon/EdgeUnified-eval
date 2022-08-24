/*
  EgeDriver handling GPIO as an example of combining to EdgeUnified.
  gpio.hpp is an implementation of GPIO EdgeDriver. EdgeDriver basically
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

#include "EdgeUnified.h"

/*
  The gpio.hpp source code file is not a compilation unit. It suggests a way
  to write EdgeDriver source code files as pseudo discrete modules while
  still following the contexts of the One Definition Rule, the C++ standard.
  Although it has the appearance of a pseudo-module, it is different from the
  modules implemented by scripting language processors such as JavaScript and
  Python.
  Essentially a module is a file that can share declarations and definitions
  between source files but does not leak macro definitions or private
  implementation details, providing safety assurance while avoiding name
  collisions. Even if modules are imported in a different order, they are safe
  and do not create undefined states.
  But no such processing system is provided in Xtensa gcc tools set based on
  C++ 11. Since standard C++ 20 commits to module implementation.
  An indefinite module such as the one presented by this example has the
  following limitations:
  1. Use namespace to limit the scope of definitions and declarations.
  2. Exclude from namespace the global instances referenced by EdgeDeriver
     such as AutoConnect, WebServer, PubSubClient, etc.
     Therefore, they should not be redefined in other module source files or
     in the main source file.
  3. The location of the module.hpp included in the main source file must be
     guaranteed to be forward referenced.
  4. Depending on the order of inclusion, undefined declarations may occur.
 */

namespace EdgeGPIO {

#if defined(ARDUINO_ARCH_ESP8266)
const uint8_t LED_ACTIVE = LOW;
#elif defined(ARDUINO_ARCH_ESP32)
const uint8_t LED_ACTIVE = HIGH;
#endif

// Define EdgeData structure for GPIO.
// This is the data structure handled by GPIO EdgeDriver. It has GPIO pins
// connected to LEDs and a blinking cycle.
#ifndef LED_BUILTIN
#pragma message("Warning, LED_BUILTIN is undefined. Assumes Pin #2.")
#define LED_BUILTIN 2
#endif

typedef struct {
  int     pin = LED_BUILTIN;
  unsigned long cycle = 500;
} GPIO_t;

/**
 * GPIO custom Web page descriptions.
 */
static const char SETTINGS_GPIO[] PROGMEM = R"*(
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

static const char BLINKING_GPIO[] PROGMEM = R"*(
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

// Declares that main.ino can refer to EdgeDriver's on-demand functions. It is
// referenced by the EdgeUnified::attach function.
void startGPIO(void);
void processGPIO(void);
void endGPIO(void);

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

}

using namespace EdgeGPIO;
