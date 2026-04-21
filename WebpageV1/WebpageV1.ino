#include <WiFi.h>
#include <WebServer.h>

// -------------------------
// Access Point credentials
// -------------------------
const char* AP_SSID = "LED_Sphere_Setup";
const char* AP_PASSWORD = "sphere123";   // Must be at least 8 chars

// -------------------------
// Web server on port 80
// -------------------------
WebServer server(80);

// Store last received text
String lastReceivedText = "";

// -------------------------
// HTML page stored in flash
// -------------------------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Sphere Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 24px;
      background: #f5f5f5;
      color: #222;
    }
    .card {
      max-width: 500px;
      margin: 40px auto;
      background: white;
      border-radius: 12px;
      padding: 24px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.08);
    }
    h1 {
      margin-top: 0;
      font-size: 1.6rem;
    }
    p {
      color: #555;
    }
    input[type="text"] {
      width: 100%;
      padding: 12px;
      font-size: 1rem;
      margin-top: 8px;
      margin-bottom: 16px;
      border: 1px solid #ccc;
      border-radius: 8px;
      box-sizing: border-box;
    }
    button {
      padding: 12px 18px;
      font-size: 1rem;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      background: #222;
      color: white;
    }
    button:hover {
      opacity: 0.92;
    }
    .status {
      margin-top: 16px;
      font-weight: bold;
    }
    .small {
      margin-top: 20px;
      font-size: 0.9rem;
      color: #666;
      word-break: break-word;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>LED Sphere Control</h1>
    <p>Enter text below and send it to the ESP32.</p>

    <input id="textInput" type="text" placeholder="Type message here..." maxlength="200">
    <button onclick="sendText()">Send to Sphere</button>

    <div class="status" id="status"></div>
    <div class="small" id="deviceInfo">Connecting to device...</div>
  </div>

  <script>
    async function loadStatus() {
      try {
        const res = await fetch('/status');
        const data = await res.json();
        document.getElementById('deviceInfo').innerText =
          "Connected to AP: " + data.ap_name +
          " | Last received text: " + (data.last_text || "(none)");
      } catch (err) {
        document.getElementById('deviceInfo').innerText =
          "Could not get device status.";
      }
    }

    async function sendText() {
      const textValue = document.getElementById('textInput').value.trim();
      const statusEl = document.getElementById('status');

      if (!textValue) {
        statusEl.innerText = "Please enter some text first.";
        return;
      }

      try {
        const response = await fetch('/submit', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
          },
          body: 'message=' + encodeURIComponent(textValue)
        });

        const result = await response.text();
        statusEl.innerText = result;
        loadStatus();
      } catch (err) {
        statusEl.innerText = "Failed to send text to ESP32.";
      }
    }

    loadStatus();
  </script>
</body>
</html>
)rawliteral";

// -------------------------
// Route handlers
// -------------------------
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleSubmit() {
  if (!server.hasArg("message")) {
    server.send(400, "text/plain", "Error: no message received.");
    return;
  }

  lastReceivedText = server.arg("message");

  Serial.println("----- Text received from webpage -----");
  Serial.println(lastReceivedText);
  Serial.println("--------------------------------------");

  // Later, this is where you would pass the text
  // into your display-processing pipeline.

  server.send(200, "text/plain", "Text sent successfully to ESP32.");
}

void handleStatus() {
  String json = "{";
  json += "\"ap_name\":\"" + String(AP_SSID) + "\",";
  json += "\"last_text\":\"" + lastReceivedText + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// -------------------------
// Setup
// -------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting ESP32 in Access Point mode...");

  // Start AP mode
  bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

  if (!apStarted) {
    Serial.println("Failed to start Access Point.");
    while (true) {
      delay(1000);
    }
  }

  IPAddress ip = WiFi.softAPIP();

  Serial.println("Access Point started successfully.");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Open this IP in your browser: http://");
  Serial.println(ip);

  // Web routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

// -------------------------
// Main loop
// -------------------------
void loop() {
  server.handleClient();
}