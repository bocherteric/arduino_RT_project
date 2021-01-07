#ifndef CAN_FLOAT_BUFFER_H
# define CAN_FLOAT_BUFFER_H

struct instructionFloat{
  unsigned char data[6];
};


class CanFloatBuffer {

    static const int bufferSize = 2;
    instructionFloat fsmBuffer[bufferSize];

    int readIndex = 0;
    int writeIndex = 0;
    bool flagFull = false; // buffer full
    bool flagEmpty = true;
    

  public:
 
    bool write(instructionFloat &);
    bool read(instructionFloat &);

};

inline bool CanFloatBuffer::write(instructionFloat &instruct){
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

inline bool CanFloatBuffer::read(instructionFloat &instruct){
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
