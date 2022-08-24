/*
  EgeDriver handling MQTT as an example of combining to EdgeUnified.
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
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <HTTPClient.h>
#endif
#include <ArduinoJson.h>
#include <AutoConnect.h>
#include <PubSubClient.h>
#include "EdgeUnified.h"
#include "MyMqtt.h"

/**
 * MQTT custom Web page descriptions.
 */
const char SETTINGS_MQTT[] PROGMEM = R"*(
{
  "title": "MQTT Setting",
  "uri": "/mqtt_setting",
  "menu": true,
  "element": [
    {
      "name": "style",
      "type": "ACStyle",
      "value": "label+input,label+select{position:sticky;left:140px;width:204px!important;box-sizing:border-box;}"
    },
    {
      "name": "header",
      "type": "ACElement",
      "value": "<h2 style='text-align:center;color:#2f4f4f;margin-top:10px;margin-bottom:10px'>MQTT Broker settings</h2>"
    },
    {
      "name": "caption",
      "type": "ACText",
      "value": "Publish WiFi signal strength via MQTT, publishing the RSSI value of the ESP module to the ThingSpeak public channel.",
      "style": "font-family:serif;color:#053d76",
      "posterior": "par"
    },
    {
      "name": "mqttserver",
      "type": "ACInput",
      "label": "Server",
      "pattern": "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$",
      "placeholder": "MQTT broker server",
      "global": true
    },
    {
      "name": "apikey",
      "type": "ACInput",
      "label": "User API Key",
      "global": true
    },
    {
      "name": "channelid",
      "type": "ACInput",
      "label": "Channel ID",
      "pattern": "^[0-9]{6}$",
      "global": true
    },
    {
      "name": "writekey",
      "type": "ACInput",
      "label": "Write API Key",
      "global": true
    },
    {
      "name": "nl1",
      "type": "ACElement",
      "value": "<hr>"
    },
    {
      "name": "credential",
      "type": "ACText",
      "value": "MQTT Device Credentials",
      "style": "font-weight:bold;color:#1e81b0",
      "posterior": "div"
    },
    {
      "name": "clientid",
      "type": "ACInput",
      "label": "Client ID",
      "global": true
    },
    {
      "name": "username",
      "type": "ACInput",
      "label": "Username",
      "global": true
    },
    {
      "name": "password",
      "type": "ACInput",
      "label": "Password",
      "apply": "password",
      "global": true
    },
    {
      "name": "nl2",
      "type": "ACElement",
      "value": "<hr>"
    },
    {
      "name": "period",
      "type": "ACRadio",
      "value": [
        "30 sec.",
        "60 sec.",
        "180 sec."
      ],
      "label": "Update period",
      "arrange": "vertical",
      "global": true
    },
    {
      "name": "uniqueid",
      "type": "ACCheckbox",
      "value": "unique",
      "label": "Use APID unique",
      "checked": false
    },
    {
      "name": "hostname",
      "type": "ACInput",
      "value": "",
      "label": "ESP host name",
      "pattern": "^([a-zA-Z0-9]([a-zA-Z0-9-])*[a-zA-Z0-9]){1,24}$",
      "global": true
    },
    {
      "name": "save",
      "type": "ACSubmit",
      "value": "Save&amp;Start",
      "uri": "/mqtt_start"
    },
    {
      "name": "discard",
      "type": "ACSubmit",
      "value": "Discard",
      "uri": "/"
    },
    {
      "name": "stop",
      "type": "ACSubmit",
      "value": "Stop publishing",
      "uri": "/mqtt_stop"
    }
  ]
}
)*";

const char START_MQTT[] PROGMEM = R"*(
{
  "title": "MQTT Setting",
  "uri": "/mqtt_start",
  "menu": false,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "<h4>Parameters saved as:</h4>",
      "style": "text-align:center;color:#2f4f4f;padding:5px;"
    },
    {
      "name": "mqttserver",
      "type": "ACText",
      "format": "Server: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "apikey",
      "type": "ACText",
      "format": "User API Key: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "channelid",
      "type": "ACText",
      "format": "Channel ID: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "writekey",
      "type": "ACText",
      "format": "Write API Key: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "clientid",
      "type": "ACText",
      "format": "Client ID: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "username",
      "type": "ACText",
      "format": "Username: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "password",
      "type": "ACText",
      "format": "Password: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "hostname",
      "type": "ACText",
      "format": "ESP host: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "period",
      "type": "ACText",
      "format": "Update period: %s",
      "posterior": "br",
      "global": true
    },
    {
      "name": "clear",
      "type": "ACSubmit",
      "value": "Clear channel",
      "uri": "/mqtt_clear"
    }
  ]
}
)*";

const char CLEAR_MQTT[] PROGMEM = R"*(
{
  "title": "MQTT Setting",
  "uri": "/mqtt_clear",
  "menu": false,
  "response": false
}
)*";

const char STOP_MQTT[] PROGMEM = R"*(
{
  "title": "MQTT Setting",
  "uri": "/mqtt_stop",
  "menu": false,
  "response": false
}
)*";

/**
 * Edge entities
 */
EdgeDriver<MQTT_t>  mqtt(startMQTT, processMQTT, endMQTT);

/**
 * Instance responsible for implementation of various IOs dependent on EdgeDriver.
 */
extern WebServer  server;
extern AutoConnect portal;
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

/**
 * AutoConnectAux custom web page request handlers.
 */
// AutoConnectAux handler. Transfer EdgeData to AutoConnectElement values
String auxMQTTSetting(AutoConnectAux& aux, PageArgument& args) {
  // The behavior of the auxMQTTSetting function below transfers the values
  // stored in mqtt EdgeData (they are the connection settings to the MQTT
  // broker) to the value of each AutoConnectInput element on the custom web
  // page. (i.e., displayed as preset values)
  aux["mqttserver"].as<AutoConnectInput>().value = mqtt.data.server;
  aux["apikey"].as<AutoConnectInput>().value = mqtt.data.apikey;
  aux["channelid"].as<AutoConnectInput>().value = mqtt.data.channelid;
  aux["writekey"].as<AutoConnectInput>().value = mqtt.data.writekey;
  aux["clientid"].as<AutoConnectInput>().value = mqtt.data.clientid;
  aux["username"].as<AutoConnectInput>().value = mqtt.data.username;
  aux["password"].as<AutoConnectInput>().value = mqtt.data.password;
  aux["hostname"].as<AutoConnectInput>().value = mqtt.data.hostname;
  aux["period"].as<AutoConnectRadio>().checked = mqtt.data.publishInterval / (30 * 1000);
  return String();
}

// AutoConnectAux handler. Obtain AutoConnectElement values, copy to EdgeData
String auxMQTTStart(AutoConnectAux& aux, PageArgument& args) {
  // Get the connection settings entered in SETTINGS_MQTT via START_MQTT and
  // set them in mqtt EdgeData. Each AutoConnectElements with the same name in
  // SETTINGS_MQTT and START_MQTT has a global attribute so that input values
  // to SETTINGS_MQTT can be obtained from START_MQTT.
  mqtt.data.server = aux["mqttserver"].as<AutoConnectText>().value;
  mqtt.data.apikey = aux["apikey"].as<AutoConnectText>().value;
  mqtt.data.channelid = aux["channelid"].as<AutoConnectText>().value;
  mqtt.data.writekey = aux["writekey"].as<AutoConnectText>().value;
  mqtt.data.clientid = aux["clientid"].as<AutoConnectText>().value;
  mqtt.data.username = aux["username"].as<AutoConnectText>().value;
  mqtt.data.password = aux["password"].as<AutoConnectText>().value;
  mqtt.data.hostname = aux["hostname"].as<AutoConnectText>().value;

  // If the AutoConnectElement of the input side is AutoConnectRadio or
  // AutoConnectSelect, the selected value cannot be taken directly into the
  // value member of AutoConnectText. Therefore, the value cannot be shared
  // directly with AutoConnectText, which has a global attribute with the same
  // element name.
  // In this case, we get the AutoConnectAux of the requester and try to get
  // the value directly from the selector's element. To obtain the requester's
  // AutoConnectAux, use the AutoConnect::aux and AutoConnect::where function
  // in combination as follows:
  const AutoConnectAux* requestAux = portal.aux(portal.where());
  if (requestAux) {
    int checkedPeriod = ((AutoConnectAux&)(*requestAux))["period"].as<AutoConnectRadio>().checked;
    unsigned long interval = 0;
    if (checkedPeriod == 1)
      interval = 30;
    else if (checkedPeriod == 2)
      interval = 60;
    else if (checkedPeriod == 3)
      interval = 180;
    mqtt.data.publishInterval = interval * 1000;
  }

  // You make EdgeData persistent when the value it has is changed.
  mqtt.save();

  // Restart mqtt EdgeDriver due to a change in the host address of the MQTT
  // broker.
  // In this example, startMQTT (EdgeDriver's on-start function) uses the
  // PubSubClient::setServer function to set up the MQTT broker; it uses
  // EdgeDriver's start function without going through EdgeUnified to call the
  // startMQTT function.
  // Centralize the logic and maintain the independence of each EdgeDevice by
  // consolidating the processing related to devise startup in the EdgeDriver's
  // on-start function.
  mqtt.start();

  return String();
}

// AutoConnectAux handler. Obtain AutoConnectElement values, copy to EdgeData
String auxMQTTStop(AutoConnectAux& aux, PageArgument& args) {
  mqtt.end();
  aux.redirect("/");
  return String();
}

// AutoConnectAux handler. Obtain AutoConnectElement values, copy to EdgeData
String auxMQTTClear(AutoConnectAux& aux, PageArgument& args) {
  HTTPClient  httpClient;

  String  endpoint = mqtt.data.server;
  endpoint.replace("mqtt3", "api");
  String  delUrl = "http://" + endpoint + "/channels/" + mqtt.data.channelid + "/feeds.json?api_key=" + mqtt.data.apikey;

  Serial.print("DELETE " + delUrl);
  if (httpClient.begin(wifiClient, delUrl)) {
    Serial.print(":");
    int resCode = httpClient.sendRequest("DELETE");
    const String& res = httpClient.getString();
    Serial.println(String(resCode) + String(",") + res);
    httpClient.end();
  }
  else
    Serial.println(" failed");

  aux.redirect("/");
  return String();
}

int getStrength(uint8_t points) {
  uint8_t sc = points;
  long    rssi = 0;

  while (sc--) {
    rssi += WiFi.RSSI();
    delay(20);
  }
  return points ? static_cast<int>(rssi / points) : 0;
}

/**
 * MQTT start callback
 */
void startMQTT() {
  Serial.println("Starting MQTT");
  mqtt.data.inPublish = false;
  mqtt.data.retryInterval = 5000;
  mqttClient.setServer(mqtt.data.server.c_str(), 1883);
  if (mqtt.data.hostname.length()) {
    if (!mqtt.data.hostname.equalsIgnoreCase(String(WiFi.getHostname()))) {
      WiFi.setHostname(mqtt.data.hostname.c_str());
      startMDNS();
    }
  }
}

/**
 * MQTT process callback
 * By checking the connection with the broker at the time of the publish
 * request, it can delegate the reconnection attempt to the loop function.
 * This strategy eliminates handling the delay that occurs during the
 * broker reconnection attempt loop and smooth AutoConnect communication
 * with the client.
 * 
 * Note that the following the processMQTT callback function do not have their
 * own loop logic. It has an error retry structure when MQTT issuance fails,
 * and even the retry processing does not form a loop within the processMQTT.
 * 
 * In the Arduino framework, the loop() function itself is the actual event
 * loop. Forming a small independent loop inside the loop function is not
 * appropriate as a programming structure because it interferes with other
 * processing.
 * 
 */
void processMQTT() {
  if (mqtt.data.server.length()) {
    if (millis() > mqtt.data.nextPeriod) {
      // Attempts to connect to the MQTT broker based on a valid server name.
      // mqttClient.setServer(mqtt.data.server.c_str(), 1883);
      if (!mqttClient.connected()) {
        Serial.println(String("Attempting MQTT broker:") + mqtt.data.server);
        if ((mqtt.data.inPublish = mqttClient.connect(mqtt.data.clientid.c_str(), mqtt.data.username.c_str(), mqtt.data.password.c_str())))
          Serial.println("Established:" + mqtt.data.clientid);
        else
          Serial.print("Connection failed:" + String(mqttClient.state()));
      }

      if (mqtt.data.inPublish) {
        String  topic = String("channels/") + mqtt.data.channelid + String("/publish");
        String  message = String("field1=") + String(getStrength(7));
        mqttClient.publish(topic.c_str(), message.c_str());
        mqtt.data.inPublish = mqttClient.loop();
        if (!mqtt.data.inPublish)
          Serial.print("MQTT publishing failed");
      }

      if (mqtt.data.inPublish) {
        mqttClient.disconnect();
        mqtt.data.nextPeriod = millis() + mqtt.data.publishInterval;
        mqtt.data.retry = 0;
      }
      else {
        // Error retry. By varying the interval until the next turn of the
        // process called, the processMQTT performs an error retry without an
        // internal loop.
        if (mqtt.data.retry++ < 3) {
          mqtt.data.nextPeriod = millis() + mqtt.data.retryInterval;
          Serial.printf("...retrying %d\n", mqtt.data.retry);
        }
        else {
          mqtt.data.nextPeriod = millis() + mqtt.data.publishInterval;
          mqtt.data.retry = 0;
          Serial.println(", retries exceeded, abandoned.");
        }
      }
    }
  }
}

/**
 * MQTT end callback
 */
void endMQTT() {
  mqttClient.disconnect();
  mqtt.data.inPublish = false;
  Serial.println("MQTT publishing stopped\n");
}

/**
 * EdgeData serializer
 * If EdgeData contains class objects, EdgeUnified converts to JSON and
 * exports it to the file system. Import from the file system also assumes
 * that EdgeData is in JSON format. The serializer and deserializer support
 * inter-conversion between JSON objects and EdgeData instances.
 */
// Serializer: MQTT_t EdgeData. to MQTT EdgeData JSON object. 
void mqttSerialize(JsonObject& edgeData) {
  edgeData[F("server")] = mqtt.data.server;
  edgeData[F("apikey")] = mqtt.data.apikey;
  edgeData[F("channelid")] = mqtt.data.channelid;
  edgeData[F("writekey")] = mqtt.data.writekey;
  edgeData[F("clientid")] = mqtt.data.clientid;
  edgeData[F("username")] = mqtt.data.username;
  edgeData[F("password")] = mqtt.data.password;
  edgeData[F("hostname")] = mqtt.data.hostname;
  edgeData[F("publishInterval")] = mqtt.data.publishInterval;
}

// Deserializer: MQTT EdgeData JSON object to MQTT_t EdgeData.
void mqttDeserialize(JsonObject& edgeData) {
  mqtt.data.server = edgeData[F("server")].as<String>();
  mqtt.data.apikey = edgeData[F("apikey")].as<String>();
  mqtt.data.channelid = edgeData[F("channelid")].as<String>();
  mqtt.data.writekey = edgeData[F("writekey")].as<String>();
  mqtt.data.clientid = edgeData[F("clientid")].as<String>();
  mqtt.data.username = edgeData[F("username")].as<String>();
  mqtt.data.password = edgeData[F("password")].as<String>();
  mqtt.data.hostname = edgeData[F("hostname")].as<String>();
  mqtt.data.publishInterval = edgeData[F("publishInterval")].as<unsigned long>();
}
