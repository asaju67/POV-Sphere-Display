#include "web_ui.h"
#include "app_state.h"
#include "config.h"
#include "hall_sensor.h"
#include "patterns.h"
#include "timer_mode.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Globe Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f4f4f4;
      margin: 0;
      padding: 8px;
      color: #222;
    }

    .card {
      width: calc(100vw - 16px);
      max-width: 100%;
      margin: 8px auto;
      background: white;
      border-radius: 14px;
      padding: 16px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.08);
      box-sizing: border-box;
    }

    h1, h3 {
      margin-top: 0;
    }

    textarea {
      width: 100%;
      min-height: 120px;
      padding: 12px;
      border-radius: 8px;
      border: 1px solid #ccc;
      box-sizing: border-box;
      font-family: monospace;
      font-size: 14px;
      resize: vertical;
    }

    button {
      margin-top: 12px;
      padding: 12px 18px;
      border: none;
      border-radius: 8px;
      background: #222;
      color: white;
      cursor: pointer;
      font-size: 15px;
    }

    button:hover {
      opacity: 0.92;
    }

    .status {
      margin-top: 14px;
      font-weight: bold;
    }

    .info {
      margin-top: 10px;
      color: #555;
      line-height: 1.5;
    }

    .hint {
      color: #666;
      font-size: 14px;
      margin-top: 8px;
    }

    .binaryDisplay {
      background: #111;
      padding: 8px;
      border-radius: 10px;
      width: 100%;
      box-sizing: border-box;
      overflow: hidden;
      font-family: monospace;
      font-size: 5px;
      line-height: 1.0;
      letter-spacing: 0px;
      white-space: pre;
    }

    .bit1 { color: #00ff88; }
    .bit0 { color: #111; }
  </style>
</head>
<body>
  <div class="card">
    <h1>64-bit Shift Register Control</h1>

    <div>
      <label><input type="radio" name="mode" value="hex" checked onchange="toggleMode()"> Hex mode</label>
      <label style="margin-left: 16px;"><input type="radio" name="mode" value="timer" onchange="toggleMode()"> Timer mode</label>
    </div>

    <div id="hexModeFields">
      <p>Enter up to 360 64-bit hex values.</p>
      <p class="hint">Each value should be up to 16 hex digits. Example: C000000000000000</p>
      <textarea id="patternInput">C000000000000000
6000000000000000
3000000000000000
1800000000000000
0C00000000000000
0600000000000000
0300000000000000
8000000000000001</textarea>
    </div>

    <div id="timerModeFields" style="display:none;">
      <p>Enter time in HH:MM:SS format.</p>
      <p class="hint">Example: 01:23:45</p>
      <input id="timerInput" type="text" placeholder="00:00:00" style="width:100%; padding:12px; border-radius:8px; border:1px solid #ccc; box-sizing:border-box; font-family:monospace; font-size:14px;" />
    </div>

    <button onclick="sendPatterns()">Send</button>

    <div class="status" id="status"></div>
    <div class="info" id="deviceInfo"></div>

    <h3>Binary Visualization</h3>
    <div id="binaryOutput" class="binaryDisplay">(loading...)</div>
  </div>

  <script>
    function normalizeHexArray(csvString) {
      if (!csvString || csvString.length === 0) return [];
      return csvString
        .split(',')
        .map(s => s.trim())
        .filter(s => s.length > 0)
        .map(s => {
          if (s.startsWith('0x') || s.startsWith('0X')) s = s.slice(2);
          return s.toUpperCase().padStart(16, '0');
        });
    }

    function renderBitColumns(hexValues) {
      const outputEl = document.getElementById('binaryOutput');

      if (!hexValues || hexValues.length === 0) {
        outputEl.innerHTML = "(none)";
        return;
      }

      let html = "";

      for (let bit = 63; bit >= 0; bit--) {
        for (let col = 0; col < hexValues.length; col++) {
          const nibbleIndex = Math.floor((63 - bit) / 4);
          const nibbleChar = hexValues[col][nibbleIndex];
          const nibbleValue = parseInt(nibbleChar, 16);
          const bitInNibble = 3 - ((63 - bit) % 4);
          const bitValue = (nibbleValue >> bitInNibble) & 1;

          html += bitValue ? '<span class="bit1">1</span>' : '<span class="bit0">0</span>';

          if (col < hexValues.length - 1) html += ' ';
        }
        if (bit > 0) html += '\n';
      }

      outputEl.innerHTML = html;
    }

    async function loadStatus() {
      try {
        const res = await fetch('/status');
        const data = await res.json();

        document.getElementById('deviceInfo').innerHTML =
          "AP: " + data.ap_name + "<br>" +
          "Stored 64-bit values: " + data.count + "<br>" +
          "Mode: " + data.mode + (data.mode === 'timer' ? ' (' + data.timer_text + ')' : '') + "<br>" +
          "Fixed latch rate: " + data.fixed_hz + " Hz<br>" +
          "Current step interval: " + data.step_interval_us + " us<br>" +
          "Estimated RPM: " + data.rpm + "<br>" +
          "Hall signal valid: " + data.hall_valid;

        renderBitColumns(normalizeHexArray(data.last_hex_csv));
      } catch (err) {
        document.getElementById('deviceInfo').innerText = "Could not load device status.";
        document.getElementById('binaryOutput').innerHTML = "(status unavailable)";
      }
    }

    function toggleMode() {
      const mode = document.querySelector('input[name="mode"]:checked').value;
      document.getElementById('hexModeFields').style.display = mode === 'hex' ? 'block' : 'none';
      document.getElementById('timerModeFields').style.display = mode === 'timer' ? 'block' : 'none';
      document.getElementById('status').innerText = '';
    }

    async function sendPatterns() {
      const statusEl = document.getElementById('status');
      const mode = document.querySelector('input[name="mode"]:checked').value;
      let body = 'mode=' + encodeURIComponent(mode);

      if (mode === 'timer') {
        const timerValue = document.getElementById('timerInput').value.trim();
        if (!timerValue) {
          statusEl.innerText = "Please enter a time in HH:MM:SS format.";
          return;
        }
        body += '&timer=' + encodeURIComponent(timerValue);
      } else {
        const input = document.getElementById('patternInput').value.trim();
        if (!input) {
          statusEl.innerText = "Please enter some 64-bit hex values first.";
          return;
        }
        body += '&patterns=' + encodeURIComponent(input);
      }

      try {
        const response = await fetch('/submitPatterns', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: body
        });

        const result = await response.text();
        statusEl.innerText = result;
        loadStatus();
      } catch (err) {
        statusEl.innerText = "Failed to send data to ESP32.";
      }
    }

    toggleMode();
    loadStatus();
    setInterval(loadStatus, 500);
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  appState.server.send(200, "text/html", INDEX_HTML);
}

void handleSubmitPatterns() {
  if (!appState.server.hasArg("mode")) {
    appState.server.send(400, "text/plain", "Error: missing mode parameter.");
    return;
  }

  String mode = appState.server.arg("mode");
  bool ok = false;
  String message;

  if (mode == "timer") {
    if (!appState.server.hasArg("timer")) {
      appState.server.send(400, "text/plain", "Error: missing timer value.");
      return;
    }
    const String timerValue = appState.server.arg("timer");
    ok = setTimerModeFromTimeString(timerValue.c_str());
    message = ok ? "Timer loaded successfully." : "Invalid time format. Use HH:MM:SS.";
  } else if (mode == "hex") {
    if (!appState.server.hasArg("patterns")) {
      appState.server.send(400, "text/plain", "Error: missing patterns value.");
      return;
    }
    const String patternData = appState.server.arg("patterns");
    ok = setHexModeFromString(patternData);
    message = ok ? "64-bit hex sequence stored successfully." : "Invalid hex input. Use comma-separated 64-bit values.";
  } else {
    appState.server.send(400, "text/plain", "Error: unknown mode.");
    return;
  }

  if (!ok) {
    appState.server.send(400, "text/plain", message);
    return;
  }

  Serial.println("----- Received display update -----");
  if (appState.timerMode) {
    Serial.printf("Timer mode: %s\n", appState.timerText.c_str());
  } else {
    for (int i = 0; i < appState.patternLength; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(uint64ToHexString(appState.patternBuffer[i]));
    }
  }
  Serial.println("----------------------------------------");

  appState.server.send(200, "text/plain", message);
}

void handleStatus() {
  updateHallDerivedTiming();

  String modeStr = appState.timerMode ? "timer" : "hex";
  String hallValidStr = appState.hallSignalValid ? "true" : "false";

  String json = "{";
  json += "\"ap_name\":\"" + String(AP_SSID) + "\",";
  json += "\"count\":" + String(appState.patternLength) + ",";
  json += "\"mode\":\"" + modeStr + "\",";
  json += "\"timer_text\":\"" + escapeJson(appState.timerText) + "\",";
  json += "\"fixed_hz\":" + String(LATCH_RATE_HZ, 2) + ",";
  json += "\"step_interval_us\":" + String((unsigned long)(USE_HALL_SENSOR ? appState.liveStepIntervalMicros : appState.fixedShiftIntervalMicros)) + ",";
  json += "\"rpm\":\"" + String(appState.smoothedRPM, 1) + "\",";
  json += "\"hall_valid\":\"" + hallValidStr + "\",";
  json += "\"last_hex_csv\":\"" + escapeJson(appState.lastReceivedHex) + "\"";
  json += "}";

  appState.server.send(200, "application/json", json);
}

void handleNotFound() {
  appState.server.send(404, "text/plain", "404: Not found");
}

void setupWebServer() {
  appState.server.on("/", HTTP_GET, handleRoot);
  appState.server.on("/submitPatterns", HTTP_POST, handleSubmitPatterns);
  appState.server.on("/status", HTTP_GET, handleStatus);
  appState.server.onNotFound(handleNotFound);
  appState.server.begin();
}
