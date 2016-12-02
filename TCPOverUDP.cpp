#include "TCPOverUDP.h"
#include <string>

#include <iostream>

void Packet::consume(std::string encoded) {
    unsigned char high, low;
    high = encoded[0]; low = encoded[1];
    sequenceNumber = (high << 8) + low;

    high = encoded[2]; low = encoded[3];
    ackNumber = (high << 8) + low;

    high = encoded[4]; low = encoded[5];
    window = (high << 8) + low;

    low = encoded[7];
    bitA = low & 4;
    bitS = low & 2;
    bitF = low & 1;
    payload = encoded.substr(8);
}


std::string Packet::encode() {
    std::string encoded;
    unsigned char full = 255;
    encoded.push_back(sequenceNumber >> 8);
    encoded.push_back(sequenceNumber & full);
    encoded.push_back(ackNumber >> 8);
    encoded.push_back(ackNumber & full);
    encoded.push_back(window >> 8);
    encoded.push_back(window & full);
    encoded.push_back(0);
    encoded.push_back(bitF + (bitS << 1) + (bitA << 2));

    return encoded + payload;
}

void Packet::printHeader() {
    std::cout << sequenceNumber << '\n'
              << ackNumber << '\n'
              << window << '\n'
              << bitA << bitS << bitF << '\n'
              << payload << std::endl;
};
