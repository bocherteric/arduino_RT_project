


void hubServer() {

  if (Serial.available() > 0) {

    hubServerFlag = true;
    newMessage = true;
    CR = Serial.readStringUntil('\n');

    Serial.print("The Arduino number ");
    Serial.print(node.myHwId);
    Serial.println(" is the hub");

  }

  if (hubServerFlag && newMessage) {

    switch (CR[0]) {

      case 'g':
        Serial.println(" g entered");
        switch (CR[1]) {

          case 'I':
            Serial.println(" I enterd");
            uint8_t hwId = static_cast<uint8_t>(CR[2] - '0');
            Serial.println(hwId);
            //Serial.println(hwId2);
            Serial.println(node.myHwId);
            if (hwId == node.myHwId) {
              float_val = read_lux();
              Serial.print(CR[1]);
              Serial.print(CR[2]);
              Serial.println(float_val);
            } else {

              canWrite(0, 15, hwId);

            }

            break;

          case 'd':
            break;

          case 'o':
            break;

          case 'O':
            break;

          case 'U':
            break;

          case 'L':
            break;

          case 'x':
            break;

          case 'r':
            break;

          case 'c':
            break;

          case 'p':
            break;

          case 't':
            break;

          case 'e':
            break;

          case 'v':
            break;

          case 'f':
            break;

          default:

            Serial.println("Invalid command");

        }

        break;

      case 'o':
        break;

      case 'O':
        break;

      case 'U':
        break;

      case 'c':
        break;

      case 'r':
        break;

      /*case 'b':
        break;

        case 's':
        break;*/

      default:

        Serial.println("Invalid command");

    }
    newMessage = false;

  }
}

void hubServerResponse() {

  if (hubServerFlag) {
    instruction tempInst; // create temporary instruction to store received can message
    instructionFloat tempInstF;

    if (hubBuffer.read(tempInst)) {
      switch (tempInst.data[1]) {
          //here go all the cases that are not floats
      }

    }

    if (floatHubBuffer.read(tempInstF)) {
      switch (tempInstF.data[1]) {
        case 16:
          for (uint8_t i = 0; i < 5; i++) {
            Serial.println(tempInst.data[i]);
          }
          float_val = canToFloat(tempInstF.data[5], tempInstF.data[4], tempInstF.data[0]);
          Serial.print("I");
          Serial.print(tempInstF.data[2]);
          Serial.println(float_val);
          break;
      }

    }
  }
}

void hubClient() {
  if (!hubServerFlag) {
    instruction tempInst; // create temporary instruction to store received can message
    float fData;
    uint8_t exponent;
    if (hubBuffer.read(tempInst)) {
      switch (tempInst.data[1]) {
        case 15:
          //fData = read_lux();

          fData = 11.11;
          exponent = getExponent(fData);
          canWrite(0, 16, tempInst.data[2], exponent, floatToCan(fData, exponent));
          break;
        case 16:
          break;
      }
    }
  }
}

uint8_t getExponent(float fData) {

  if (static_cast<long>(fData * 100) < 65535) {
    return 100;
  } else {
    return 10;
  }
}

uint16_t floatToCan(float f, uint8_t e) {
  return static_cast<uint16_t>(f * e);
}

float canToFloat(uint8_t msb, uint8_t lsb, uint8_t e) {
  uint16_t fData = (msb << 8) | lsb;
  return (static_cast<float>(fData)) / e;
}
