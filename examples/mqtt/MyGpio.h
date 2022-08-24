/*
  EgeDriver handling GPIO as an example of combining to EdgeUnified.
  This file shows the basic structure of the EdgeDriver header file.
  EdgeDriver requires the following symbols to be placed in the global scope:
  1. A struct of EdgeData.
  2. An instance of EdgeDriver.
  3. EdgeDriver on-demand functions.
  4. AutoConnectAux JSON descriptions.
  5. AutoConnectAux request handlers.
  Copyright (c) 2022 Hieromon Ikasamo.
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#ifndef _MYGPIO_H_
#define _MYGPIO_H_

#include "EdgeUnified.h"

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

// The data members that this structure contains are globally accessible via
// instances of EdgeDriver like as follows:
//    EdgeDriver<GPIO_t>  gpio;
//    gpio.data.pin = 1;
//    unsigned long blink = gpio.data.cycle;
extern EdgeDriver<GPIO_t>  gpio;

// Declares that main.ino can refer to EdgeDriver's on-demand functions. It is
// referenced by the EdgeUnified::attach function.
void startGPIO(void);
void processGPIO(void);
void endGPIO(void);

// External linkage of AutoConnectAux JSON definitions.
// These external declarations are referenced by EdgeUnified:join function.
extern const char SETTINGS_GPIO[] PROGMEM;
extern const char BLINKING_GPIO[] PROGMEM;

// AutoConnectAux request handlers
String auxGPIOApply(AutoConnectAux&, PageArgument&);
String auxGPIOSetting(AutoConnectAux&, PageArgument&);

#if defined(ARDUINO_ARCH_ESP8266)
const uint8_t LED_ACTIVE = LOW;
#elif defined(ARDUINO_ARCH_ESP32)
const uint8_t LED_ACTIVE = HIGH;
#endif

#endif // !_MYGPIO_H_

