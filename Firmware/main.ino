#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "YourWifiNameHereIAintGivingMine";
const char* password = "idkmansomelameahhpassword";

#define DHTPIN 18
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

float temperature = 0;
float humidity = 0;
unsigned long lastRead = 0;
const long readInterval = 2000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Weather Station</title>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Poppins', sans-serif;
      background: #000;
      min-height: 100vh;
      color: #fff;
      padding: 40px 20px;
    }
    .container { max-width: 720px; margin: 0 auto; }
    header { text-align: center; margin-bottom: 60px; }
    h1 { font-size: 1.5rem; font-weight: 600; letter-spacing: -0.5px; margin-bottom: 8px; }
    .status { font-size: 0.75rem; color: #666; font-weight: 300; }
    .status span { color: #fff; }
    .cards { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
    @media (max-width: 500px) { .cards { grid-template-columns: 1fr; } }
    .card {
      border: 1px solid #222;
      padding: 40px 30px;
      text-align: center;
    }
    .card-icon { margin-bottom: 20px; }
    .card-icon svg { width: 32px; height: 32px; stroke: #fff; fill: none; stroke-width: 1.5; }
    .card-label {
      font-size: 0.7rem;
      text-transform: uppercase;
      letter-spacing: 2px;
      color: #666;
      margin-bottom: 12px;
      font-weight: 400;
    }
    .card-value { font-size: 3rem; font-weight: 300; }
    .card-unit { font-size: 1rem; color: #666; margin-left: 4px; font-weight: 300; }
    .update {
      text-align: center;
      margin-top: 40px;
      font-size: 0.7rem;
      color: #444;
      font-weight: 300;
    }
    footer {
      text-align: center;
      margin-top: 60px;
      font-size: 0.7rem;
      color: #333;
      font-weight: 300;
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>Weather Station</h1>
      <p class="status">ESP32 <span>Online</span></p>
    </header>
    <div class="cards">
      <div class="card">
        <div class="card-icon">
          <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2v20M12 2a4 4 0 0 1 4 4v10a4 4 0 1 1-8 0V6a4 4 0 0 1 4-4z"/>
            <circle cx="12" cy="16" r="2"/>
          </svg>
        </div>
        <div class="card-label">Temperature</div>
        <div class="card-value"><span id="temp">--</span><span class="card-unit">C</span></div>
      </div>
      <div class="card">
        <div class="card-icon">
          <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2v6M12 22a6 6 0 1 1 0-12 6 6 0 0 1 0 12z"/>
            <path d="M12 8a6 6 0 0 0-6 6c0 3.3 2.7 6 6 6"/>
          </svg>
        </div>
        <div class="card-label">Humidity</div>
        <div class="card-value"><span id="humid">--</span><span class="card-unit">%</span></div>
      </div>
    </div>
    <p class="update">Updated <span id="time">--</span></p>
    <footer>Saurabh / Hack Club Blueprint</footer>
  </div>
  <script>
    async function fetchData() {
      try {
        const r = await fetch('/data');
        const d = await r.json();
        document.getElementById('temp').textContent = d.temperature.toFixed(1);
        document.getElementById('humid').textContent = d.humidity.toFixed(1);
        document.getElementById('time').textContent = new Date().toLocaleTimeString();
      } catch (e) { console.error(e); }
    }
    fetchData();
    setInterval(fetchData, 3000);
  </script>
</body>
</html>
)rawliteral";

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println("WEATHER STATION");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Temp:");
  display.setTextSize(2);
  display.setCursor(40, 15);
  display.print(temperature, 1);
  display.setTextSize(1);
  display.print(" C");
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("Humid:");
  display.setTextSize(2);
  display.setCursor(40, 37);
  display.print(humidity, 1);
  display.setTextSize(1);
  display.print(" %");
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("IP:");
  display.print(WiFi.localIP());
  display.display();
}

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
    Serial.printf("Temp: %.1f°C | Humidity: %.1f%%\n", temperature, humidity);
  } else {
    Serial.println("DHT11 read failed!");
  }
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  String json = "{\"temperature\":" + String(temperature) + 
                ",\"humidity\":" + String(humidity) + "}";
  server.send(200, "application/json", json);
}
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Weather Station ===");
  dht.begin();
  Serial.println("DHT11 initialized");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed!");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println("Connecting WiFi...");
  display.display();
  Serial.println("OLED initialized");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started");
  readSensor();
  updateDisplay();
}

void loop() {
  server.handleClient();
  if (millis() - lastRead >= readInterval) {
    lastRead = millis();
    readSensor();
    updateDisplay();
  }
}