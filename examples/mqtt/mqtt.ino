/*
  ESP8266/ESP32 publish the RSSI as the WiFi signal strength to ThingSpeak channel.
  This example is for explaining how to use the EdgeUnified library.
  Copyright (c) 2022 Hieromon Ikasamo.
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define mDNSUpdate()  do {MDNS.update();} while (0)
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#define mDNSUpdate() do {(void)0;} while(0)
#endif
#include <AutoConnect.h>

// Include the EdgeUnified header.
#include "EdgeUnified.h"

// Include the module header for the EdgeDriver to be used.
#include "MyMqtt.h"
#include "MyGpio.h"

/**
 * Declare required instances.
 */
WebServer   server;
AutoConnect portal(server);
AutoConnectConfig config;

/**
 * Write a callback for AutoConnect like onConnect handler. You don't have to
 * include it in the main module, but I put it below for clarity in this example.
 */
void handleRoot() {
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "</head>"
    "<body>"
    "<iframe width=\"450\" height=\"260\" style=\"transform:scale(0.79);-o-transform:scale(0.79);-webkit-transform:scale(0.79);-moz-transform:scale(0.79);-ms-transform:scale(0.79);transform-origin:0 0;-o-transform-origin:0 0;-webkit-transform-origin:0 0;-moz-transform-origin:0 0;-ms-transform-origin:0 0;border: 1px solid #cccccc;\" src=\"https://thingspeak.com/channels/{{CHANNEL}}/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&type=line\"></iframe>"
    "<p style=\"padding-top:5px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
    "</body>"
    "</html>";

  content.replace("{{CHANNEL}}", mqtt.data.channelid);
  server.send(200, "text/html", content);
}

void startMDNS(void) {
  if (MDNS.queryService("http", "tcp"))
    MDNS.end();
  
  const char* hostname = WiFi.getHostname();
  Serial.printf("mDNS responder %s.local start", hostname);
  if (MDNS.begin(hostname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("ed");
  }
  else {
    Serial.println(" failed");
  }
}

void onWiFiConnect(IPAddress& ip) {
  Serial.println("connected:" + WiFi.SSID());
  Serial.println("IP:" + WiFi.localIP().toString());
  startMDNS();
}

/**
 * Example of main sketch forming an event loop with EdgeUnified.
 */
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("Start");

  /*
    The basic steps to combine multiple device IOs into one loop using Edge
    are as follows:

    1. Perform necessary pretreatment.

    This example enables AutoConnect configuration settings, and also starts
    the file system, and enables persistence such as EdgeData.

    2. Enable EdgeData automatic recovery by EdgeDriver itself if necessary.

    Enabling the autoRestore in EdgeDriver will load previously saved EdgeData
    from the file system and restore it as an EdgeData instance.
    If EdgeData is harboring a class object such as a String, you will need to
    provide a serializer and deserializer to assist in EdgeData persistence.
    If EdgeData contains class objects, EdgeUnified converts to JSON and
    exports it to the file system. Import from the file system also assumes
    that EdgeData is in JSON format. The serializer and deserializer support
    inter-conversion between JSON objects and EdgeData instances. Examples of
    a serializer and a deserializer can be found in the mqttSerialize and
    mqttDeserialize functions of the MyMqtt.cpp module.

    3. Attach EdgeDrivers to EdgeUnified.

    Attaches the EdgeDriver to the EdgeUnified. Once attached, the EdgeDriver's
    onStart callback is invoked. if autoRestore is enabled, EdgeData is loaded
    from the file system.

    4. Join AutoConnectAux to EdgeUnified.

    Pair the JSON description of the AutoConnectAux custom web page with the
    request handler and bind it to EdgeUnified. If EdgeUnified does not own the
    AutoConnect instance at the time of binding (i.e. AutoConnect::begin has not
    run), EdgeUnified will delay loading the AutoConnectAux JSON.

    5. Start AutoConnect

    As AutoConnect::begin().
  */

  config.portalTimeout = 1000;
  config.autoReconnect = true;
  config.reconnectInterval = 1;
  config.retainPortal = true;
  portal.config(config);
  portal.onConnect(onWiFiConnect);

#if defined(ARDUINO_ARCH_ESP8266)
  LittleFS.begin();
#elif defined(ARDUINO_ARCH_ESP32)
  SPIFFS.begin(true);
#endif

  gpio.autoRestore(true);
  mqtt.autoRestore(true);
  mqtt.serializer(mqttSerialize, mqttDeserialize, 512);

  /*
    To make the EdgeDriver a member of the event loop by EdgeUnified, register
    the EdgeDriver using the EdgeUnified::attach function.
    Specify the EdgeDriver as an argument to EdgeUnified::attach. The following
    two examples register gpio and mqtt EdgeDriver separately.
   */
  Edge.attach(gpio);
  Edge.attach(mqtt);
  // Multiple EdgeDrivers can be registered at one time. In this case, multiple
  // EdgeDrivers are specified by enclosing them with '{' and '}'.
  // Edge.attach({ gpio, mqtt });

  /*
    To make the EdgeDriver a member of the event loop by EdgeUnified, register
    the EdgeDriver using the EdgeUnified::attach function.
    Specify the EdgeDriver as an argument to EdgeUnified::attach. The following
    two examples register gpio and mqtt EdgeDriver separately.
   */
  Edge.join(FPSTR(SETTINGS_GPIO), auxGPIOSetting);
  Edge.join(FPSTR(BLINKING_GPIO), auxGPIOApply);
  // The above two lines can also be written on a single function call as follows:
  // Edge.join({
  //   {FPSTR(SETTINGS_GPIO), auxGPIOSetting},
  //   {FPSTR(BLINKING_GPIO), auxGPIOApply}
  // });

  Edge.join({
    {FPSTR(SETTINGS_MQTT), auxMQTTSetting},
    {FPSTR(START_MQTT), auxMQTTStart},
    {FPSTR(CLEAR_MQTT), auxMQTTClear},
    {FPSTR(STOP_MQTT), auxMQTTStop}
  });

  // Restoring from the EdgeData file system can optionally be done with
  // EdgeUnified::restore function by Sketch in addition to automatic
  // restoration by EdgeDriver as follows.
  // Edge.restore();

  // If a sketch has its own Web page (which is usually bound directly to the
  // WebServer class), bind it.
  server.on("/", handleRoot);

  // If you want to finish loading all AutoConnectAuxes in the setup function,
  // use the EdgeUnified::portal function; there is no need to use the
  // AutoConnect::load function separately.
  // Edge.portal(portal);

  portal.begin();
}

void loop() {
  /*
    EdgeUnified allows you to squeeze multiple EdgeDrivers into a single event
    loop. On the other hand, each EdgeDriver is independent and unaware of the
    state of other EdgeDrivers. In other words, the process functions of
    EdgeDrivers are asynchronous between EdgeDrivers.
    If you need to use the status of the MQTT EdgeDriver to control the
    behavior of the GPIO EdgeDriver, as in the example below, it is recommended
    to do so in a loop function. By granting global access only to the loop()
    function, you can make the EdgeDriver more independent and available for
    other scenes.
  */

  // The following example procedure represents a cross-reference of EdgeData
  // between EdgeDrivers. It will continue to flash the LED only if publishing
  // to MQTT is successful. Also, to stop blinking the LED when an MQTT message
  // cannot be published, the gpio.enable function is used to control the
  // blinking state with the value obtained from the mqtt data member held by
  // EdgeData contained in EdgeDriver.
  // The MQTT publish state is maintained by EdgeDriver mqtt as the inPublish.
  gpio.enable(mqtt.data.inPublish);

  // Consecutively calls the process function of the EdgeDrivers bound to the
  // EdgeUnifined to execute an event loop.
  // EdgeUnified::process function with the portal argument allows EdgeUnified
  // to dynamically load and bind AutoConnectAux. Sketches can release and join
  // AutoConnectAuxes during EdgeUnified's event loop using the process function
  // with a portal argument.
  Edge.process(portal);

  // The post process of the loop function calls other service instances that
  // need to be invoked as event loops.
  mDNSUpdate();
  portal.handleClient();
}
