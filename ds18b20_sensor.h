#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>
#define DS18B20_PIN 4
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

void setupTemperatureSensors() {
  ds18b20.begin();
  Serial.printf("DS18B20 devices found: %d\n", ds18b20.getDeviceCount());
}
void readTemperatures(float t[3]) {
  ds18b20.requestTemperatures();
  for (int i=0; i<3; i++) {
    float v = ds18b20.getTempCByIndex(i);
    t[i] = (v == DEVICE_DISCONNECTED_C) ? NAN : v;
  }
}
