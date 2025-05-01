#include <WiFi.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ==== PIN DEFINITIONS ====
#define TFT_CS     17
#define TFT_RST    5
#define TFT_DC     16
#define TFT_MOSI   23
#define TFT_CLK    18
#define TFT_MISO   19
#define SDA_PIN    21
#define SCL_PIN    22

// ==== DISPLAY ====
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ==== SENSOR ====
Adafruit_BME680 bme;
unsigned long lastLogTime = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long LOG_INTERVAL = 15 * 60 * 1000;
const unsigned long DISPLAY_UPDATE_INTERVAL = 60 * 1000;
String currentData;

// ==== WIFI ====
const char* ssid = "ADD YOUR WIFI NETWORK HERE";
const char* password = "ADD YOUR WIFI PASSWORD HERE";
const long gmtOffset_sec = -7 * 3600;
const int daylightOffset_sec = 0;

// ==== SERVER ====
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_CLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  // 🔧 Glitch Splash for Chr0nicHacker
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_MAGENTA);
  int x = 30, y = 100;

// Initial draw
  tft.setCursor(x, y);
  tft.print("Chr0nicHacker");

// Glitch animation
  for (int i = 0; i < 15; i++) {
  // Random white glitch lines
    int rx = random(x, x + 200);
    int ry = random(y - 10, y + 40);
    int rw = random(10, 60);
    int rh = 2;
    tft.fillRect(rx, ry, rw, rh, ILI9341_WHITE);
    delay(30);
    tft.fillRect(rx, ry, rw, rh, ILI9341_BLACK);

  // Random pixel flicker
    int px = random(x, x + 200);
    int py = random(y - 10, y + 30);
    tft.drawPixel(px, py, ILI9341_MAGENTA);
    delay(10);
}

// Final clean draw
  tft.setTextColor(ILI9341_MAGENTA);
  tft.setCursor(x, y);
  tft.print("Chr0nicHacker");

  delay(2000);  // Let it sit before booting


  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bme.begin()) {
    Serial.println("❌ BME680 not found");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setGasHeater(320, 150);

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Connected");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");

  Serial.print("Waiting for NTP time");
  while (time(nullptr) < 10000) {
    delay(500); Serial.print(".");
  }
  Serial.println("⏰ Time synced");

  setupWebServer();
}

void loop() {
  unsigned long now = millis();

  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL || lastDisplayUpdate == 0) {
    readSensor();
    ws.textAll(currentData);
    lastDisplayUpdate = now;
  }

  if (now - lastLogTime >= LOG_INTERVAL || lastLogTime == 0) {
    trimLogFile(100 * 1024);
    logSensorData();
    lastLogTime = now;
  }
}
void readSensor() {
  Serial.println("📡 Reading sensor...");
  if (!bme.performReading()) {
    Serial.println("⚠️ Sensor read failed");
    currentData = "Sensor error";
    return;
  }

  float tempF = bme.temperature * 9.0 / 5.0 + 32.0;
  float humidity = bme.humidity;
  float pressure = bme.pressure / 100.0;
  float gasKOhms = bme.gas_resistance / 1000.0;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Time not synced, skipping display update.");
    return;
  }
  char timeBuf[16], dateBuf[16];
  strftime(timeBuf, sizeof(timeBuf), "%I:%M %p", &timeinfo);
  strftime(dateBuf, sizeof(dateBuf), "%m/%d/%Y", &timeinfo);

  String timeOnly = String(timeBuf);
  String dateOnly = String(dateBuf);

  // Greeting string (newly added)
  String greeting;
  int hour = timeinfo.tm_hour;
  if (hour >= 4 && hour < 12) greeting = "Good Morning";
  else if (hour >= 12 && hour < 16) greeting = "Good Afternoon";
  else if (hour >= 16 && hour < 21) greeting = "Good Evening";
  else if (hour >= 21 || hour < 2) greeting = "Good Night";
  else greeting = "Go To Sleep, Chr0n1c H@ck3r";

  currentData = "<div class='reading'>" + timeOnly + "</div>";
  currentData += "<div class='reading'>" + dateOnly + "</div>";
  currentData += "<div class='reading'>🌡️ Temp: " + String(tempF, 2) + " F</div>";
  currentData += "<div class='reading'>💧 Humidity: " + String(humidity, 2) + "%</div>";
  currentData += "<div class='reading'>🌪️ Air Pressure: " + String(pressure, 2) + " hPa</div>";
  currentData += "<div class='reading'>🫁 Air Quality: " + String(gasKOhms, 2) + " KOhms</div>";

  tft.fillScreen(ILI9341_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(5, 5);
  tft.print(timeOnly);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(dateOnly, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(320 - w - 5, 5);
  tft.print(dateOnly);

  // Greeting (centered)
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.getTextBounds(greeting, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((320 - w) / 2, 50);
  tft.print(greeting);
  Serial.println("✅ Greeting shown");

  // Temperature color + label
  uint16_t tempColor = ILI9341_WHITE;
  String tempLabel = "Average";
  if (tempF >= 90)       { tempColor = ILI9341_RED;    tempLabel = "Hot"; }
  else if (tempF >= 80)  { tempColor = ILI9341_ORANGE; tempLabel = "Warm"; }
  else if (tempF >= 61)  { tempColor = ILI9341_WHITE;  tempLabel = "Average"; }
  else if (tempF >= 41)  { tempColor = ILI9341_CYAN;   tempLabel = "Cold"; }
  else                   { tempColor = ILI9341_BLUE;   tempLabel = "Freezing"; }

  tft.setTextSize(4);
  tft.setTextColor(tempColor);
  String tempStr = String(tempF, 1) + " F";
  tft.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((320 - w) / 2, 90);
  tft.print(tempStr);
  Serial.println("✅ Temp shown");

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.getTextBounds(tempLabel, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((320 - w) / 2, 140);
  tft.print(tempLabel);
  Serial.println("✅ Label shown");

  // Metric colors
  uint16_t humidityColor = ILI9341_WHITE;
  if (humidity < 20 || humidity > 70) humidityColor = ILI9341_RED;
  else if (humidity < 30 || humidity > 60) humidityColor = ILI9341_ORANGE;

  uint16_t pressureColor = ILI9341_WHITE;
  if (pressure < 980 || pressure > 1040) pressureColor = ILI9341_RED;
  else if ((pressure >= 980 && pressure < 1000) || (pressure > 1025 && pressure <= 1040)) pressureColor = ILI9341_ORANGE;

  uint16_t gasColor = ILI9341_WHITE;
  if (gasKOhms < 5) gasColor = ILI9341_RED;
  else if (gasKOhms < 10) gasColor = ILI9341_ORANGE;

  tft.setTextSize(1);
  int leftX = 10;
  int baseY = 190;
  tft.setTextColor(humidityColor); tft.setCursor(leftX, baseY);       tft.printf("Humidity: %.1f %%", humidity);
  tft.setTextColor(pressureColor); tft.setCursor(leftX, baseY + 15);  tft.printf("Air Pressure: %.1f hPa", pressure);
  tft.setTextColor(gasColor);      tft.setCursor(leftX, baseY + 30);  tft.printf("Air Quality: %.2f KOhms", gasKOhms);

  if (gasColor == ILI9341_RED) {
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(leftX, baseY + 50);
    tft.print("⚠️ AIR QUALITY: OPEN WINDOW");
  }
}
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "TimeError";
  char buf[32];
  strftime(buf, sizeof(buf), "%I:%M:%S %p %m/%d/%Y", &timeinfo);
  return String(buf);
}

void logSensorData() {
  float tempF = bme.temperature * 9.0 / 5.0 + 32.0;
  float humidity = bme.humidity;
  float pressure = bme.pressure / 100.0;
  float gasKOhms = bme.gas_resistance / 1000.0;

  String timestamp = getFormattedTime();
  String logEntry = "[" + timestamp + "] Temp: " + String(tempF, 2) + " F, "
                  + "Humidity: " + String(humidity, 2) + "%, "
                  + "Pressure: " + String(pressure, 2) + "hPa, "
                  + "Gas: " + String(gasKOhms, 2) + "KOhms\n";

  File file = SPIFFS.open("/weatherlog.txt", FILE_APPEND);
  if (file) {
    file.print(logEntry);
    file.close();
    Serial.println("📝 Logged: " + logEntry);
  } else {
    Serial.println("❌ Log write failed");
  }
}

void trimLogFile(size_t maxSize) {
  File file = SPIFFS.open("/weatherlog.txt", FILE_READ);
  if (!file || file.size() <= maxSize) {
    file.close(); return;
  }

  String lines = "";
  while (file.available()) lines += file.readStringUntil('\n') + "\n";
  file.close();

  int keepLines = 100;
  int lineCount = 0;
  int startIndex = lines.length();
  while (startIndex > 0 && lineCount < keepLines)
    startIndex = lines.lastIndexOf('\n', startIndex - 1), lineCount++;

  String trimmed = lines.substring(startIndex + 1);
  File trimmedFile = SPIFFS.open("/weatherlog.txt", FILE_WRITE);
  trimmedFile.print(trimmed);
  trimmedFile.close();
}

void setupWebServer() {
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("🔌 WebSocket client connected");
    }
  });
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Indoor Weather</title>";
    html += "<style>body{background:#111;color:#0f0;font-family:'Segoe UI',sans-serif;max-width:600px;margin:auto;padding:20px;}h2{font-size:1.5rem;color:#6aff6a}.reading{margin:0.5em 0;font-size:1.2rem}a{color:#0f0}button{padding:10px 20px;background:#0f0;color:#000;border:none;cursor:pointer;width:100%;max-width:300px}</style>";
    html += "<script>let socket=new WebSocket('ws://' + location.host + '/ws');socket.onmessage=e=>{document.getElementById('data').innerHTML=e.data;}</script></head><body>";
    html += "<h2>🌤️ Indoor Weather Data</h2><div id='data'>Loading...</div><hr><h3><a href='/weatherlog.txt'>📁 Download Log</a></h3><form action='/clear' method='POST'><button>🧼 Clear Log</button></form></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/weatherlog.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/weatherlog.txt", "text/plain");
  });

  server.on("/clear", HTTP_POST, [](AsyncWebServerRequest *request) {
    SPIFFS.remove("/weatherlog.txt");
    request->redirect("/");
  });

  server.begin();
}
