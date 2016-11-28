#include "Packet.h"
Packet::Packet(){
    seqNumber = 0;
    ackNumber = 0;
    window = 0;
    A = false;
    S = false;
    F = false;
    payload = "";
}
Packet::Packet(uint16_t seq, uint16_t ack, uint16_t win, bool a, bool s, bool f, std::string &str){
    seqNumber = seq;
    ackNumber = ack;
    window = win;
    A = a;
    S = s;
    F = f;
    payload = str;
}
void Packet::setSeqNumber(uint16_t seq){
    seqNumber = seq;
}
uint16_t Packet::getSeqNumber(){
    return seqNumber;
}
void Packet::setAckNumber(uint16_t ack){
    ackNumber = ack;
}
uint16_t Packet::getAckNumber(){
    return ackNumber;
}
void Packet::setAck(bool a){
    A = a;
}
bool Packet::getAck(){
    return A;
}
void Packet::setSyn(bool s){
    S = s;
}
bool Packet::getSyn(){
    return S;
}
void Packet::setFin(bool f){
    F = f;
}
bool Packet::getFin(){
    return F;
}
void Packet::setPayLoad(std::string &pl){
    payload = pl;
}
std::string Packet::getPayLoad(){
    return payload;
}
std::string Packet::encode(){
    char sLeft = seqNumber >> 8;
    char sRight = seqNumber & 255;
    char aLeft = ackNumber >> 8;
    char aRight = ackNumber & 255;
    char wLeft = window >> 8;
    char wRight = window & 255;
    char left = 0;
    char right = F + (S << 1) + (A << 2);
    std::string str = "";
    str.push_back(sLeft);
    str.push_back(sRight);
    str.push_back(aLeft);
    str.push_back(aRight);
    str.push_back(wLeft);
    str.push_back(wRight);
    str.push_back(left);
    str.push_back(right);
    str = str + payload;
    return str;
}
void Packet::consume(char wire[]){
    char sL = wire[0];
    char sR = wire[1];
    seqNumber = (sL << 8) + sR;
    char aL = wire[2];
    char aR = wire[3];
    ackNumber = (aL << 8) + aR;
    char wL = wire[4];
    char wR = wire[5];
    window = (wL << 8) + wR;
    char right = wire[7];
    A = ((right & 4)>>2);
    S = (right & 2) >> 1;
    F = (right & 1);
    payload = std::string(reinterpret_cast<char*>(wire + 8));
}
FileReader::FileReader(){
    chunkSize = 1024; //this is the payload length of a packet;
    lastChunkSize = 0;
    filename = "";
    topString = "";
    chunk = 0;
    cursor = 0;  
}
void FileReader::setChunkSize(int cs){
    chunkSize = cs;
}
int FileReader::getChunkSize(){
    return chunkSize;
}
std::string FileReader::getFileName(){
    return filename;
}
int FileReader::getChunk(){
    return chunk;
}
int FileReader::getCursor(){
    return cursor;
}
void FileReader::read(std::string &fn){
    filename = fn;
    filestream.open(filename);
    if(!filestream.good()){
        std::cerr << "Error: cannot open file: " + filename <<std::endl;
        filestream.clear();
        filestream.close();
        return;
    }
    struct stat filestatus;
    stat(filename.c_str(), &filestatus);
    int length = filestatus.st_size;
    chunk = length / chunkSize;
    lastChunkSize = length % chunkSize;
    chunk = (lastChunkSize == 0) ? chunk : chunk + 1;
}
std::string FileReader::getTop(){
    int size = (cursor < chunk) ? chunkSize : lastChunkSize;
    char *c = (char*)malloc(sizeof(char) * (size + 1));
    memset(c, '\0', size + 1);
    filestream.read(c, size);
    topString = std::string(c);
    return topString;
}
std::string FileReader::pop(){
    std::string result = getTop();
    cursor++;
    return result;
}
bool FileReader::hasNext(){
    return cursor < chunk;
}
