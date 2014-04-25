#include "packet.h"

#include <string.h>
#include <iostream>
#include <cstdlib>

  //Constructor
  Packet::Packet () {
	sequenceNum = 0;
	checkSum = 0;
	ackNack = 0;
        dataBuff[121];
  }
  Packet::Packet (int sn, const char db[122]){
    sequenceNum = sn % 32;
	std::cout << "db: " << db << std::endl;
    strcpy(dataBuff, db);
    checkSum = generateCheckSum();
    ackNack =0;
  }
  //Setter Methods
  void Packet::setSequenceNum(int sn){
     sequenceNum = sn;
  }
 
  void Packet::setCheckSum(int cs){
     checkSum = cs;
  }
 
  void Packet::setAckNack(int an){
     ackNack = an;
  }

  void Packet::loadDataBuffer(char* data){
    //Jakes load buffer code goes here
    strcpy(dataBuff, data);
  }
  char* Packet::getDataBuffer() {
    return dataBuff;
  }
  //Attach header to the data array
  char* Packet::str(){
    std::string tempStr(dataBuff);
    std::string packetString;
    std::string csStr;
	std::string sns;

    csStr = std::to_string((long long int)checkSum);
    while(csStr.length() < 5) csStr += '0';

	sns = std::to_string((long long int)sequenceNum);
	std::cout << "sns length: " << sns.length() << std::endl;
	if(sns.length() < 2) sns.insert(0, 1, '0');

    packetString = sns + csStr + std::to_string((long long int)ackNack) + tempStr;
    strcpy(packet, packetString.c_str());
    return packet;
  }
  //Getter Methods
  int Packet::getSequenceNum(){
    return sequenceNum;
  }
 
  int Packet::getCheckSum(){
    return checkSum;
  }

  int Packet::getAckNack(){
    return ackNack;
  }
  bool Packet::chksm() {
    return (checkSum) == generateCheckSum();
  }
  int Packet::generateCheckSum() {
    int cs = 0;
    if(dataBuff == NULL){
      return -1;
    }
    
    for(int x = 0; x < sizeof(dataBuff); x++) {
      if(dataBuff[x] == '\0') {
        x = sizeof(dataBuff);
        break;
      }
      cs += dataBuff[x];
    }

    if(cs <= 9999) cs *= 10;

    if(cs > 0) return cs;
    return -1;
  }
