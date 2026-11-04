#pragma once
#define VOLTAGE_SENSOR_PIN 34
// Assumption for a common 0-25V module. Verify/calibrate your exact board.
const float VOLTAGE_MODULE_RATIO = 5.0f;
void setupVoltageSensor() {
  analogReadResolution(12);
  analogSetPinAttenuation(VOLTAGE_SENSOR_PIN, ADC_11db);
}
float readBatteryVoltage() {
  unsigned long sum = 0;
  for (int i=0; i<20; i++) {
    sum += analogReadMilliVolts(VOLTAGE_SENSOR_PIN);
    delay(2);
  }
  float adcV = (sum / 20.0f) / 1000.0f;
  return adcV * VOLTAGE_MODULE_RATIO;
}
