#include "TCPOverUDP.h"
#include <string>
#include <iostream>

using namespace std;

int main() {
    // test Packet functionality
    Packet packet(100, 200, 300, 1, 0, 1);
    Packet p2;
    string encoded(packet.encode().c_str(), 8);
    p2.consume(encoded);
    p2.printHeader();

    char a = 200;
    char b = -55;
    unsigned char ua = a;
    unsigned char ub = b;
    string as;
    as.push_back(a);
    as.push_back(ua);
    as.push_back(b);
    as.push_back(ub);
    cout << (int) ((unsigned char)as[0]) << ' ' << int(as[1]) << endl;
    cout << int(as[2]) << ' ' << int(as[3]) << endl;
    cout << int(as.c_str()[0]) << ' ' << int(as.c_str()[1]) << endl;
    cout << int(as[2]) << ' ' << int(as[3]) << endl;
    string bs(as.c_str());
    cout << int(bs[2]) << ' ' << int(bs[3]) << endl;
}
