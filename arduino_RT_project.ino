#include <can.h>
#include <mcp2515.h>
#include <SPI.h>

#include "Can_frame_stream.h"
#include "CanFsmBuffer.h"
#include "NodesContainer.h"
#include "globalVariables.h"
#include "Node.h"



// Git test

void setup() {
  Serial.begin(2000000);
  SPI.begin();
  attachInterrupt(0, irqHandler, FALLING);
  SPI.usingInterrupt(0);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();
  node.setValues();
}

void loop() {
  while (millis() - timeVar >= periodVar) {
    timeVar = millis();
    if ( interrupt ) {
      canRead();
    }

    canWakeUpInterface();
    canWakeUp();

    
    if(calibration){
    calibration_function();
    calibration_interface();
    }

    /*
    //testing hub
    if (calibration){
      hubServer();
      hubServerResponse();
      hubClient();
    }
    */

  }

}



void canWrite(uint32_t id, uint8_t purpose, uint8_t recipient = 0, uint8_t data = 0) {
  my_can_msg canMsg;
  canMsg.bytes[3] = recipient;
  canMsg.bytes[2] = node.myHwId;
  canMsg.bytes[1] = purpose;
  canMsg.bytes[0] = data;
  if ( write(id, canMsg.value) != MCP2515::ERROR_OK )
    Serial.println( "MCP2515 Buf Full" );
}

void canRead() { //!!!!Check excecution time -> Read out one message at a time?!!!!
  Serial.println("canRead");
  interrupt = false;
  if ( mcp2515_overflow ) {
    Serial.println( "\tMCP2516 RX Buf Overflow" );
    mcp2515_overflow = false;
  }
  if ( arduino_overflow ) {
    Serial.println( "\tArduino Buffers Overflow" );
    arduino_overflow = false;
  }
  can_frame frame; //Can Message Temp
  instruction instr; // FSM Instruction Temp
  bool has_data;
  cli(); has_data = cf_stream.get( frame ); sei();
  while (has_data) {
    if (frame.data[3] == node.myHwId || frame.data[3] == 0) { //check am I the addresee?
      for (int i = 0; i < 3; i++) { // Is there a better way, faster, and no copying values?
        instr.data[i] = frame.data[i];
      }
      switch (frame.data[1]) { //check which FSM needs to hear about this can message -> store in respective vector
        case 0://wake-up
        case 1:
        case 2:
        case 3:
          if (!(cwuBuffer.write(instr)))
            Serial.println("cwuBuffer overflow");
          break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
          if (!(caliBuffer.write(instr)))
            Serial.println("caliBuffer overflow");
          break;
        case 9:
        case 10:
        case 11:
        if (!(consensusBuffer.write(instr)))
            Serial.println("consensusBuffer overflow");
          break;
        case 12:
        case 13:
        case 14:
          if (!(miniHubBuffer.write(instr)))
            Serial.println("miniHubBuffer overflow");
          break;
        case 15:
        case 16:
          if (!(hubBuffer.write(instr)))
            Serial.println("hubBuffer overflow");
          break;

          

      }
    } else {
      Serial.println("I am not the addresse");
    }
    cli(); has_data = cf_stream.get( frame ); sei();
  }

}

void irqHandler() {
  can_frame frm;
  uint8_t irq = mcp2515.getInterrupts();
  //check messages in buffer 0
  if ( irq & MCP2515::CANINTF_RX0IF ) {
    mcp2515.readMessage( MCP2515::RXB0, & frm );
    if ( !cf_stream.put( frm ) ) //no space
      arduino_overflow = true;
  }

  //check messages in buffer 1
  if ( irq & MCP2515::CANINTF_RX1IF ) {
    mcp2515.readMessage( MCP2515::RXB1, & frm);
    if ( !cf_stream.put( frm ) ) //no space
      arduino_overflow = true;
  }
  irq = mcp2515.getErrorFlags(); //read EFLG
  if ( (irq & MCP2515::EFLG_RX0OVR) | (irq & MCP2515::EFLG_RX1OVR) ) {
    mcp2515_overflow = true;
    mcp2515.clearRXnOVRFlags();
  }
  mcp2515.clearInterrupts();
  interrupt = true;
}

MCP2515::ERROR write(uint32_t id, uint32_t val) {
  Serial.println("am I writing?");
  can_frame frame;
  frame.can_id = id;
  frame.can_dlc = 4;
  my_can_msg msg;
  msg.value = val; //pack data
  for ( int i = 0; i < 4; i++ ) { //prepare can message
    frame.data[i] = msg.bytes[i];
    Serial.println(msg.bytes[i]);
  }
  //send data
  return mcp2515.sendMessage(&frame);
}



double mean_analogread(){ //does the average of the value mesured to avoid noise
  
  double sample1=analogRead(analogInPin);
  double sample2=analogRead(analogInPin);
  double sample3=analogRead(analogInPin);
  double sample4=analogRead(analogInPin);
  return (sample1+sample2+sample3+sample4)/4;
}

double read_lux(){
    double R2= R1*1023/mean_analogread()-R1; //compute R2 in Kohm
    return pow(10,(log10(R2)-node.b)/node.m);
}

double read_voltage(){
  return mean_analogread()*5.0/1023.0;
}
