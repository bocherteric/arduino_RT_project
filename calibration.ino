

enum States {DISTURBANCE,READY, TURN, MY_TURN,READY2,OVER, RESET, NOT_MY_TURN};
States cs_calib=DISTURBANCE;

void calibration_function(){
    switch(cs_calib){
      
      case DISTURBANCE:
        analogWrite(analogOutPin,0);
        node.o=read_lux();
        Serial.print("O value: ");
        Serial.println(node.o);
        canWrite(node.myHwId,4);//tell everybody I'm ready with my disturbance
        cs_calib=READY;
      break;

      case READY://wait until everybody has said they are ready with their disturbance;
       if(Ready==nodesCont.numberOfNodes()){
        Ready=0;
        cs_calib=TURN;
        }
      break;

      case TURN:
        if(node.myHwId==index){//is it my turn?
          nb_turn+=1;//To track the number of turns of calibration 
          if (nb_turn>1){// meaning, if it's the second time I do it ==> loop completed, the all calibration is done
            calibration=0;
            calibrationDone=1;
            canWrite(node.myHwId,5);//Tell everybody the all calibration is done
            break;
          }
          else{
            cs_calib=MY_TURN;
          }
       }else{
         cs_calib=NOT_MY_TURN;
       }
       break;     
       
      case MY_TURN:
        if(change_lux){//makes sure it changes the lux only once before computing the gain
          change_lux=0;
          analogWrite(analogOutPin, p*2.55);//change the value of the lux
          prevTime=millis();
        }   
        if(millis()-prevTime>turnOnLightTime){//wait until the light is on (~300ms)  
          g+=(read_lux()-node.o)/p;//compute gain
          canWrite(node.myHwId,6);//Tell the others to compute their gain
          cs_calib=READY2;
        }
      break;
      
      case READY2://wait until everybody has compute their gain
        if(Ready==nodesCont.numberOfNodes()){
          Ready=0;
          cs_calib=OVER;
        }
       break;

      case OVER:
        if(p>=100){
          node.maxLux=read_lux();
          Serial.print("MAX LUX ");
          Serial.println(node.maxLux);
          canWrite(node.myHwId,7);//Send to everybody it's over!
          node.k[index-1]=g/5;//compute final gain
        Serial.println("Values of the cross gain");
        for(uint8_t counter=0; counter<nodesCont.numberOfNodes()+1; counter++){
          Serial.println(node.k[counter]);
        }
          g=0;//clear gain for next calibration
          p=20;
          cs_calib=RESET;
        }
        else if (p<100){
          p+=20; //increase the lux 
          change_lux=1;
          cs_calib=MY_TURN; 
        }
       break;
     
       case RESET:
        analogWrite(analogOutPin,0);//turn off the light
        index=nodesCont.getNextNode(); 
        canWrite(node.myHwId,8,0,index); //Send to everybody who's turn it is
        cs_calib=TURN;
      break;

      case NOT_MY_TURN: //Loop waiting between compute_gain, over, and my turn
      if(compute_gain){
        compute_gain=0;
        g+=(read_lux()-node.o)/p;//compute gain 
        p+=20;//increase the value of lux
        canWrite(node.myHwId,4,index);//Send I'm ready to the index
      }
      if(over){
        over=0;
        node.k[index-1]=g/5; //store total gain at the row corresponding to the index
        g=0;//clear gain
        p=20;
        Serial.println("Values of the cross gain");
        for(uint8_t counter=0; counter<nodesCont.numberOfNodes()+1; counter++){
          Serial.println(node.k[counter]);
        }
      }
      if(node.myHwId == index)
        cs_calib=TURN;
      break;
        
      }
    
  }

void calibration_interface(){
  
  //read
  
  instruction tempInst;
  if (caliBuffer.read(tempInst)) {
    switch (tempInst.data[1]) { //purpose byte of can message
      case 4:
        Ready+=1;
      break;
      case 5:
        calibration=0;
        calibrationDone=1;
      break;

      case 6:
        compute_gain=1;
      break;

      case 7:
       over=1;
      break;

      case 8:
        index=tempInst.data[0];
       break;
       
      
    }
  }
} 
  
  


  
  
