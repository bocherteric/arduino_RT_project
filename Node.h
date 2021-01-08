#ifndef NODE_H
#define NODE_H

#include <EEPROM.h>

class Node{
  public:
  float L[3]={15,30,0};
  float low_bound=15;
  float maxLux=0;
  float c=1;
  float o=50;
  uint8_t myHwId;
  float k[3]={};
  float b;
  float m;
  double tau1;
  double tau2;
  
  float tau(float lux_asked);
  void setValues();  
};

inline void Node::setValues(){
  myHwId=EEPROM.read(16);
  b=(1-2*EEPROM.read(0))*(EEPROM.read(1)+EEPROM.read(2)/pow(10,EEPROM.read(3)));
  Serial.print("M VALUE: ");
  m=(1-2*EEPROM.read(4))*(EEPROM.read(5)+EEPROM.read(6)/pow(10,EEPROM.read(7))); 
  Serial.println(m);
  tau1=(1-2*EEPROM.read(8))*(EEPROM.read(9)+EEPROM.read(10)/pow(10,EEPROM.read(11)));
  tau2=(1-2*EEPROM.read(12))*(EEPROM.read(13)+EEPROM.read(14)/pow(10,EEPROM.read(15)));
  
}

inline float Node::tau(float lux_asked){
  if(myHwId==1) {
    double R2= 10*1023/lux_asked-10; //compute R2 in Kohm
    return pow(10,-6)*R2*10/(R2+10)+tau1; //Req*C1+constante, 10Kohm=R1 and C1=1µF
  }
  else if(myHwId==2) return tau1*log10(lux_asked)+tau2;
  else if(myHwId==3) return tau1*log(lux_asked)+tau2;
}

#endif 
