#include "UDPEnd.h"

bool UDPEnd::init() {
  struct hostent *h;
  string hs = "131.204.14.192";
  short int port = 10038; /* Can be any port within 10038-10041, inclusive. */
  
  int s = 0;

  string m = string("Hello, server world! I'm gonna talk for a long long time and see if this works. Maybe \r\n")
      + "it'll properly tell me there are more bytes; maybe it won't. Either way I'll be proud of the work I've done. \r\n"
      + "Also, Patrick is gay.";

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    cout << "Socket creation failed. (socket s)" << endl;
    return false;
  }

  memset((char *)&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  a.sin_port = htons(0);

  if (::bind(s, (struct sockaddr *)&a, sizeof(a)) < 0){
    cout << "Socket binding failed. (socket s, address a)" << endl;
    return false;
  }

  memset((char *)&oa, 0, sizeof(oa));
  oa.sin_family = AF_INET;
  oa.sin_port = htons(port);
  inet_pton(AF_INET, hs.c_str(), &(oa.sin_addr));
 
  return true;
}
