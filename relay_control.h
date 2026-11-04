#pragma once
#define RELAY_1_PIN 27
#define RELAY_2_PIN 26
// Verify relay board polarity first. Set to 1 for active-LOW, 0 for active-HIGH.
#define RELAY_ACTIVE_LOW 1
void relayWrite(int pin, bool on) {
#if RELAY_ACTIVE_LOW
  digitalWrite(pin, on ? LOW : HIGH);
#else
  digitalWrite(pin, on ? HIGH : LOW);
#endif
}
void setupRelays() {
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  relayWrite(RELAY_1_PIN, false);
  relayWrite(RELAY_2_PIN, false);
}
void setRelaysOff() {
  relayWrite(RELAY_1_PIN, false);
  relayWrite(RELAY_2_PIN, false);
}
// Provisional demo mapping only: 0 Normal, 1 Warning, 2 High Risk.
// Verify class order from the actual model before enabling.
void applyRelayPrediction(int c) {
  if (c==0) { relayWrite(RELAY_1_PIN,false); relayWrite(RELAY_2_PIN,false); }
  else if (c==1) { relayWrite(RELAY_1_PIN,true); relayWrite(RELAY_2_PIN,false); }
  else if (c==2) { relayWrite(RELAY_1_PIN,true); relayWrite(RELAY_2_PIN,true); }
  else setRelaysOff();
}
