#pragma once
#include <WiFi.h>
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Wi-Fi");
  unsigned long start=millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-start<10000) {
    delay(300); Serial.print(".");
  }
  Serial.println(WiFi.status()==WL_CONNECTED ? "\nWi-Fi connected" : "\nWi-Fi unavailable; local readings continue");
}
// Upload code will be added after teammate provides backend API URL and JSON format.
void sendTelemetry(const float t[3], float amps, float volts, int prediction) {
  (void)t; (void)amps; (void)volts; (void)prediction;
}
