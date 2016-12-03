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
#include "client.h"


void TCPClient::setupAndRun() {
    /* This function sets up sockets, runs into event loop and transits the 
     * client state from CLOSED to SYN_SENT.
     **/
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(server_host.c_str(), 
                          server_host.c_str(), 
                          &hints, &servinfo)) != 0) {
        logger.logging(ERROR, "getaddrinfo error");
        return;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            logger.logging(ERROR, "socket error");
            continue;
        }
        break;
    }

    if (p == NULL) {
        logger.logging(ERROR, "failed to create socket");
        return;
    }

    freeaddrinfo(servinfo);

    // TODO: conjure a SYN packet here and send to server
    
    client_state = SYN_SENT;
    run();
}


int packetReceiver(int sockfd, char buf[], int max_len) {
    /* packet receiving helper function */
        int nbytes = 0;
        socklen_t addr_len;
        struct sockaddr_storage their_addr;
        if ((nbytes = recvfrom(sockfd, buf, max_len-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            return -1;
        }
        return nbytes;
}


void TCPClient::run() {
    /* Main event loop for TCPClient.
     * This is where the client establishes connection with server, receives 
     * data packets, store the data, and etc.
     * */
    struct timeval tv;
    fd_set read_fds;
    FD_SET(sockfd, &read_fds);
    int nReadyFds = 0;
    int fdmax = sockfd;

    while (true) {
        logger.logging(DEBUG, "Client running in state " + stateStringify());
        // set timeout timer depending on the state client is in
        switch(client_state) {
            case SYN_SENT: tv.tv_usec = RETRANS_TIMEOUT_USEC;
                           tv.tv_sec = 0;
                           break;
            case ESTABLISHED: tv.tv_usec = 0;
                              tv.tv_sec = ECHO_SEC;
                              break;
            case LAST_ACK: tv.tv_sec = FIN_TIME_WAIT;
                           tv.tv_usec = 0;
                           break;
            default: break;
        }

        // monitor sockfd file descriptor and timeout timer
        if ((nReadyFds = select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1) {
            logger.logging(ERROR, "Select error.");
            continue;
        }
        
        // handle events from different state handlers
        switch(client_state) {
            case SYN_SENT: runningSynSent(nReadyFds); break;
            case ESTABLISHED: runningEstablished(nReadyFds); break;
            case LAST_ACK: runningLastAck(nReadyFds); break;
            case CLOSED: close(sockfd); return;
            default: break;
        }
    }
}


void TCPClient::runningSynSent(int nReadyFds) {
    /* Server behavior in ESTABLISHED state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: resend syn
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from server");
        
        /* TODO: 1. see if the packet is a SYN/ACK\
         *       if so, send back ack
         *       initialize recvbuffer
         *       change state to established
         **/
    }
}


void TCPClient::runningEstablished(int nReadyFds) {
    /* Server behavior in ESTABLISHED state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: output a nice little message
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from server");
        /* TODO: 1. see if the packet is a data packet
         *       1.1 if so, send back the right ack, and mantain recvbuffer
         *       2. if the packet is a FIN
         *       3. send ACK/FIN and change state to LAST_ACK
         **/
    }
}


void TCPClient::runningLastAck(int nReadyFds) {
    /* Server behavior in ESTABLISHED state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: go to closed 
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from server");

        /* TODO: 1. see if the packet is a Fin-ACK\
         *       if so, go to closed
         **/
    }
}


int main() {
    // TODO: args
    std::string server_host = "10.0.0.2", server_port = "9999";
    TCPClient tcp_client(server_host, server_port);
    tcp_client.setupAndRun();
}
