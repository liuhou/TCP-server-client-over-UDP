#include <cstdlib>
#include <fstream>
#include "Packet.h"
using namespace std;
#define BACKLOG 10     // how many pending connections queue will hold
#define PORT "4000"
#define BUFSIZE 1032
#define LISTEN 0
#define SYNACK_SENT 1
#define ESTAB 2
/*
 * 
 */
/*void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}*/
int main(int argc, char** argv) {
    int sockfd, new_fd;
    int status = LISTEN;
    struct addrinfo hints;
    struct sockaddr_storage their_addr; //connectors' address information
    struct addrinfo *servinfo, *p;
    socklen_t addr_len;
    //char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_flags = AI_PASSIVE;
    
    if((rv = getaddrinfo("10.0.0.1", PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    //loop through all the results and bing the first we can
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if(p == NULL){
        std::fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    printf("server: waiting to receive...\n");
    addr_len = sizeof their_addr;
    char *buf;
    while(true){
        
        int numbytes = 0;
        memset(buf, 0, BUFSIZE * sizeof(unsigned char));
        if((numbytes = recvfrom(sockfd, buf, BUFSIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("recvfrom");
            continue;
        }
        Packet packet;
        packet.consume(buf);
        if(packet.getSyn() && status == LISTEN && !packet.getAck() && !packet.getFin()){
            std::cout<<"Receiving packet "<<packet.getSeqNumber()<<std::endl;
            uint16_t initialSeq = rand() % MAX_SEQ;
            uint16_t initialAck = packet.getSeqNumber() + 1;
            packet.setAck(true);
            packet.setAckNumber(initialAck);
            packet.setSyn(true);
            packet.setFin(false);
            std::string payload = "";
            packet.setPayLoad(payload);
            std::string sendPacket = packet.encode();
            if(sendto(sockfd, sendPacket.c_str(), sendPacket.size(), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1){
                perror("sendto");
                exit(1);
            }
            std::cout<<"Send packet "<<initialSeq<<" SYN"<<std::endl;
            status = SYNACK_SENT;
            
        }else if(packet.getAck()){
            if(status == SYNACK_SENT && (!packet.getSyn()) && (!packet.getFin())){
                std::cout<<"Receiving packet "<<packet.getAckNumber()<<std::endl;
                uint16_t seqNum = packet.getSeqNumber();
                
                status = ESTAB;
            }
        }
        
        
        break;
    }
    return 0;
}

