# ifndef GLOBAL_VAR_H
# define GLOBAL_VAR_H

#include "CanFsmBuffer.h"
#include "CanFloatBuffer.h"
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
float percent=0;
//FSM DataBuffers
CanFsmBuffer caliBuffer;
CanFsmBuffer consensusBuffer;
CanFsmBuffer miniHubBuffer;


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

///CONSENSUS VARIABLES///
float d[3]={};//solutions in percentage
float d_all[3][3]={};
float d_av[3] ={};
float y[3] = {};
int c[3]={};//cost function paramters ≠ final cost!!
float rho=0.07;
int a=0;// Number of current iterations
int maxiter=25;//Number of iterations in consensus
bool solution_ready=0;
bool partial_solution_ready=0;
float final_cost=0;
float d_best[3] = {};
float cost_best = 1000000; //large number
bool sol = 1;
float cost = 1;
float z[3]={};
float w=0;
bool ready_for_consensus=0;

//########## Shaida ##########
String CR; //Client Request
String SR; //Server Response

CanFsmBuffer hubBuffer;
CanFloatBuffer floatHubBuffer;

bool hubServerFlag = false;
bool newMessage= false;
float float_val = 0.0;
bool bool_val = 0;

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
