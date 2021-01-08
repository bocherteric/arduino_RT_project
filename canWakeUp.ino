#include "globalVariables.h"

void canWakeUp() {

  bool senderFlag = true;

  static int cwuSwitchOld = 0; // static not static?
  instruction tempInst;

  switch (cwuSwitch) { //Test changes: canWrite in case1, data.byte[2->0] in interface, cwuSwitch=3 in case 1, canWakeUpINterface in loop
    case 0: // HELLO BROADCAST
      cwuTimer ++;
      if (cwuTimer == 100) {
        canWrite(0, 0);
      }
      if (cwuTimer == 200) {
        cwuTimer = 0;
        cwuCounter++;
        if (cwuCounter == 3) {
          helloBroadcast = false;
          cwuSwitch++;
        }
      }
      break;
    case 1: //CHECK NUMBER OF NODES
      if (nodesCont.checkIfFirstNode(node.myHwId)) {
        canWrite(0, 1, nodesCont.setNextNode(node.myHwId), nodesCont.numberOfNodes());
        checkIfFirstNode = true;
        checkedNodes = true;
      }
      cwuSwitch++;
    case 2: //CHECK NUMBER OF NODES
      if (index == node.myHwId) {
        canWrite(0, 1, nodesCont.setNextNode(node.myHwId), nodesCont.numberOfNodes());
        checkedNodes = true;
      }
      if (restartWakeUp) {
        cwuSwitch = 4;
      } else if (checkedNodes) {
        index = 0;
        cwuSwitch++;
      }
      break;
    case 3: //IDLE
      if (restartWakeUp) {
        cwuSwitch = 4;
      }
      break;
    case 4: ////RESTART WAKE UP SCENARIO 1: BROADCAST RESTART + RESET + RESTART
      canWrite(0, 2);
      resetWakeUpVar();
      cwuSwitch = 0;
      break;
    case 5: //FINISH WAKE UP
      Serial.println("Wake-Up is finished1");
      canWrite(0, 3, 0, node.myHwId);
      calibration=true;
      wakeUp=0;
      cwuSwitch++;
      break;
    case 6: //DEBUG
      Serial.println("Nodes:");
      for (uint8_t i = 0; i < nodesCont.numberOfNodes(); i++)
        Serial.println(nodesCont.getNode(i));
      Serial.println("Next_Node:");
      Serial.println(nodesCont.getNextNode());
      cwuSwitch = 3;
      break;
  }
}

void canWakeUpInterface() {
  instruction tempInst; // create temporary instruction to store received can message
  if (cwuBuffer.read(tempInst)) {
    switch (tempInst.data[1]) { //purpose byte of can message
      case 0: //HELLO BROADCAST
        if (!nodesCont.newNode(tempInst.data[2])) {
          break;
        } else if (helloBroadcast) {
          nodesCont.addNode(tempInst.data[2]);
        } else if (!helloBroadcast) {
          restartWakeUp = true;     // RESTART: Because new node wants to join after hello finished
        }
        break;
      case 1: //CHECK NUMBER OF NODES
        if (checkedNodesCounter) { // ERROR: no perfect circle
          restartWakeUp = true;
        } else if (checkIfFirstNode) {
          index = node.myHwId;
          cwuSwitch = 5;       // Broadcast Start
        } else if (nodesCont.numberOfNodes() == tempInst.data[0]) {
          index = node.myHwId;
        } else {
          restartWakeUp = true; //ERROR: differnet number of nodes detected
        }
        checkedNodesCounter = true;
        break;
      case 2: //RESTART WAKE UP SCENARIO 2: RESET + RESTART
        resetWakeUpVar(); //reset
        cwuSwitch = 0; //restart
        break;
      case 3: //WAKE-UP FINISHED
        calibration=true;
        wakeUp=0;
        index = tempInst.data[0];
        Serial.println("Wake-Up is finished2");
        cwuSwitch = 6;
        break;
    }
  }
}

void resetWakeUpVar() {
  Serial.println("Wake-Up Sequence was restarted!");
  nodesCont.reset();
  cwuTimer = 0;
  cwuCounter = 0;
  helloBroadcast = true;
  restartWakeUp = false;
  checkedNodes = false;
  checkIfFirstNode = false;
  checkedNodesCounter = false;
}
