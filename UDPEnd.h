#ifndef UDPEND
#define UDPEND

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

using namespace std;

class UDPEnd {

  private:
  int s; //our socket
  struct sockaddr_in a; //our full address
  struct sockaddr_in oa; //their full address
  socklen_t len = sizeof(oa); //size of their full address

  public:
  bool init(); //returns false if any failure was detected
  char* listen(); //obtains a packet from the socket

};

#endif
