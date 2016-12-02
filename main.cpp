#include "TCPOverUDP.h"

#include <cstdlib>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    string str = "I must finish this project quickly";
    Packet packet(1000, 3212, 1234, false, false, false, str);
    string encodeP1 = packet.encode();
    cout<<"encode length"<<encodeP1.size();

    Packet packet3;
    packet3.consume(encodeP1);
    cout <<"seq"<<packet3.getSeqNumber()<<"ack"<<packet3.getAckNumber()<<"bool"<<packet3.getSyn()<<packet3.getAck()<<packet3.getFin()<<endl;
    cout<<packet3.getPayLoad().size()<<endl;
    cout<<packet3.getPayLoad()<<endl;
    Packet packet2(1034, 1, 1024, false, false, false, str);
    Segment segment;
    segment.setPacket(packet);
    segment.setSeqNum();
    segment.setAckNum();
    cout << segment.getSupposedAck()<<endl;
    segment.addDuplicateAck();
    segment.addDuplicateAck();
    cout << segment.getDuplicateAck() << endl;
    cout << segment.getSeqNum() << endl;
    cout << segment.getAckNum() << endl;
    Segment segment2;
    SendBuffer sendbuff;
    sendbuff.push(segment);
    cout<<"buff.start"<<sendbuff.getStart()<<"buff.end" <<sendbuff.getEnd()<<endl;
    segment2.setPacket(packet2);
    cout << sendbuff.push(segment2)<<endl;
    cout<<"new end"<<sendbuff.getEnd()<<endl;
    cout<<(*sendbuff.findSegment(1034)).getSeqNum()<<endl;
    int rs = sendbuff.ack(1034,3);
    cout<<rs;
    uint16_t seq = 0;
    cout<<rs<<"space  "<<sendbuff.timeout(5, seq)<<endl;
    rs = sendbuff.ack(1034,3);
    
    rs = sendbuff.ack(1034,4);
    cout<<rs;
    rs = sendbuff.ack(1034,4.001);
    cout<<rs;
    rs = sendbuff.ack(1034,4.002);
    cout<<rs;
    return 0;
}

