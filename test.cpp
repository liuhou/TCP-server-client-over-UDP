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
    string t = "123\0 123\0";
    cout << t.size() << endl;
}
