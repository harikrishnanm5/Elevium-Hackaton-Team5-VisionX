#include "ds18b20_sensor.h"
#include "acs712_sensor.h"
#include "voltage_sensor.h"
#include "feature_calculation.h"
#include "random_forest_model.h"
#include "relay_control.h"
#include "wifi_communication.h"

unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  setupTemperatureSensors();
  setupCurrentSensor(); // Requires zero current during startup calibration
  setupVoltageSensor();
  setupRelays();
  setupWiFi();
  Serial.println("Sensor monitor started. Real ML model is not installed yet.");
}

void loop() {
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  float t[3];
  readTemperatures(t);
  float amps = readCurrentAmps();
  float volts = readBatteryVoltage();

  Serial.printf("T1 %.2f C | T2 %.2f C | T3 %.2f C | I %.3f A | V %.3f V\n",
                t[0], t[1], t[2], amps, volts);

  float features[8];
  calculateFeatures(t, amps, volts, features);
  int prediction = predictRiskClass(features);

  if (prediction >= 0) applyRelayPrediction(prediction);
  else {
    setRelaysOff();
    Serial.println("ML not configured: relays held OFF.");
  }
  sendTelemetry(t, amps, volts, prediction);
}
