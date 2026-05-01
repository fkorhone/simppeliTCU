#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <WebServer.h>
#include "vehicleTypes.h"

// 1. Main page (HTML UI)
void sendMainPage(WebServer& server, float currentSOC, float cabinTemp, bool isCharging, ChargerState chargerState, bool isHvacOn) {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  server.sendContent(F(
    "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>\n"
    "body { font-family: Arial; text-align: center; margin-top: 20px; }\n"
    ".btn { padding: 15px 30px; font-size: 18px; margin: 10px; border-radius: 8px; text-decoration: none; color: white; display: inline-block; width: 80%; max-width: 300px;}\n"
    ".btn-refresh { background-color: #2196F3; }\n"
    ".btn-hvac-on { background-color: #ff5722; }\n"
    ".btn-hvac-off { background-color: #795548; }\n"
    ".btn-charge-on { background-color: #4CAF50; }\n"
    ".btn-unlock { background-color: #9C27B0; }\n"
    ".btn-lock { background-color: #673AB7; }\n"
    ".data-box { background-color: #f1f1f1; padding: 15px; margin: 15px auto; width: 80%; max-width: 300px; border-radius: 10px; box-shadow: 2px 2px 5px rgba(0,0,0,0.1); }\n"
    "</style></head><body>\n"
    "<h1>Leaf Remote Control</h1>\n"
    "<div class='data-box'>\n"
    "<h3>Car Status</h3>\n"
  ));
  
  if (currentSOC >= 0) {
    server.sendContent(F("<p style='font-size: 20px; margin: 5px;'><b>Battery:</b> "));
    server.sendContent(String(currentSOC, 1));
    server.sendContent(F(" %</p>\n"));
  } else {
    server.sendContent(F("<p style='font-size: 20px; margin: 5px;'><b>Battery:</b> Waiting...</p>\n"));
  }

  if (cabinTemp > -30.0 && cabinTemp < 80.0) {
    server.sendContent(F("<p style='font-size: 20px; margin: 5px;'><b>Cabin Temp:</b> "));
    server.sendContent(String(cabinTemp, 1));
    server.sendContent(F(" &deg;C</p>\n"));
  } else {
    server.sendContent(F("<p style='font-size: 20px; margin: 5px;'><b>Cabin Temp:</b> Waiting...</p>\n"));
  }

  server.sendContent(F("<p style='font-size: 20px; margin: 5px;'><b>Charger:</b> "));
  switch (chargerState) {
    case ChargerState::CHARGING: server.sendContent(F("Charging")); break;
    case ChargerState::FINISHED: server.sendContent(F("Finished")); break;
    case ChargerState::INTERRUPTED: server.sendContent(F("Interrupted")); break;
    case ChargerState::WAITING: server.sendContent(F("Waiting")); break;
    case ChargerState::IDLE:
    default: server.sendContent(F("Idle")); break;
  }
  server.sendContent(F("</p>\n"));

  server.sendContent(F(
    "</div>\n"
    "<a href='/refresh' class='btn btn-refresh'>Update Data (Wake Up)</a><br><hr style='width: 80%; max-width: 300px;'>\n"
  ));

  // Status indicators
  if (isHvacOn) {
    server.sendContent(F("<h3 style='color: #ff5722;'>HVAC ON</h3>\n"));
  }
  if (isCharging) {
    server.sendContent(F("<h3 style='color: #4CAF50;'>CHARGING ON</h3>\n"));
  }
  
  // Buttons
  server.sendContent(F(
    "<a href='/hvac_on' class='btn btn-hvac-on'>Start HVAC</a><br>\n"
    "<a href='/hvac_off' class='btn btn-hvac-off'>Stop HVAC</a><br><br>\n"
    "<a href='/charge_on' class='btn btn-charge-on'>Start Charging</a><br><br>\n"
    "<a href='/unlock' class='btn btn-unlock'>Unlock Doors</a><br>\n"
    "<a href='/lock' class='btn btn-lock'>Lock Doors</a><br>\n"
    "</body></html>\n"
  ));

  // An empty sendContent tells the client the chunked stream has ended
  server.sendContent("");
}

#endif