#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#include <string>
#include <iostream>
class Packet{
  //Private variables
  private:
    int sequenceNum;
    int checkSum;
    int ackNack;
    char dataBuff[122];
    std::string tempStr;
    std::string packetString;
    char packet[129];
  //Public functions and variables
  public:
    void setSequenceNum(int sn);
    void setCheckSum(int cs);
    void setAckNack(int an);
    int getSequenceNum();
    int getCheckSum();
    int getAckNack();
    bool chksm();
    int generateCheckSum();
    void loadDataBuffer(char* data);
    char* getDataBuffer();
    Packet(int sn, const char db[122]);
    Packet(); //constructor
    char* str();
};
#endif
