#pragma once
#define ACS712_PIN 35
// Divider: ACS OUT -- 10k -- ADC node -- 20k -- GND.
// ADC node voltage is 2/3 of ACS output, so multiply by 1.5.
const float ACS_DIVIDER_RECOVERY = 1.5f;
const float ACS_SENSITIVITY = 0.185f; // ACS712-05B: 185 mV/A
float acsZeroV = 0;

void setupCurrentSensor() {
  analogReadResolution(12);
  analogSetPinAttenuation(ACS712_PIN, ADC_11db);
  Serial.println("Ensure ZERO current through ACS712 during offset calibration.");
  delay(300);
  unsigned long sum = 0;
  for (int i=0; i<200; i++) {
    sum += analogReadMilliVolts(ACS712_PIN);
    delay(3);
  }
  acsZeroV = (sum / 200.0f) / 1000.0f * ACS_DIVIDER_RECOVERY;
  Serial.printf("ACS zero voltage: %.3f V\n", acsZeroV);
}
float readCurrentAmps() {
  unsigned long sum = 0;
  for (int i=0; i<20; i++) {
    sum += analogReadMilliVolts(ACS712_PIN);
    delay(2);
  }
  float adcV = (sum / 20.0f) / 1000.0f;
  float sensorV = adcV * ACS_DIVIDER_RECOVERY;
  return (sensorV - acsZeroV) / ACS_SENSITIVITY;
}
