#include "Packet.h"
Packet::Packet(){
    seqNumber = 0;
    ackNumber = 0;
    window =  15360;
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
    unsigned char sLeft = seqNumber >> 8;
    unsigned char sRight = seqNumber & 255;
    unsigned char aLeft = ackNumber >> 8;
    unsigned char aRight = ackNumber & 255;
    unsigned char wLeft = window >> 8;
    unsigned char wRight = window & 255;
    unsigned char left = 0;
    unsigned char right = F + (S << 1) + (A << 2);
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
void Packet::consume(char *wire){
    unsigned char sL = wire[0];
    unsigned char sR = wire[1];
    seqNumber = (uint16_t(sL) << 8) + uint16_t(sR);
    unsigned char aL = wire[2];
    unsigned char aR = wire[3];
    ackNumber = (uint16_t(aL) << 8) + uint16_t(aR);
    unsigned char wL = wire[4];
    unsigned char wR = wire[5];
    window = (uint16_t(wL) << 8) + uint16_t(wR);
    unsigned char right = wire[7];
    A = ((right & 4)>>2);
    S = (right & 2) >> 1;
    F = (right & 1);
    payload = "";

    //payload = std::string(reinterpret_cast<char*>(wire + 8));
}
Segment::Segment(){
    isAcked = false;
    duplicateAck = 0;
    seqNum = 0;
    ackNum = 0;
}
void Segment::setPacket(Packet &p){
    packet = p;
    setSeqNum();
    setAckNum();
}
void Segment::setSeqNum(uint16_t s){
    seqNum = s;
}
void Segment::setSeqNum(){
    seqNum = packet.getSeqNumber();
}
void Segment::setAckNum(uint16_t a){
    ackNum = a;
}
void Segment::setAckNum(){
    ackNum = packet.getAckNumber();
}
void Segment::addDuplicateAck(){
    duplicateAck++;
}
void Segment::resetDuplicateAck(){
    duplicateAck = 0;
}
void Segment::setAck(bool a){
    isAcked = a;
}
uint16_t Segment::getSeqNum(){
    return seqNum;
}
uint16_t Segment::getAckNum(){
    return ackNum;
}
uint16_t Segment::getSupposedAck(){
    return (uint32_t(seqNum) + uint32_t((packet.getPayLoad()).size()))%MAX_SEQ;
}
bool Segment::getAck(){
    return isAcked;
}
int Segment::getDuplicateAck(){
    return duplicateAck;
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
void FileReader::read(std::string &fn){ //load the file
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
std::string FileReader::getTop(){   //return a chunk on the top
    int size = (cursor < chunk) ? chunkSize : lastChunkSize;
    char *c = (char*)malloc(sizeof(char) * (size + 1));
    memset(c, '\0', size + 1);
    filestream.read(c, size);
    topString = std::string(c);
    return topString;
}
std::string FileReader::pop(){ // return a chunk on the top and move the cursor to the next chunk
    std::string result = getTop();
    cursor++;
    return result;
}
bool FileReader::hasNext(){
    return cursor < chunk;
}

SendBuffer::SendBuffer(){
    startSeqNum = 0;
    cwnd = 1024;
    endSeqNum = 0;
}
void SendBuffer::setStart(uint16_t s){
    startSeqNum = s;
}
uint16_t SendBuffer::getStart(){
    return startSeqNum;
}
uint16_t SendBuffer::getEnd(){
    return endSeqNum;
}
bool SendBuffer::canContain(uint16_t size){
    if(buffer.size() == 0){
        return size <= cwnd;
    }
    uint16_t left = buffer.front().getSeqNum();
    uint16_t currentSize;
    if(left > endSeqNum){
        currentSize = MAX_SEQ - left + endSeqNum;
    }else{
        currentSize = endSeqNum - left;
    }
    return currentSize + size <= cwnd;
}
int SendBuffer::push(Segment& segment){
    uint16_t size = segment.packet.getPayLoad().size();
    if(canContain(size)){
        buffer.push_back(segment);
        endSeqNum = segment.getSupposedAck();
        startSeqNum = buffer.front().getSeqNum();
        return 0;
    }
    return -1;
}
Segment* SendBuffer::findSegment(uint16_t seq){
    for(std::vector<Segment>::iterator i = buffer.begin(); i != buffer.end(); i++){
        if((*i).getSeqNum() == seq){
            return &(*i);
        }
    }
    std::cout << "Cannot find a segment in buffer with seq "<<seq<<std::endl;
    return NULL;
}
int SendBuffer::ack(uint16_t ackNum){// return 0: nomal ack; return 1: retransmit; return 2: duplicate ack
    bool found = false;
    std::vector<Segment>::iterator it = buffer.begin();
    for(; it != buffer.end(); it++){
        if((*it).getSupposedAck() == ackNum){
            (*it).setAck(true);
            found = true;
            break;
        }
    }
    if(found){
        for(std::vector<Segment>::iterator i = buffer.begin(); i != it && i != buffer.end(); i++){
            (*i).setAck(true);
                      
        }
        startSeqNum = (*it).getSupposedAck(); 
        buffer.erase(buffer.begin(), it + 1);
    }else{
        it = buffer.begin();
        if((*it).getSeqNum() == ackNum){
            (*it).addDuplicateAck();
        }else{
            return -1; //error code: not a duplicate ack, not a normal ack
        }
        if((*it).getDuplicateAck() >= 3){
            return 1; // need to retransmit
        }else{
            return 2;//duplicate ack
        }
    }
    if(buffer.size() == 0){
        buffer.clear();
    }
    return 0;
}
