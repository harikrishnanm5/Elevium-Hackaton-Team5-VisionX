#include "ds18b20_sensor.h"
#include "acs712_sensor.h"
#include "voltage_sensor.h"
#include "feature_calculation.h"
#include "random_forest_model.h"
#include "relay_control.h"
#include "wifi_communication.h"

// ============================================================================
// ELEVIUM - EV BATTERY THERMAL RUNAWAY EARLY DETECTION NODE
// Developed for Elevium Hackathon 2026 by Team Vision X:
// Srimathi M | Sri Gayatri G | Hari Krishnan M | Udaya Sankar K
// ============================================================================

unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================================");
  Serial.println("   ELEVIUM: EV BATTERY THERMAL RUNAWAY MONITORING NODE  ");
  Serial.println("       Team Vision X | Dual ML Edge Safety Interlock     ");
  Serial.println("========================================================");

  setupTemperatureSensors();
  setupCurrentSensor(); // Requires zero current during startup calibration
  setupVoltageSensor();
  setupRelays();
  setupWiFi();

  Serial.println("[SYSTEM] Multi-point sensor network & ML inference engine online.\n");
}

void loop() {
  // 1.0 Hz deterministic sampling period (1000 ms)
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  // 1. Acquire raw sensor telemetry
  float t[3];
  readTemperatures(t);
  float amps = readCurrentAmps();
  float volts = readBatteryVoltage();

  Serial.printf("[SENSORS] T1: %.2f C | T2: %.2f C | T3: %.2f C | I: %.3f A | V: %.3f V\n",
                t[0], t[1], t[2], amps, volts);

  // 2. Extract 8-feature multivariate representation
  float features[8];
  calculateFeatures(t, amps, volts, features);

  // 3. Execute Edge AI Random Forest inference
  int prediction = predictRiskClass(features);
  float estimatedCore = predictCoreTemperature(features);

  const char* riskLabels[] = {"NORMAL", "WARNING", "HIGH_RISK", "FAULT"};
  const char* currentLabel = (prediction >= 0 && prediction <= 2) ? riskLabels[prediction] : riskLabels[3];

  Serial.printf("[AI INFERENCE] Risk Class: %d (%s) | Est. Core Temp: %.2f C\n",
                prediction, currentLabel, estimatedCore);

  // 4. Actuate dual-relay active safety interlocks
  if (prediction >= 0) {
    applyRelayPrediction(prediction);
  } else {
    setRelaysOff();
    Serial.println("[SAFETY] Sensor anomaly or ML bypass: Relays held OFF (fail-safe).");
  }

  // 5. Transmit telemetry packet to Web Dashboard & Serial
  sendTelemetry(t, amps, volts, prediction, estimatedCore);
}
