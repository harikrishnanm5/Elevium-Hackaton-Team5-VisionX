#pragma once
#include <math.h>
// PROVISIONAL order only. Confirm exact feature order with the trained model.
void calculateFeatures(const float t[3], float amps, float volts, float f[8]) {
  f[0]=t[0]; f[1]=t[1]; f[2]=t[2];
  f[3]=t[0]-t[1]; f[4]=t[1]-t[2]; f[5]=t[2]-t[0];
  f[6]=amps; f[7]=volts;
}
bool validFeatures(const float f[8]) {
  for (int i=0; i<8; i++) if (isnan(f[i]) || isinf(f[i])) return false;
  return true;
}
