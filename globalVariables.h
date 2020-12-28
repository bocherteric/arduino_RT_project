# ifndef GLOBAL_VAR_H
# define GLOBAL_VAR_H

#include "CanFsmBuffer.h"
#include "Node.h"



//########## Shared ##########
//System FREQ
unsigned long timeVar = 0;
unsigned long periodVar = 10; // in miliseconds
//Pin declarations
const double analogInPin = A0; 
const int analogOutPin = 3;
//FSM Switch Variables
uint8_t cwuSwitch = 0; //Switch Variable for canWakeUp
//Index
uint8_t index=0;
//Nodes management
NodesContainer nodesCont;
Node node;


//########## Juliette ##########
//First part of project
const double R1=10.0;//  kOhm
//FSM DataBuffers
CanFsmBuffer caliBuffer;

///CALIBRATION VARIABLES///
const int turnOnLightTime=300;
unsigned long prevTime=0;
bool calibration=false;
bool compute_gain=0;
bool over=0;
bool change_lux=1; 
int Ready=0;
float g=0;
float p=20;
int nb_turn;
//########## Shaida ##########


//########## Eric ##########
//FSM Data Buffers
CanFsmBuffer cwuBuffer;
//Objects Can Comm
volatile can_frame_stream cf_stream;
MCP2515 mcp2515(10); //SS pin 10
volatile bool interrupt = false;
volatile bool mcp2515_overflow = false;
volatile bool arduino_overflow = false;
union my_can_msg {
  unsigned long value;
  unsigned char bytes[4];
};
//canWakeUp Variables
int cwuTimer = 0;
uint8_t cwuCounter = 0;
//canWakeUp Flags
bool helloBroadcast = true; // true, bcs helloBroadcast is first thing to happen after boot
bool restartWakeUp = false; 
bool checkedNodes = false;
bool checkIfFirstNode = false;
bool checkedNodesCounter = false;


#endif
