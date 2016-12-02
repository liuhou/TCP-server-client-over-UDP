#include "TCPOverUDP.h"
#include <string>
#include <iostream>

using namespace std;

int main() {
    // test Packet functionality
    Packet packet(10000, 20000, 30000, 1, 0, 1, "plasdfasdf");
    Packet p2;
    p2.consume(packet.encode());
    p2.printHeader();

}
