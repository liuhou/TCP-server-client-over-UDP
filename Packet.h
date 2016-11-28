#ifndef PACKET_H
#define PACKET_H
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <queue>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <fcntl.h>
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
    Packet();
    Packet(uint16_t seq, uint16_t ack, uint16_t win, bool a, bool s, bool f, std::string &str);
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
    void consume(char wire[]);
};

class Segment{
private:
    int duplicateAck;
    uint16_t seqNum;
    uint16_t ackNum;
    double time;
    bool isTrans;
    bool isRetrans;
    bool isAcked;
    
public:
    Segment();
    
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
#endif /* PACKET_H */

