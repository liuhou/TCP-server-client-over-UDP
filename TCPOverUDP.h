#ifndef TCPOVERUDP_H
#define TCPOVERUDP_H
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
//#include <netdb.h>
#include <sstream>
#include <vector>
#include <sys/time.h>
//#include <pthread.h>
//#include <thread>
#include <stdlib.h>
#include <stdint.h>
//#include <fcntl.h>
#include <stdio.h>
#include <math.h>

#define TIMEOUT 6
#define MAX_SEQ 30720

class Packet{
private:

    uint16_t seqNumber;
    uint16_t ackNumber;
    uint16_t window;
    bool A;
    bool S;
    bool F;
    std::string payload;
public:
    static const int HEADER_LENGTH = 8; // number of bytes for header only

    Packet();
    Packet(uint16_t seq, uint16_t ack, uint16_t win, bool a, bool s, bool f, std::string &str);
    Packet(uint16_t seq, uint16_t ack, uint16_t win, bool a, bool s, bool f);
    void setSeqNumber(uint16_t seq);
    uint16_t getSeqNumber();
    void setAckNumber(uint16_t ack);
    uint16_t getAckNumber();
    void setAck(bool a);
    bool getAck();
    void setSyn(bool s);
    bool getSyn();
    void setFin(bool f);
    bool getFin();
    void setPayLoad(std::string &pl);
    std::string getPayLoad();
    std::string encode();
    void consume(std::string encoded);
    void printHeader();
};

class Segment{
private:
    int duplicateAck;
    uint16_t seqNum;
    uint16_t ackNum;
    //double time;
    //bool isTrans;
    //bool isRetrans;
    bool isAcked;
    double sendTime;
    
public:
    Packet packet;
    Segment();
    void setPacket(Packet &p);
    void setSeqNum(uint16_t s);
    void setSeqNum();
    void setAckNum(uint16_t a);
    void setAckNum();
    void addDuplicateAck();
    void resetDuplicateAck();
    void setAck(bool a);
    void setSendTime(double t);
    double getSendTime();
    uint16_t getSeqNum();
    uint16_t getAckNum();
    uint16_t getSupposedAck();
    int getDuplicateAck();
    bool getAck();
};

class FileReader{
private:
    int chunkSize = 1024;
    int lastChunkSize = 0;
    std::string filename;
    std::ifstream filestream;
    std::string topString;
    int chunk = 0;
    int cursor = 0;
    
public:
    FileReader();
    void setChunkSize(int cs);
    int getChunkSize();
    int getLastChunkSize();
    std::string getFileName();
    int getChunk();
    int getCursor();
    void read(std::string &fn);
    std::string getTop();
    std::string pop();
    bool hasNext();
};

class SendBuffer{
private:
    std::vector<Segment> buffer;
    uint16_t startSeqNum;
    uint16_t cwnd;
    uint16_t endSeqNum;
    
public:
    SendBuffer();
    double RTO, SRTT, DEVRTT;
    void setStart(uint16_t s);
    uint16_t getEnd();
    uint16_t getStart();
    bool canContain(uint16_t size);
    int push(Segment &segment);
    Segment* findSegment(uint16_t seq);
    int ack(uint16_t seq, double time);
    bool timeout(double time, uint16_t &seq); //if timeout, set seq to be the packet that timeout
    // you should first timeout(current time), then use fubdSegment(seq) to find the segment
    Segment* nextTimeout();
    bool isEmpty();
};

class RcvBuffer{
private:
    std::vector<Segment> buffer;
    uint16_t window;
    uint16_t cumAck;
    std::ofstream output;
    
public:
    RcvBuffer();
    void setWindow(uint16_t w);
    uint16_t getWindow();
    uint16_t getCumAck();
    void setCumAck(uint16_t a);
    void openFile(std::string filename);
    int insert(Segment &segment);
    //void writeToFile();
    void closeFile();
};
#endif /* TCPOVERUDP_H*/
