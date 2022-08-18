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

#ifndef _MYMQTT_H_
#define _MYMQTT_H_

#include "EdgeUnified.h"

// Define EdgeData structure for MQTT.
// This is the data structure handled by MQTT EdgeDriver.
typedef struct {
  String  server;
  String  apikey;
  String  channelid;
  String  writekey;
  String  clientid;
  String  username;
  String  password;
  String  hostname;
  unsigned long publishInterval;
  unsigned long retryInterval;
  unsigned long nextPeriod;
  int   retry;
  bool  inPublish;
} MQTT_t;

// External linkage of EdgeDriver MQTT instance.
extern EdgeDriver<MQTT_t>  mqtt;

// Declares that main.ino can refer to EdgeDriver's on-demand functions. It is
// referenced by the EdgeUnified::attach function.
void startMQTT(void);
void processMQTT(void);
void endMQTT(void);

// When persisting EdgeData containing class objects, a serializer and a
// deserializer are required.
void mqttSerialize(JsonObject& json);
void mqttDeserialize(JsonObject& json);

// External linkage of AutoConnectAux JSON definitions.
// These external declarations are referenced by EdgeUnified:join function.
extern const char SETTINGS_MQTT[] PROGMEM;
extern const char START_MQTT[] PROGMEM;
extern const char CLEAR_MQTT[] PROGMEM;
extern const char STOP_MQTT[] PROGMEM;

// AutoConnectAux request handlers
String auxMQTTSetting(AutoConnectAux&, PageArgument&);
String auxMQTTStart(AutoConnectAux&, PageArgument&);
String auxMQTTStop(AutoConnectAux&, PageArgument&);
String auxMQTTClear(AutoConnectAux&, PageArgument&);

#endif // !_MYMQTT_H_
