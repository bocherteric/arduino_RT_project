#ifndef NODES_CONTAINER_H
#define NODES_CONTAINER_H

class NodesContainer {

    static const uint8_t maxNodes = 3;
    uint8_t nodesContainer [maxNodes];
    uint8_t index = 0;
    uint8_t nextNode = 0;


  public:

    bool newNode(uint8_t node);
    bool addNode(uint8_t node);
    bool nodeAvailable(uint8_t node);
    bool checkIfFirstNode(uint8_t myHwId);
    uint8_t setNextNode(uint8_t myHwId);
    void reset();
      

    uint8_t getNextNode() {
      return nextNode;
    };

    uint8_t getNode(uint8_t i){
      return nodesContainer[i];
    };

    uint8_t numberOfNodes() {
      return index;
    };

};

 inline uint8_t NodesContainer::setNextNode(uint8_t myHwId) {
      bool nextNodeFound = false;
      uint8_t temp = myHwId;
      do {
        temp++;
        if (temp == maxNodes + 1)
          temp = 1;
        for (uint8_t i = 0; i < index; i++) {
          if (temp == nodesContainer[i]) {
            nextNode = temp;
            nextNodeFound = true;
          }
        }
      } while (!nextNodeFound);
      return nextNode;
    }
    
 inline  bool NodesContainer::checkIfFirstNode(uint8_t myHwId) {
      for (uint8_t i = 0; i < index; i++) {
        if (myHwId > nodesContainer[i])
          return false;
      }
      return true;
    }

inline bool NodesContainer::addNode(uint8_t node) {
      if (index < maxNodes) {
        nodesContainer[index] = node;
        index++;
        return true;
      }
      return false;
    };

inline bool NodesContainer::newNode(uint8_t node) {
      for (uint8_t i = 0; i < index; i++) {
        if (nodesContainer[i] == node)
          return false;
      }
      return true;
    }; 

inline void NodesContainer::reset(){
  for (uint8_t i = 0; i < index; i++) {
        nodesContainer[i]=0;
      }
  index = 0;
  nextNode = 0;
}

inline bool NodesContainer::nodeAvailable(uint8_t node){
  for (uint8_t i = 0; i < index; i++) {
        if (nodesContainer[i] == node)
          return true;
      }
      return false;
}

#endif
