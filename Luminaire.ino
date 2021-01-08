
void luminaire() {

  unsigned long currentTime = millis();
  float lux_asked=percent*node.maxLux/100;
  float calculated_voltage = prevision.vldr(vi, ti, currentTime);
  
  float read_volt = read_voltage();
  float ff = feedforward(node.k[node.myHwId-1], lux_asked);
  
  if ( controler.calc(calculated_voltage, read_volt) + ff < 0)
  {
    outputDC = controler.anti_wdup(0, 1, ff)+ff;
  } else if (ff + controler.calc(calculated_voltage, read_volt) > 255)
  {
    outputDC = controler.anti_wdup(1, 0, ff) + ff;
  } else {
    outputDC = controler.calc(calculated_voltage, read_volt) + ff;
  }
  analogWrite(analogOutPin, outputDC);

}

float feedforward( float gain, float lux_asked)
{
  return lux_asked/gain;
};
