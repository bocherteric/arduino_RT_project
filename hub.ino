


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
      Serial.println(" g enterd");
        switch (CR[1]) {

          case 'I':
            Serial.println(" I enterd");
            int tempInt = CR[2] - '0';
            if (tempInt == node.myHwId) {
              float_val = read_lux();
              char SR2[3];
              SR2[0] = CR[1];
              SR2[1] = CR[2];
              SR2[2] = float_val;
              
              Serial.write(SR2,3);
            } else {

              canWrite(0, 15, tempInt);

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
      if (hubBuffer.read(tempInst)) {
        switch (tempInst.data[1]) {
          case 16:
            char SR2[3];
            SR2[0] = "I";
            SR2[1] = tempInst.data[2];
            SR2[2] = tempInst.data[0];
            
            Serial.write(SR2,3);
            break;
        }

      }
    }
  }
  void hubClient() {
    if (!hubServerFlag) {
      instruction tempInst; // create temporary instruction to store received can message
      if (hubBuffer.read(tempInst)) {
        switch (tempInst.data[1]) {
          case 15:
            uint8_t dummyIlluminance = 10;
            canWrite(0, 16, tempInst.data[2], dummyIlluminance);
            break;
          case 16:
            break;
        }
      }
    }
  }
