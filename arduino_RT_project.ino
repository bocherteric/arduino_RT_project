#include <can.h>
#include <mcp2515.h>
#include <SPI.h>

#include "Can_frame_stream.h"
#include "CanFsmBuffer.h"
#include "NodesContainer.h"
#include "globalVariables.h"
#include "Node.h"

#define MSB 1
#define LSB 0



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

    
      if (calibration) {
      calibration_function();
      calibration_interface();
      }


  if(ready_for_consensus){
      distributed_solver();
      consensus_interface();
      if(solution_ready){
        ready_for_consensus=0;
        solution_ready=0;
        Serial.println("Here is the final solution of the consensus function");
        for(uint8_t counter=0; counter<=2; counter++){
          Serial.println(d[counter]);
        }
        Serial.print("The cost is: ");
          Serial.println(final_cost);
      }
    }

    //testing hub
    if (calibration) {
      hubServer();
      hubServerResponse();
      hubClient();
    }
   

  }


}



void canWrite(uint32_t id, uint8_t purpose, uint8_t recipient = 0, uint8_t data = 0, uint16_t floatData = 65535) {
  my_can_msg canMsg;
  canMsg.bytes[3] = recipient;
  canMsg.bytes[2] = node.myHwId;
  canMsg.bytes[1] = purpose;
  canMsg.bytes[0] = data;
  if ( write(id, canMsg.value, floatData) != MCP2515::ERROR_OK )
    Serial.println( "MCP2515 Buf Full" );

}

MCP2515::ERROR write(uint32_t id, uint32_t val, uint16_t floatData) {
  Serial.println("am I writing?");
  can_frame frame;
  frame.can_id = id;
  my_can_msg msg;
  msg.value = val;

  if (floatData == 65535) {
    frame.can_dlc = 4;
    for ( int8_t i = 0; i < 4; i++ ) {
      frame.data[i] = msg.bytes[i];
      Serial.println(msg.bytes[i]);
    }
  } else {
    frame.can_dlc = 6;
    for ( int i = 0; i < 4; i++ ) { //prepare can message
      frame.data[i] = msg.bytes[i];
      Serial.println(msg.bytes[i]);
    }
    frame.data[4] = splitInt(floatData, LSB);
    Serial.println(frame.data[4]);
    frame.data[5] = splitInt(floatData, MSB);
    Serial.println(frame.data[5]);
  }
  return mcp2515.sendMessage(&frame);
}

uint8_t splitInt(uint16_t floatData, uint8_t byt) {
  if ( byt == LSB) {
    return (floatData & 0xFF);
  } else if (byt == MSB) {
    return ((floatData >> 8) & 0xFF);
  } else {                                          //Just for testing purposes
    Serial.println("splitInt() wrong byt input!!!!");
    return 0;
  }
}


void canRead() { //!!!!Check excecution time -> Read out one message at a time?!!!!
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
  instructionFloat instrF; // Float Instruction Temp
  bool has_data;
  cli(); has_data = cf_stream.get( frame ); sei();
  while (has_data) {


    if (frame.data[3] == node.myHwId || frame.data[3] == 0) { //check am I the addresee?
      Serial.println("canRead");
      if (frame.can_dlc == 4) {
        for (int i = 0; i < 3; i++) { // Is there a better way, faster, and no copying values?
          instr.data[i] = frame.data[i];
          Serial.println(instr.data[i]);
        }
      } else if (frame.can_dlc == 6) {
        for (int i = 0; i < 6; i++) { // Is there a better way, faster, and no copying values?
          instrF.data[i] = frame.data[i];
          Serial.println(instrF.data[i]);
        }
      } else {
        Serial.println("Error in Can read()");
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
        case 12:
          if (!(consensusBuffer.write(instr)))
            Serial.println("floatconsensusBuffer overflow");
          break;
        case 15:
          if (!(hubBuffer.write(instr)))
            Serial.println("floatHubBuffer overflow");
        case 16:
          if (!(floatHubBuffer.write(instrF)))
            Serial.println("floatHubBuffer overflow");
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
