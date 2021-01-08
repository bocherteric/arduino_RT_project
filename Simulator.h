#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "node.h"

class Simulator {

  public:
    void simulatorStart(float lux_asked);
    float vldr(float vi, float ti, float currentTime );
  private:
    float tau = 0.0;
    float vf = 0;

};
Node node;
void Simulator::simulatorStart(float lux_asked) {
  float R2s = pow(10, node.m * log10(lux_asked) + node.b); //
  tau = node.tau(lux_asked); //compute  simulation tau (us)
  if (lux_asked == 0)
    tau = 1; //to avoid division by inf in the computation of vldr
  vf = 10 * 5 / (10 + R2s);
}


float Simulator::vldr(float vi, float ti, float currentTime) {
  return vf - (vf - vi) * exp(-(currentTime - ti) / (tau)); // compute the expected Vld (V)
}

#endif
