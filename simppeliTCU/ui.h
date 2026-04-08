#ifndef UI_H
#define UI_H

// 1. Main page (HTML UI)
String mainPage(float currentSOC, float cabinTemp, int isHeating) {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 20px; }";
  html += ".btn { padding: 15px 30px; font-size: 18px; margin: 10px; border-radius: 8px; text-decoration: none; color: white; display: inline-block; width: 80%; max-width: 300px;}";
  html += ".btn-refresh { background-color: #2196F3; }";
  html += ".btn-heat-on { background-color: #ff5722; }";
  html += ".btn-heat-off { background-color: #795548; }";
  html += ".btn-charge-on { background-color: #4CAF50; }";
  html += ".data-box { background-color: #f1f1f1; padding: 15px; margin: 15px auto; width: 80%; max-width: 300px; border-radius: 10px; box-shadow: 2px 2px 5px rgba(0,0,0,0.1); }";
  html += "</style></head><body>";
  
  html += "<h1>Leaf Et&auml;ohjaus</h1>";

  // Show read car data
  html += "<div class='data-box'>";
  html += "<h3>Auton Tila</h3>";
  
  if (currentSOC >= 0) {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Akku:</b> " + String(currentSOC, 1) + " %</p>";
  } else {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Akku:</b> Odotetaan...</p>";
  }

  if (cabinTemp > -30.0 && cabinTemp < 80.0) {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Sis&auml;l&auml;mp&ouml;:</b> " + String(cabinTemp, 1) + " &deg;C</p>";
  } else {
    html += "<p style='font-size: 20px; margin: 5px;'><b>Sis&auml;l&auml;mp&ouml;:</b> Odotetaan...</p>";
  }
  html += "</div>";

  html += "<a href='/refresh' class='btn btn-refresh'>P&auml;ivit&auml; tiedot (Her&auml;t&auml;)</a><br><hr style='width: 80%; max-width: 300px;'>";

  // Status indicators
  if (isHeating) html += "<h3 style='color: #ff5722;'>L&Auml;MMITYS P&Auml;&Auml;LL&Auml;</h3>";
  
  // Buttons
  html += "<a href='/heat_on' class='btn btn-heat-on'>K&auml;ynnist&auml; L&auml;mmitys</a><br>";
  html += "<a href='/heat_off' class='btn btn-heat-off'>Sammuta L&auml;mmitys</a><br><br>";
  html += "<a href='/charge_on' class='btn btn-charge-on'>K&auml;ynnist&auml; Lataus</a><br>";
  
  html += "</body></html>";
  return html;
}

#endif