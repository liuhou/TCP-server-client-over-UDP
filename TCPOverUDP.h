#ifndef _TCPOVERUDP_H
#define _TCPOVERUDP_H

#include <string>

class Packet {
private:
    unsigned sequenceNumber;
    unsigned ackNumber;
    unsigned window;
    bool bitA; // indicates that the value of ackNumber is valid
    bool bitS;
    bool bitF;
    std::string payload; // data
public:
    Packet() = default;
    Packet(int sn, int an, int win,
           bool a, bool s, bool f,
           std::string pl)
        : sequenceNumber(sn)
        , ackNumber(an)
        , window(win)
        , bitA(a)
        , bitS(s)
        , bitF(f)
        , payload(pl) {  }

    std::string encode();
    void consume(std::string encoded);
    void printHeader();
};

#endif
