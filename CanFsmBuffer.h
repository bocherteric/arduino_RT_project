#ifndef CAN_FSM_BUFFER_H
# define CAN_FSM_BUFFER_H

//using instruction = unsigned char[3];

struct instruction{
  unsigned char data[3];
};


class CanFsmBuffer {

    static const int bufferSize = 3;
    instruction fsmBuffer[bufferSize];

    int readIndex = 0;
    int writeIndex = 0;
    bool flagFull = false; // buffer full
    bool flagEmpty = true;
    

  public:
  /*
    bool getFlagEmpty() {
      return flagEmpty;
    }

    bool getFlagFull() {
      return flagFull;
    }
  */
    bool write(instruction &);
    bool read(instruction &);

};

inline bool CanFsmBuffer::write(instruction &instruct){
  if(flagFull)
    return false;
  if(flagEmpty)
    flagEmpty = false;
  fsmBuffer[writeIndex] = instruct;
  writeIndex = ( ++writeIndex ) % bufferSize;
  if (writeIndex== readIndex)
    flagFull = true;
  return true;
}

inline bool CanFsmBuffer::read(instruction &instruct){
  if(flagEmpty)
    return false;
  if(flagFull)
    flagFull = false;
  instruct = fsmBuffer[readIndex];
  readIndex = (++readIndex) % bufferSize;
  if(readIndex == writeIndex)
    flagEmpty = true;
  return true;
}




#endif
