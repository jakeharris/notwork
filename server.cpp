#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "packet.h"

#define USAGE "Usage:\r\nc \r\n[tux machine number] \r\n[probability of packet corruption in int form] \r\n[probability of packet loss in int form] \r\n[probability of packet delay] \r\n[length of delay in ms] \r\n"
#define PORT 10038
#define PAKSIZE 512
#define ACK 0
#define NAK 1
#define BUFSIZE 505
#define FILENAME "Testfile"
#define TIMEOUT 100 //in ms
#define WIN_SIZE 16

using namespace std;

bool isvpack(unsigned char * p);
bool init(int argc, char** argv);
bool loadFile();
bool getFile();
bool sendFile();
bool isAck();
void handleAck();
void handleNak(int& x);
bool* gremlin(Packet * pack, int corruptProb, int lossProb, int delayProb);
Packet createPacket(int index);
void loadWindow();
bool sendPacket();
bool getGet();

struct sockaddr_in a;
struct sockaddr_in ca;
socklen_t calen;
int rlen;
int s;
bool ack;
string fstr;
char * file;
int probCorrupt;
int probLoss;
int probDelay;
int delayT;
Packet p;
Packet window[16];
int length;
bool dropPck;
bool delayPck;
int toms;
unsigned char b[BUFSIZE];
int base;

int main(int argc, char** argv) {
  
  if(!init(argc, argv)) return -1;
  
  sendFile();
  
  return 0;
}

bool init(int argc, char** argv){

  if(argc != 6) { 
    cout << USAGE << endl;
    return false;
  }

  char * probCorruptStr = argv[2];
  probCorrupt = boost::lexical_cast<int>(probCorruptStr);
  char * probLossStr = argv[3];
  probLoss = boost::lexical_cast<int>(probLossStr);
  char * probDelayStr = argv[4];
  probDelay = boost::lexical_cast<int>(probDelayStr);
  char* delayTStr = argv[5];
  delayT = boost::lexical_cast<int>(delayTStr);
  
  struct timeval timeout;
  timeout.tv_usec = TIMEOUT * 1000;
  toms = TIMEOUT;

  /* Create our socket. */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    cout << "Socket creation failed. (socket s)" << endl;
    return 0;
  }

  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

  /* 
   * Bind our socket to an IP (whatever the computer decides) 
   * and a specified port. 
   *
   */

  if (!loadFile()) {
    cout << "Loading file failed. (filename FILENAME)" << endl;
    return false; 
  }
  
  memset((char *)&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  a.sin_port = htons(PORT);

  if (bind(s, (struct sockaddr *)&a, sizeof(a)) < 0) {
    cout << "Socket binding failed. (socket s, address a)" << endl;
    return 0;
  }
  
  fstr = string(file);
  cout << "File: " << endl << fstr << endl;

  base = 0;
  dropPck = false;
  calen = sizeof(ca);

  getGet();
  
  return true;
}

bool getGet(){
	unsigned char packet[PAKSIZE + 1];
	rlen = recvfrom(s, packet, PAKSIZE, 0, (struct sockaddr *)&ca, &calen);
	return (rlen > 0) ? true : false;
}

bool isvpack(unsigned char * p) {
  cout << endl << "=== IS VALID PACKET TESTING" << endl;

  char * sns = new char[2];
  memcpy(sns, &p[0], 1);
  sns[1] = '\0';

  char * css = new char[7];
  memcpy(css, &p[1], 6);
  css[6] = '\0';
      
  char * db = new char[121 + 1];
  memcpy(db, &p[2], 121);
  db[121] = '\0';

  cout << "Seq. num: " << sns << endl;
  cout << "Checksum: " << css << endl;
  cout << "Message: " << db << endl;

  int sn = boost::lexical_cast<int>(sns);
  int cs = boost::lexical_cast<int>(css);

  Packet pk (0, db);
  pk.setSequenceNum(sn);

  // change to validate based on checksum and sequence number

  if(sn == 0) return false; //doesn't matter, only for the old version (netwark)
  if(cs != pk.generateCheckSum()) return false;
  return true;
}

bool getFile(){

  /* Loop forever, waiting for messages from a client. */
  cout << "Waiting on port " << PORT << "..." << endl;

  ofstream file("Dumpfile");

  for (;;) {
    unsigned char packet[PAKSIZE + 1];
    unsigned char dataPull[PAKSIZE - 7 + 1];
    rlen = recvfrom(s, packet, PAKSIZE, 0, (struct sockaddr *)&ca, &calen);

    for(int x = 0; x < PAKSIZE - 7; x++) {
      dataPull[x] = packet[x + 7];
    }
    dataPull[PAKSIZE - 7] = '\0';
    packet[PAKSIZE] = '\0';
    if (rlen > 0) {
      char * css = new char[6];
      memcpy(css, &packet[1], 5);
      css[5] = '\0';
      cout << endl << endl << "=== RECEIPT" << endl;
      cout << "Seq. num: " << packet[0] << endl;
      cout << "Checksum: " << css << endl;
      cout << "Received message: " << dataPull << endl;
      cout << "Sent response: ";
      if(isvpack(packet)) {
        ack = ACK;
        //seqNum = (seqNum) ? false : true;
        file << dataPull;
      } else { 
        ack = NAK;
      }
      cout << ((ack == ACK) ? "ACK" : "NAK") << endl;
      Packet p (false, reinterpret_cast<const char *>(dataPull));
      p.setCheckSum(boost::lexical_cast<int>(css));
      p.setAckNack(ack);

      if(sendto(s, p.str(), PAKSIZE, 0, (struct sockaddr *)&ca, calen) < 0) {
        cout << "Acknowledgement failed. (socket s, acknowledgement message ack, client address ca, client address length calen)" << endl;
        return 0;
      }
      delete css;
    }
  }
  file.close();
  return true;
}

bool loadFile() {

  ifstream is (FILENAME, ifstream::binary);

  if(is) {
    is.seekg(0, is.end);
    length = is.tellg();
    is.seekg(0, is.beg);

    file = new char[length];

    cout << "Reading " << length << " characters..." << endl;
    is.read(file, length);

    if(!is) { cout << "File reading failed. (filename " << FILENAME << "). Only " << is.gcount() << " could be read."; return false; }
    is.close();
  }
  return true;
}

void loadWindow(){
	for(int i = base; i < base + WIN_SIZE; i++) {
		window[i-base] = createPacket(i);
		if(strlen(window[i-base].getDataBuffer()) < BUFSIZE - 1 && window[i-base].getDataBuffer()[0] != 'G') { 
			for(++i; i < base + WIN_SIZE; i++){
				window[i-base].loadDataBuffer("\0");
			}
			return;
		}
	}
}

bool sendFile() {
	/*Currently causes the program to only send the first 16 packets of file out
		requires additional code later to sendFile again with updated window*/
	base = 0;
	while(base * BUFSIZE < length) {
		loadWindow();

		cout << "packet " << base << ": " << window[0].str() << endl;

		for(int x = 0; x < WIN_SIZE; x++) {
			p = window[x];
			if(!sendPacket()) continue;
		}
		if(p.str()[0] == '\0') break;
		for(int x = 0; x < WIN_SIZE; x++) {
			if(recvfrom(s, b, BUFSIZE + 7, 0, (struct sockaddr *)&ca, &calen) < 0) {
				cout << "=== ACK TIMEOUT" << endl;
				x--;
				continue;
			}

			if(isAck()) { 
				handleAck();
			} else { 
				handleAck();
				//handleNak(x);
			}

			memset(b, 0, BUFSIZE);
		}
	}

	if(sendto(s, "\0", BUFSIZE, 0, (struct sockaddr *)&ca, sizeof(ca)) < 0) {
		cout << "Final package sending failed. (socket s, server address sa, message m)" << endl;
		return false;
	}
	return true;
}

Packet createPacket(int index){
    string mstr = fstr.substr(index * BUFSIZE, BUFSIZE);

	if(mstr.length() < BUFSIZE) {
		cout << "Null terminated mstr." << endl;
		mstr[length - (index * BUFSIZE)] = '\0';
    }
    return Packet (index, mstr.c_str());
}

bool sendPacket(){
	cout << endl;
    cout << "=== TRANSMISSION START" << endl;
    int pc = probCorrupt; int pl = probLoss; int pd = probDelay;
	bool* pckStatus = gremlin(&p, pc, pl, pd);

	dropPck = pckStatus[0];
	delayPck = pckStatus[1];

	if (dropPck == true) return false;
	if (delayPck == true) p.setAckNack(1);

    if(sendto(s, p.str(), BUFSIZE + 8, 0, (struct sockaddr *)&ca, sizeof(ca)) < 0) {
		cout << "Package sending failed. (socket s, server address sa, message m)" << endl;
		return false;
    }
    return true;
}
bool isAck() {
    cout << endl << "=== SERVER RESPONSE TEST" << endl;
    cout << "Data: " << b << endl;
    if(b[6] == '0') return true;
    else return false;
}
void handleAck() {
	int ack = boost::lexical_cast<int>(b);
	if(base < ack) base = ack;
	cout << "Window base: " << base << endl;
}
void handleNak(int& x) {

      char * sns = new char[2];
      memcpy(sns, &b[0], 1);
      sns[1] = '\0';

      char * css = new char[5];
      memcpy(css, &b[1], 5);
      
      char * db = new char[BUFSIZE + 1];
      memcpy(db, &b[2], BUFSIZE);
      db[BUFSIZE] = '\0';

      cout << "Sequence number: " << sns << endl;
      cout << "Checksum: " << css << endl;

      Packet pk (0, db);
      pk.setSequenceNum(boost::lexical_cast<int>(sns));
      pk.setCheckSum(boost::lexical_cast<int>(css));

      if(!pk.chksm()) x--; 
      else x = (x - 2 > 0) ? x - 2 : 0;
}
bool* gremlin(Packet * pack, int corruptProb, int lossProb, int delayProb){

  bool* packStatus = new bool[2];
  int r = rand() % 100;

  cout << "Corruption probability: " << corruptProb << endl;
  cout << "Random number: " << r << endl;

  packStatus[0] = false;
  packStatus[1] = false;

  if(r <= (lossProb)){
	packStatus[0] = true;
    cout << "Dropped!" << endl;
  }
  else if(r <= (delayProb)){
	  packStatus[1] = true;
	  cout << "Delayed!" << endl;
  }
  else if(r <= (corruptProb)){
    cout << "Corrupted!" << endl;
    pack->loadDataBuffer((char*)"GREMLIN LOL");
  }
  cout << "Seq. num: " << pack->getSequenceNum() << endl;
  cout << "Checksum: " << pack->getCheckSum() << endl;
  //cout << "Message: "  << pack->getDataBuffer() << endl;

  return packStatus;
}