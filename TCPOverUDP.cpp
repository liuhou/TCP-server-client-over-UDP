#include "TCPOverUDP.h"
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
Packet::Packet(uint16_t seq, uint16_t ack, uint16_t win, bool a, bool s, bool f){
    seqNumber = seq;
    ackNumber = ack;
    window = win;
    A = a;
    S = s;
    F = f;
    payload = "";
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

void Packet::consume(std::string encoded) {
    unsigned char high, low;
    high = encoded[0]; low = encoded[1];
    seqNumber = (high << 8) + low;

    high = encoded[2]; low = encoded[3];
    ackNumber = (high << 8) + low;

    high = encoded[4]; low = encoded[5];
    window = (high << 8) + low;

    low = encoded[7];
    A = low & 4;
    S = low & 2;
    F = low & 1;
    payload = "";
    if (encoded.size() > 8) 
        payload = encoded.substr(8);
}


std::string Packet::encode() {
    std::string encoded;
    unsigned char full = 255;
    encoded.push_back(seqNumber >> 8);
    encoded.push_back(seqNumber & full);
    encoded.push_back(ackNumber >> 8);
    encoded.push_back(ackNumber & full);
    encoded.push_back(window >> 8);
    encoded.push_back(window & full);
    encoded.push_back(0);
    encoded.push_back(F + (S << 1) + (A << 2));

    return encoded + payload;
}

void Packet::printHeader() {
    std::cout << seqNumber << '\n'
              << ackNumber << '\n'
              << window << '\n'
              << A << S << F << '\n'
              << payload << std::endl;
}

Segment::Segment(){
    isAcked = false;
    duplicateAck = 0;
    seqNum = 0;
    ackNum = 0;
    sendTime = 0;
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
void Segment::setSendTime(double t){
    sendTime = t;
}
double Segment::getSendTime(){
    return sendTime;
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
    filestream.open(filename, std::ifstream::in | std::ifstream::binary);
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
    cwnd = 10000;
    endSeqNum = 0;
    RTO = 1;
    SRTT = 0.5;
    DEVRTT = 0.25;
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
int SendBuffer::ack(uint16_t ackNum, double time){// return 0: nomal ack; return 1: retransmit; return 2: duplicate ack
    bool found = false;
    std::vector<Segment>::iterator it = buffer.begin();
    for(; it != buffer.end(); it++){
        if((*it).getSupposedAck() == ackNum){
            (*it).setAck(true);
            found = true;
            
            //deal with RTO
            /*double sampleRTT = time - it->getSendTime();
            SRTT = 0.875 * SRTT + 0.125 * sampleRTT;
            DEVRTT = 0.75 * DEVRTT + 0.25 * fabs(sampleRTT - DEVRTT);
            RTO = SRTT + 4 * DEVRTT;*/
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
bool SendBuffer::timeout(double time, uint16_t &seq){
    for(std::vector<Segment>::iterator it = buffer.begin(); it != buffer.end(); it++){
        if((time - it->getSendTime() > RTO) && (!it->getAck())){
            RTO = 2 * RTO;
            seq = it->getSeqNum();
            return true;
        }
    }
    return false;
}
Segment* SendBuffer::nextTimeout(){
    if(buffer.empty()){
        return NULL;
    }
    Segment* result = &buffer.front();
    double minTime = buffer.front().getSendTime();
    for(std::vector<Segment>::iterator it = buffer.begin(); it != buffer.end(); it++){
        if(it->getSendTime() < minTime){
            result = &(*it);
            minTime = it->getSendTime();
        }
    }
    return result;
}
bool SendBuffer::isEmpty(){
    return buffer.empty();
}

RcvBuffer::RcvBuffer(){
    window = 15360;
    cumAck = 0;
}
void RcvBuffer::setWindow(uint16_t w){
    window = w;
}
uint16_t RcvBuffer::getWindow(){
    return window;
}
uint16_t RcvBuffer::getCumAck(){
    return cumAck;
}
void RcvBuffer::setCumAck(uint16_t a){
    cumAck = a;
}
void RcvBuffer::openFile(std::string filename){
    output.open(filename, std::ofstream::out | std::ofstream::binary);
}
int RcvBuffer::insert(Segment &segment){ //return 0: inserted and popped out; return 1: discard; return 2: inserted
    std::vector<Segment>::iterator it = buffer.begin();
    if(segment.getSeqNum() == cumAck){
        /*for(; it != buffer.end() && (*it).getSupposedAck() <= cumAck; it++){
            output<<(*it).packet.getPayLoad();
        }*/
        it = buffer.insert(it, segment);
        std::vector<Segment>::iterator itnext = it + 1;
        while(itnext != buffer.end()){
            if(it->getSupposedAck() != itnext->getSeqNum()){
                break;
            }
            output<<(*it).packet.getPayLoad();
            it++;
            itnext++;
        }
        output<<(*it).packet.getPayLoad();
        cumAck = (*it).getSupposedAck();
        buffer.erase(buffer.begin(), itnext);
        return 0;
    }
    if(segment.getSeqNum() < cumAck){
        if(cumAck + window > MAX_SEQ && segment.getSeqNum() < (cumAck + window)% MAX_SEQ){
            for(; it!=buffer.end() && (it->getSeqNum()>cumAck || it->getSeqNum()<segment.getSeqNum());it++){
                if(segment.getSeqNum() == it->getSeqNum()){
                return 1;
                }
                it = buffer.insert(it, segment);
                return 2;
            }
        }else{
            return 1; //discard the segment
        }
    }
    if(segment.getSeqNum() - cumAck >= window){
        return 1; //discard the segment
    }
    //seq > cumAck && seq < window + cumAck
    if(buffer.empty()){
        it = buffer.insert(it, segment);
        return 2;
    }
    for(it = buffer.begin(); it != buffer.end() && segment.getSeqNum() < it->getSeqNum(); it++){
        if(segment.getSeqNum() == it->getSeqNum()){
            return 1;
        }
    }
    it = buffer.insert(it, segment);
    return 2;
}