/*
  This example shows how to link dynamic changes in EdgeDriver enable condition
  with changes in the corresponding AutoConnectAux context.
  Copyright (c) 2022 Hieromon Ikasamo.
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include "EdgeUnified.h"

typedef struct {
  bool  d1 = false;
  bool  d2 = false;
  bool  d3 = false;
} Drivers_t;

typedef struct {
  int stats;
} D1_t;

typedef struct {
  int stats;
} D2_t;

typedef struct {
  int stats;
} D3_t;

const char  PAGE_MENU[] PROGMEM = R"(
{
  "title": "Drivers",
  "uri": "/",
  "menu": false,
  "element": [
    {
      "name": "ls",
      "type": "ACStyle", 
      "value": ".ls{padding-top:10px;padding-bottom:15px;}"
    },
    {
      "name": "d1pre",
      "type": "ACElement",
      "value": "<div class='ls'>"
    },
    {
      "name": "d1",
      "type": "ACCheckbox",
      "label": "Driver-1",
      "checked": false,
      "posterior": "none"
    },
    {
      "name": "d1stats",
      "type": "ACSubmit",
      "value": "Stats",
      "uri": "/d1-stats"
    },
    {
      "name": "d1post",
      "type": "ACElement",
      "value": "</div><div class='ls'>"
    },
    {
      "name": "d2",
      "type": "ACCheckbox",
      "label": "Driver-2",
      "checked": false,
      "posterior": "none"
    },
    {
      "name": "d2stats",
      "type": "ACSubmit",
      "value": "Stats",
      "uri": "/d2-stats"
    },
    {
      "name": "d2post",
      "type": "ACElement",
      "value": "</div><div class='ls'>"
    },
    {
      "name": "d3",
      "type": "ACCheckbox",
      "label": "Driver-3",
      "checked": false,
      "posterior": "none"
    },
    {
      "name": "d3stats",
      "type": "ACSubmit",
      "value": "Stats",
      "uri": "/d3-stats"
    },
    {
      "name": "d3post",
      "type": "ACElement",
      "value": "</div>"
    },
    {
      "name": "set",
      "type": "ACSubmit",
      "value": "SET",
      "uri": "/"
    }
  ]
}
)";

const char STATS_D1[] PROGMEM = R"(
{
  "title": "Drivers",
  "uri": "/d1-stats",
  "menu": false,
  "element": [
    {
      "name": "d1stats",
      "type": "ACText",
      "format": "D1 Stats: %s"
    }
  ]
}
)";

const char STATS_D2[] PROGMEM = R"(
{
  "title": "Drivers",
  "uri": "/d2-stats",
  "menu": false,
  "element": [
    {
      "name": "d2stats",
      "type": "ACText",
      "format": "D2 Stats: %s"
    }
  ]
}
)";

const char STATS_D3[] PROGMEM = R"(
{
  "title": "Drivers",
  "uri": "/d3-stats",
  "menu": false,
  "element": [
    {
      "name": "d3stats",
      "type": "ACText",
      "format": "D3 Stats: %s"
    }
  ]
}
)";

void d1Start(void);
void d1Process(void);

void d2Start(void);
void d2Process(void);

void d3Start(void);
void d3Process(void);

EdgeDriver<D1_t>  d1(d1Start, d1Process, nullptr);
EdgeDriver<D2_t>  d2(d2Start, d2Process, nullptr);
EdgeDriver<D3_t>  d3(d3Start, d3Process, nullptr);

void driversStart(void);
void driversProcess(void);

EdgeDriver<Drivers_t>  drivers(driversStart, driversProcess, nullptr);

String aux_d1stats(AutoConnectAux& aux, PageArgument& args) {
  aux["d1stats"].value = String(d1.data.stats);
  return String();
}

String aux_d2stats(AutoConnectAux& aux, PageArgument& args) {
  aux["d2stats"].value = String(d2.data.stats);
  return String();
}

String aux_d3stats(AutoConnectAux& aux, PageArgument& args) {
  aux["d3tats"].value = String(d3.data.stats);
  return String();
}

void d1Start(void) {
  d1.data.stats = 0;
}

void d1Process(void) {
  d1.data.stats++;
}

void d2Start(void) {
  d2.data.stats = 0;
}

void d2Process(void) {
  d2.data.stats++;
}

void d3Start(void) {
  d3.data.stats = 0;
}

void d3Process(void) {
  d3.data.stats++;
}

void driversStart(void) {
  Edge.attach(d1, 1000);
  Edge.attach(d2, 1000);
  Edge.attach(d3, 1000);
}

AutoConnect portal;
AutoConnectConfig config;

String aux_drivers(AutoConnectAux& aux, PageArgument& args) {
  String  referer = String(portal.where());
  if (!referer.length())
    referer = "/";
  AutoConnectAux& request = *(portal.aux(referer));

  drivers.data.d1 = request["d1"].as<AutoConnectCheckbox>().checked;
  request["d1stats"].as<AutoConnectSubmit>().enable = drivers.data.d1;

  drivers.data.d2 = request["d2"].as<AutoConnectCheckbox>().checked;
  request["d2stats"].as<AutoConnectSubmit>().enable = drivers.data.d2;

  drivers.data.d3 = request["d3"].as<AutoConnectCheckbox>().checked;
  request["d3stats"].as<AutoConnectSubmit>().enable = drivers.data.d3;

  return String();
}

void driversProcess(void) {
  d1.enable(drivers.data.d1);
  if (drivers.data.d1) {
    if (!portal.aux("/d1-stats")) {
      Edge.join(STATS_D1, aux_d1stats);
      Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
    }
  }
  else if (portal.aux("/d1-stats")) {
    Edge.release("/d1-stats");
    Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
  }

  d2.enable(drivers.data.d2);
  if (drivers.data.d2) {
    if (!portal.aux("/d2-stats")) {
      Edge.join(STATS_D2, aux_d2stats);
      Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
    }
  }
  else if (portal.aux("/d2-stats")) {
    Edge.release("/d2-stats");
    Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
  }

  d3.enable(drivers.data.d3);
  if (drivers.data.d3) {
    if (!portal.aux("/d3-stats")) {
      Edge.join(STATS_D3, aux_d1stats);
      Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
    }
  }
  else if (portal.aux("/d3-stats")) {
    Edge.release("/d3-stats");
    Serial.printf("Free heap: %" PRIu32 "\n", ESP.getFreeHeap());
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  config.autoReconnect = true;
  config.portalTimeout = 1;
  config.reconnectInterval = 1;
  portal.config(config);
  portal.begin();

  Edge.attach(drivers);
  Edge.join(PAGE_MENU, aux_drivers);
}

void loop() {
  Edge.process(portal);

  portal.handleClient();
}
