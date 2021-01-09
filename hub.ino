#define RPI 0


void hubServer() {

  if (Serial.available() > 0) {

    hubServerFlag = true;
    newMessage = true;
    CR = Serial.readStringUntil('\n');
    if(!RPI){
      Serial.println(CR);
    }
    /*
      Serial.print("The Arduino number ");
      Serial.print(node.myHwId);
      Serial.println(" is the hub");
    */
  }

  if (hubServerFlag && newMessage) {
    uint8_t hwId, temp, exponent;
    float tempF;
    switch (CR[0]) {

      case 'g':
        hwId = static_cast<uint8_t>(CR[2] - '0');
        switch (CR[1]) {
          case 'I':
            //Serial.println(hwId);
            //Serial.println(node.myHwId);
            if (hwId == node.myHwId) {
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(read_lux());
            } else if (nodesCont.nodeAvailable(hwId)) {
              canWrite(0, 15, hwId);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          case 'd':
            if (hwId == node.myHwId) {
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(outputDC);
            } else if (nodesCont.nodeAvailable(hwId)) {
              canWrite(0, 17, hwId);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          case 'o':
            if (hwId == node.myHwId) {
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(occupancy);
            } else if (nodesCont.nodeAvailable(hwId)) {
              canWrite(0, 19, hwId);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          case 'O':
            if (hwId == node.myHwId) {
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(luxOccupied);
            } else if (nodesCont.nodeAvailable(hwId)) {
              canWrite(0, 23, hwId);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          case 'U':
            if (hwId == node.myHwId) {
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(luxUnoccupied);
            } else if (nodesCont.nodeAvailable(hwId)) {
              canWrite(0, 25, hwId);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          case 'L': 
            if(hwId == node.myHwId || nodesCont.nodeAvailable(hwId)){
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(node.L[hwId-1]);
            } else {
              Serial.println("Requested Node is NOT available");
            }
            break;

          default:

            Serial.println("Invalid command gCR[1]");

        }
        break;

      case 'o':
        hwId = static_cast<uint8_t>(CR[1] - '0');
        temp = static_cast<uint8_t>(CR[2] - '0');
        if (temp > 1) { //no useless CAN message necessary
          Serial.println("err");
          break;
        } else if (hwId == node.myHwId) {
          if (occupancy == temp) {
            Serial.println("err");
          } else {
            occupancy = temp;
            conStartSwitch=1;  //INITIATE CONSENSUS IF NEED BE
            Serial.println("ack");
          }
        } else if (nodesCont.nodeAvailable(hwId)) {
          canWrite(0, 21, hwId,temp);
        } else {
          Serial.println("Requested Node is NOT available");
        }
        break;

      case 'O':
        hwId = static_cast<uint8_t>(CR[1] - '0');
        tempF = (CR.substring(2)).toFloat();
        if (hwId == node.myHwId) {
          if (tempF > node.maxLux) {
            Serial.println("err");
          } else {
            luxOccupied = tempF;
            conStartSwitch=2;  //INITIATE CONSENSUS IF NEED BE
            Serial.println("ack");
          }
        } else if (nodesCont.nodeAvailable(hwId)) {
          exponent = getExponent(tempF);
          canWrite(0, 27, hwId, exponent, floatToCan(tempF, exponent));
        } else {
          Serial.println("Requested Node is NOT available");
        }
        break;

      case 'U':
        hwId = static_cast<uint8_t>(CR[1] - '0');
        tempF = (CR.substring(2)).toFloat();
        if (hwId == node.myHwId) {
          if (tempF > node.maxLux) {
            Serial.println("err");
          } else {
            luxUnoccupied = tempF;
            conStartSwitch=3;  //INITIATE CONSENSUS IF NEED BE
            Serial.println("ack");
          }
        } else if (nodesCont.nodeAvailable(hwId)) {
          exponent = getExponent(tempF);
          canWrite(0, 29, hwId, exponent, floatToCan(tempF, exponent));
        } else {
          Serial.println("Requested Node is NOT available");
        }
        break;
      default:

        Serial.println("Invalid command CR[0]");

    }
    newMessage = false;

  }
}

void hubServerResponse() {

  if (hubServerFlag) {
    instruction tempInst; // create temporary instruction to store received can message
    instructionFloat tempInstF;
    float tempF;
    
    if (hubBuffer.read(tempInst)) {
      
      switch (tempInst.data[1]) {
          case 18:
          Serial.print("d");
          Serial.print(tempInst.data[2]);
          Serial.println(tempInst.data[0]);
          break;
          case 20:
          Serial.print("o");
          Serial.print(tempInst.data[2]);
          Serial.println(tempInst.data[0]);
          break;
          case 22:
          if(tempInst.data[0]==1){
            Serial.println("ack");
          }else{
            Serial.println("err");
          }
          break;
          case 28:
          if(tempInst.data[0]==1){
            Serial.println("ack");
          }else{
            Serial.println("err");
          }
          break;
          case 30:
          if(tempInst.data[0]==1){
            Serial.println("ack");
          }else{
            Serial.println("err");
          }
          break;
      }

    }

    if (floatHubBuffer.read(tempInstF)) {
      tempF=canToFloat(tempInstF.data[5], tempInstF.data[4], tempInstF.data[0]);
      switch (tempInstF.data[1]) {
        case 16:
          Serial.print("I");
          Serial.print(tempInstF.data[2]);
          Serial.println(tempF);
          break;
        case 24:
          Serial.print("O");
          Serial.print(tempInstF.data[2]);
          Serial.println(tempF);
        break;
        case 26:
          Serial.print("U");
          Serial.print(tempInstF.data[2]);
          Serial.println(tempF);
        break;
        case 99:
        node.L[tempInstF.data[2]-1]=tempF;
        ready_for_consensus = true;
          
      }

    }
  }
}

void hubClient() {
  if (!hubServerFlag) {
    instruction tempInst;
    instructionFloat tempInstF;
    float tempF;
    uint8_t temp, exponent;
    if (hubBuffer.read(tempInst)) {
      switch (tempInst.data[1]) {
        case 15:
          tempF = read_lux();
          exponent = getExponent(tempF);
          canWrite(0, 16, tempInst.data[2], exponent, floatToCan(tempF, exponent));
          break;
        case 17:
          canWrite(0, 18, tempInst.data[2], outputDC);
          break;
        case 19:
          canWrite(0, 20, tempInst.data[2], occupancy);
          break;
        case 21:
          if(tempInst.data[0]==occupancy){
            temp =0;
          }else{
            temp =1;
            conStartSwitch=1;  //INITIATE CONSENSUS IF NEED BE
            occupancy=tempInst.data[0];
          }
          canWrite(0, 22, tempInst.data[2], temp);
          break;
        case 23:
          exponent = getExponent(luxOccupied);
          canWrite(0, 24, tempInst.data[2], exponent, floatToCan(luxOccupied, exponent));
          break;
        case 25:
          exponent = getExponent(luxUnoccupied);
          canWrite(0, 26, tempInst.data[2], exponent, floatToCan(luxUnoccupied, exponent));
          break;
      }
    }

    if (floatHubBuffer.read(tempInstF)) {
      tempF=canToFloat(tempInstF.data[5], tempInstF.data[4], tempInstF.data[0]);
      switch (tempInstF.data[1]) {
        case 27:
        if (tempF > node.maxLux) {
            temp= 0;
          } else {
            luxOccupied = tempF;
            conStartSwitch=2;  //INITIATE CONSENSUS IF NEED BE
            temp=1;
          }
         canWrite(0, 28, tempInstF.data[2], temp);
          break;
        case 29:
        if (tempF > node.maxLux) {
            temp= 0;
          } else {
            luxUnoccupied = tempF;
            conStartSwitch=3;  //INITIATE CONSENSUS IF NEED BE
            temp=1;
          }
         canWrite(0, 28, tempInstF.data[2], temp);
          break;
        case 99:
        node.L[tempInstF.data[2]-1]=tempF;
        ready_for_consensus = true;
      }

    }

  }
}

void consensusStart(){
  uint8_t exponent;
  
  switch(conStartSwitch){
    case 0: //IDLE
      break;
    case 1: //OCCUPANCY CHANGED
      if(ready_for_consensus)//Consensus running at the moment
        break;
      if(occupancy){
        node.L[node.myHwId-1]=luxOccupied;
      }else{
        node.L[node.myHwId-1]=luxUnoccupied;
      }
      ready_for_consensus = true;
      exponent = getExponent(node.L[node.myHwId-1]);
      canWrite(0, 99, 0, exponent, floatToCan(node.L[node.myHwId-1], exponent));
      conStartSwitch=0;
      break;
    case 2: // ILLUMINANCE OCCUPIED STATE CHANGED
      if(!occupancy){
        conStartSwitch=0;
        break;
      }
      if(ready_for_consensus)//Consensus running at the moment
        break;
      ready_for_consensus = true;
      node.L[node.myHwId-1]=luxOccupied;
      exponent = getExponent(node.L[node.myHwId-1]);
      canWrite(0, 99, 0, exponent, floatToCan(node.L[node.myHwId-1], exponent));
      conStartSwitch=0;
      break;
    case 3: // ILLUMINANCE UNOCCUPIED STATE CHANGED
      if(occupancy){
        conStartSwitch=0;
        break;
      }
      if(ready_for_consensus)//Consensus running at the moment
        break;
      ready_for_consensus = true;
      node.L[node.myHwId-1]=luxUnoccupied;
      exponent = getExponent(node.L[node.myHwId-1]);
      canWrite(0, 99, 0, exponent, floatToCan(node.L[node.myHwId-1], exponent));
      conStartSwitch=0;
      break;
    default:
      Serial.print("Error consensusStart()"); 
  }
}



uint8_t getExponent(float tempF) {
  if (static_cast<long>(tempF * 100) < 65535) {
    return 100;
  } else {
    return 10;
  }
}

uint16_t floatToCan(float f, uint8_t e) {
  return static_cast<uint16_t>(f * e);
}

float canToFloat(uint8_t msb, uint8_t lsb, uint8_t e) {
  uint16_t tempF = (msb << 8) | lsb;
  return (static_cast<float>(tempF)) / e;
}
