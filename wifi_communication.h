#pragma once
#include <WiFi.h>
#include <HTTPClient.h>

// ============================================================================
// ELEVIUM - IOT TELEMETRY & CLOUD COMMUNICATIONS
// Team Vision X | Real-Time Telemetry Stream to Web Dashboard
// ============================================================================

// Wi-Fi Credentials (Configure for your access point)
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Telemetry Endpoint URL (Supports Node/Express, Flask, Firebase RTDB, or Vite Dev Server)
const char* TELEMETRY_URL = "http://192.168.1.100:5173/api/telemetry";

// Telemetry sequence tracking
static unsigned long packetSequence = 0;

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Connecting to ");
  Serial.print(WIFI_SSID);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected successfully!");
    Serial.printf("[WiFi] Assigned IP Address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] Network unavailable - running in autonomous offline edge mode.");
  }
}

/**
 * @brief Serializes sensor readings and ML predictions into JSON and transmits to server.
 * Also prints JSON payload over Serial for direct USB dashboard connection or logging.
 */
void sendTelemetry(const float t[3], float amps, float volts, int prediction, float estimatedCoreTemp = NAN) {
  packetSequence++;

  // Build JSON telemetry string
  char jsonBuffer[384];
  const char* riskLabels[] = {"NORMAL", "WARNING", "HIGH_RISK", "UNKNOWN"};
  const char* riskStr = (prediction >= 0 && prediction <= 2) ? riskLabels[prediction] : riskLabels[3];

  snprintf(jsonBuffer, sizeof(jsonBuffer),
    "{\"seq\":%lu,\"device\":\"Elevium-Node-01\",\"uptime_ms\":%lu,"
    "\"t1\":%.2f,\"t2\":%.2f,\"t3\":%.2f,\"dT12\":%.2f,\"dT23\":%.2f,\"dT31\":%.2f,"
    "\"current_amps\":%.3f,\"pack_volts\":%.3f,\"risk_class\":%d,\"risk_label\":\"%s\","
    "\"core_temp_est\":%.2f}",
    packetSequence, millis(),
    t[0], t[1], t[2], (t[0] - t[1]), (t[1] - t[2]), (t[2] - t[0]),
    amps, volts, prediction, riskStr,
    isnan(estimatedCoreTemp) ? 0.0f : estimatedCoreTemp
  );

  // Stream telemetry to Serial for local monitoring / Web Serial API
  Serial.print("[TELEMETRY] ");
  Serial.println(jsonBuffer);

  // Transmit over Wi-Fi if connected and URL configured
  if (WiFi.status() == WL_CONNECTED && TELEMETRY_URL != nullptr && strlen(TELEMETRY_URL) > 7) {
    HTTPClient http;
    http.begin(TELEMETRY_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(2000); // 2-second timeout to avoid blocking real-time control loop

    int httpCode = http.POST((uint8_t*)jsonBuffer, strlen(jsonBuffer));
    if (httpCode > 0) {
      if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_CREATED) {
        Serial.printf("[HTTP] POST status: %d\n", httpCode);
      }
    } else {
      Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
