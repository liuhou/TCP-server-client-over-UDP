/* headers for socket programming */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

/* std headers */
#include <string>
#include <iostream>

/* user-defined headers*/
#include "server.h"
#include "TCPOverUDP.h"

void TCPServer::listenAndRun() {
    /* This function sets up sockets, runs into evernt loop and transits the 
     * server state from CLOSED to LISTEN.
     **/
    struct addrinfo hints, *servinfo,  *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv=getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
        logger.logging(ERROR, "getaddrinfo error!");
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            logger.logging(ERROR, "listener socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            logger.logging(ERROR, "listener bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        logger.logging(ERROR, "listener failed to bind socket");
        return;
    }

    freeaddrinfo(servinfo);

    logger.logging(DEBUG, "listener waiting to recvfrom...");
    
    // set state from CLOSED to LISTEN
    server_state = LISTEN; 
    run();
}


void TCPServer::run() {
    /* Main event loop for TCPServer.
     * This is where the server receives different incoming packets, sends 
     * packets, handles timeout/retransmission, manages states transition, 
     * and etc.
    **/
    struct timeval tv;
    fd_set read_fds;
    FD_SET(sockfd, &read_fds);
    int nReadyFds = 0;
    reader.read(filename);
    while (true) {
        logger.logging(DEBUG, "Server running in state " + stateStringify());
        // set timeout timer depends on the state server is in
        switch(server_state) {
            case LISTEN: tv.tv_sec = ECHO_SEC;
                         tv.tv_usec = 0;
                         break;
            case SYN_RCVD: tv.tv_usec = RETRANS_TIMEOUT_USEC;
                           tv.tv_sec = 0;
                           break;
            case ESTABLISHED: 
                           if (buffer.nextTimeout() == NULL) {
                               tv.tv_sec = 0;
                               tv.tv_usec = 1;
                           } else {
                               double timeCalculate = buffer.nextTimeout()->getSendTime();
                               struct timeval now;
                               gettimeofday(&now, NULL);
                               double temp = buffer.RTO + timeCalculate - now.tv_sec - 
                                   (1e-6)*now.tv_usec;
                               tv.tv_sec = 0;
                               if (floor(temp) >= 0)
                                   tv.tv_sec = floor(temp);
                               tv.tv_usec = (temp - floor(temp))*1e6;
                           }
                           break;
            case FIN_WAIT_1: tv.tv_usec = RETRANS_TIMEOUT_USEC;
                             tv.tv_sec = 0;
                             break;
            case FIN_WAIT_2: tv.tv_sec = ECHO_SEC;
                             tv.tv_usec = 0;
                             break;
            case TIME_WAIT: tv.tv_sec = FIN_TIME_WAIT;
                            tv.tv_usec = 0;
                            break;
            default: break;
        }

        // monitor sockfd file descriptor and timeout timer
        FD_SET(sockfd, &read_fds);
        if ((nReadyFds = select(sockfd+1, &read_fds, NULL, NULL, &tv)) == -1) {
            logger.logging(ERROR, "Select error.");
            continue;
        }
        
        // handle events from different state handlers
        switch(server_state) {
            case LISTEN: runningListen(nReadyFds); break;
            case SYN_RCVD: runningSynRcvd(nReadyFds); break;
            case ESTABLISHED: runningEstablished(nReadyFds); break;
            case FIN_WAIT_1: runningFinWait1(nReadyFds); break;
            case FIN_WAIT_2: runningFinWait2(nReadyFds); break;
            case TIME_WAIT: runningTimeWait(nReadyFds); break;
            case CLOSED: close(sockfd); return;
            default: break;
        }
    }
}


void *get_in_addr(struct sockaddr *sa) {
    /* get sockaddr helper function */
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int packetReceiver(int sockfd, char buf[], int max_len, 
                    struct sockaddr_in &their_addr) {
    /* packet receiving helper function */
        int nbytes = 0;
        socklen_t addr_len = sizeof(their_addr);
        
        if ((nbytes = recvfrom(sockfd, buf, max_len-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            return -1;
        }
        return nbytes;
}


void TCPServer::runningListen(int nReadyFds) {
    /* Server behavior in LISTEN state */
    if (nReadyFds == 0) {
        // timeout
        logger.logging(DEBUG, "Server waiting for SYN...");
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, their_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet");
        
        /* 1. Check if packet is a valid SYN 
         * 2. If so, send SYN/ACK and change state to SYN_RCVD. Otherwise
         * continue to stay at LISTEN. */
        std::string packet_encoded(buf, nbytes);
        packet.consume(packet_encoded);
        if (packet.getSyn() && !packet.getAck() && !packet.getFin()) {
            std::cout << "Receiving packet " << packet.getSeqNumber() << " SYN" << std::endl;
            srand (time(NULL));
            initialSeq = rand() % MAX_SEQ;
            Packet syn_ack_packet(initialSeq, 
                                  packet.getSeqNumber() + 1,
                                  0, // TODO: fix window size later
                                  1, 1, 0);
            packet = syn_ack_packet;
            std::string sendPacket = syn_ack_packet.encode();
            if (sendto(sockfd, sendPacket.c_str(), sendPacket.size(), 0, 
                       (struct sockaddr *)&their_addr, 
                       sizeof(struct sockaddr_storage)) == -1) {
                perror("sendto");
                exit(1);
            }
            std::cout<<"Sending packet "<<initialSeq<<" SYN"<<std::endl;
            server_state = SYN_RCVD;
            
        } else {
            std::cout<<"Receiving packet "<<packet.getSeqNumber()<<std::endl;
        }
    }
}


void TCPServer::runningSynRcvd(int nReadyFds) {
    /* Server behavior in SYN_RCVD state */
    if (nReadyFds == 0) {
        /*
         * retransmit that SYN/ACK
         * */
        std::string sendPacket = packet.encode();
        if (sendto(sockfd, sendPacket.c_str(), 
                   sendPacket.size(), 0, 
                   (struct sockaddr *)&their_addr, 
                   sizeof(struct sockaddr_storage)) == -1) {
            perror("sendto");
            exit(1);
        }
        std::cout << "Sending packet " << initialSeq << " SYN" << std::endl;
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        std::cout << "have a packet to receive" << std::endl;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, their_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        /* 1. Check if packet is a valid ACK 
         * 2. If so, initialize sendbuffer, change state to ESTABLISHED,
         *    sending the first few data packets.
         **/
        std::string str_buf(buf, nbytes);
        packet.consume(str_buf);
        packet.printHeader();
        if (packet.getAck() && !packet.getFin() && !packet.getSyn() 
                && packet.getAckNumber() == initialSeq + 1) {
            server_state = ESTABLISHED;
            
            initialSeq += 1;
            while (reader.hasNext() && buffer.canContain(reader.getTop().size())){
                std::string payload = reader.pop();
                Packet sendPacket;
                sendPacket.setPayLoad(payload);
                sendPacket.setSeqNumber(initialSeq);
                initialSeq += payload.size();
                sendPacket.setAckNumber(packet.getSeqNumber() + 1);
                sendPacket.setSyn(false);
                sendPacket.setFin(false);
                sendPacket.setAck(true);
                Segment sendSegment;
                sendSegment.setPacket(sendPacket);
                struct timeval current;
                gettimeofday(&current, NULL);
                double timeCalculate = current.tv_sec + current.tv_usec*1e-6;
                sendSegment.setSendTime(timeCalculate);
                buffer.push(sendSegment);
                std::string sendPacket_str = sendPacket.encode();
                if(sendto(sockfd, sendPacket_str.c_str(), sendPacket_str.size(), 0, 
                            (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1){
                    perror("sendto");
                    exit(1);
                }
                std::cout<<"Send packet "<<sendPacket.getSeqNumber()<<std::endl;
            }
            
        }else{
            //discard the packet
        }
    }
}

void TCPServer::runningEstablished(int nReadyFds) {
    /* Server behavior in ESTABLISHED state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: find all segments that require retransmission
         * */
        Segment sendSegment;
        sendSegment = *buffer.nextTimeout();
        Packet sendPacket = sendSegment.packet;
        std::string sendPacket_str = sendPacket.encode();
        if(sendto(sockfd, sendPacket_str.c_str(), sendPacket_str.size(), 0, 
                    (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1){
            perror("sendto");
            exit(1);
        }
        std::cout<<"Send packet "<<sendPacket.getSeqNumber()<<" Time out Retransmission"<<std::endl;
    } else {
        //we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, their_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        //use SendBuffer's API
        std::string str_buf(buf, nbytes);
        packet.consume(str_buf);
        
        if(packet.getAck()&&(!packet.getFin())&&(!packet.getSyn())){
            struct timeval current;
            gettimeofday(&current, NULL);
            double timeCalculate = current.tv_sec + current.tv_usec*1e-6;
            if(buffer.ack(packet.getAckNumber(), timeCalculate) == 1){
                //fast retransmit
                Segment *reSegment;
                reSegment = buffer.findSegment(packet.getAckNumber());
                if(reSegment == NULL){
                    std::cout<<"error in Acknowledge Number"<<std::endl;
                    return;
                }
                std::string sendPacket = reSegment->packet.encode();
                if (sendto(sockfd, sendPacket.c_str(), sendPacket.size(), 0, 
                           (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1){
                    perror("sendto");
                    exit(1);
                }
                std::cout<<"Send packet "<<reSegment->getSeqNum()<<" Fast Retransmission"<<std::endl;
            }
            if((!reader.hasNext())&&buffer.isEmpty()){
                packet.setFin(true);
                packet.setAck(true);
                packet.setSyn(false);
                packet.setAckNumber(packet.getSeqNumber() + 1);
                initialSeq = buffer.getEnd();
                packet.setSeqNumber(initialSeq);
                std::string sendPacket = packet.encode();
                if(sendto(sockfd, sendPacket.c_str(), sendPacket.size(), 0, 
                            (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1){
                    perror("sendto");
                    exit(1);
                }
                std::cout<<"Send packet "<<packet.getSeqNumber()<<" FIN"<<std::endl;
                server_state = FIN_WAIT_1;
            } else {
                while(reader.hasNext()&&buffer.canContain(reader.getTop().size())){
                    std::string payload = reader.pop();
                    Packet sendPacket;
                    sendPacket.setPayLoad(payload);
                    sendPacket.setSeqNumber(buffer.getEnd());

                    sendPacket.setAckNumber(packet.getSeqNumber()+1);
                    sendPacket.setSyn(false);
                    sendPacket.setFin(false);
                    sendPacket.setAck(true);
                    Segment sendSegment;
                    sendSegment.setPacket(sendPacket);
                    gettimeofday(&current, NULL);
                    timeCalculate = current.tv_sec + current.tv_usec*1e-6;
                    sendSegment.setSendTime(timeCalculate);
                    buffer.push(sendSegment);
                    std::string sendPacket_str = sendPacket.encode();
                    if (sendto(sockfd, sendPacket_str.c_str(), 
                                sendPacket_str.size(), 0, (struct sockaddr *)&their_addr, 
                                sizeof(struct sockaddr_storage)) == -1) {
                        perror("sendto");
                        exit(1);
                    }
                    std::cout << "Send packet " << sendPacket.getSeqNumber() << std::endl;
                }
            }
        }
    }
        
}


void TCPServer::runningFinWait1(int nReadyFds) {
    /* Server behavior in FIN_WAIT_1 state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: resend FIN
         * */
        std::string sendPacket = packet.encode();
        if (sendto(sockfd, sendPacket.c_str(), sendPacket.size(), 0, 
                    (struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage)) == -1) {
            perror("sendto");
            exit(1);
        }
        std::cout<<"Send packet "<<packet.getSeqNumber()<<" FIN"<<" Time out Retransmit"<<std::endl;
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, their_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from ");
        
        /* TODO: 1. see if the packet is FIN-ACK
         *       2. if so, change state to FIN_WAIT_2
         *       3. see if the packet is FIN-ACK/FIN
         *       4. if so, change state directly to TIME_WAIT
         **/
        std::string str_buf(buf, nbytes);
        packet.consume(str_buf);
        if(packet.getAck() && packet.getFin() && (!packet.getSyn())){
            //
            packet.setAck(true);
            packet.setFin(false);
            packet.setSyn(false);
            packet.setAckNumber(packet.getSeqNumber());
            initialSeq += 1;
            packet.setSeqNumber(initialSeq);
            server_state = TIME_WAIT;
        }
    }
}

void TCPServer::runningFinWait2(int nReadyFds) {
    /* Server behavior in FIN_WAIT_2 state */
    if (nReadyFds == 0) {
        // timeout
        logger.logging(DEBUG, "waiting in FIN_WAIT_2");
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, their_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet");
        
        /* TODO: 1. see if the packet is FIN
         *       2. if so, change state to TIME_WAIT
         **/
    }
}


void TCPServer::runningTimeWait(int nReadyFds) {
    /* Server behavior in FIN_WAIT_2 state */
    if (nReadyFds == 0) {
        // timeout
        server_state = CLOSED;
    } else {
        // we have a packet to receive
        // just ignore maybe
    }
}

void TCPServer::serverClose(int sockfd) {
    close(sockfd);
}

int main() {
    // TODO: use args to specify port and host
    std::string filename = "test";
    std::string host="10.0.0.1", port="9999";
    /*if (argc == 3){
        port = argv[1];
        filename = argv[2];
    }*/
    TCPServer tcp_server(host, port, filename);
    tcp_server.listenAndRun();
    return 0;
}
