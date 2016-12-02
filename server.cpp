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

    if ((rv=getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
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
    int fdmax = sockfd;

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
            case ESTABLISHED: // TODO: set timer to next timeout point
                           tv.tv_sec = 0;
                           tv.tv_usec = 0;
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
        if ((nReadyFds = select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1) {
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
                    std::string &client_addr) {
    /* packet receiving helper function */
        int nbytes = 0;
        socklen_t addr_len;
        struct sockaddr_storage their_addr;
        char s[INET6_ADDRSTRLEN];
        if ((nbytes = recvfrom(sockfd, buf, max_len-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            return -1;
        }
        client_addr = inet_ntop(their_addr.ss_family, 
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);
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
        std::string client_addr;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, client_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from " + client_addr);
        
        /* TODO: 1. Check if packet is a valid SYN 
         *       2. If so, send SYN/ACK and change state to SYN_RCVD. Otherwise
         *          continue to stay at LISTEN. */
    }
}


void TCPServer::runningSynRcvd(int nReadyFds) {
    /* Server behavior in SYN_RCVD state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: retransmit that goddamm SYN/ACK
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        std::string client_addr;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, client_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from " + client_addr);
        
        /* TODO: 1. Check if packet is a valid ACK 
         *       2. If so, initialize sendbuffer, change state to ESTABLISHED,
         *          sending the first few data packets.
         **/
    }
}


void TCPServer::runningEstablished(int nReadyFds) {
    /* Server behavior in ESTABLISHED state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: find all segments that require retransmission
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        std::string client_addr;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, client_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from " + client_addr);
        
        /* TODO: 1. Use sendBuffer's API to determine if we need fast
         *          retransmission or not.
         *       2. See if sendBuffer can fit new segments. If so, send new 
         *          segments.
         *       3. When finish sending the entire file (all segements have been
         *          acked), send Fin to client and change state to FIN_WAIT_1
         **/
    }
}


void TCPServer::runningFinWait1(int nReadyFds) {
    /* Server behavior in FIN_WAIT_1 state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: resend FIN
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        std::string client_addr;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, client_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from " + client_addr);
        
        /* TODO: 1. see if the packet is FIN-ACK
         *       2. if so, change state to FIN_WAIT_2
         *       3. see if the packet is FIN-ACK/FIN
         *       4. if so, change state directly to TIME_WAIT
         **/
    }
}


void TCPServer::runningFinWait2(int nReadyFds) {
    /* Server behavior in FIN_WAIT_2 state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: just display a message
         * */
    } else {
        // we have a packet to receive
        char buf[MAX_BUF_LEN];
        std::string client_addr;
        int nbytes = packetReceiver(sockfd, buf, MAX_BUF_LEN, client_addr);
        if (nbytes == -1) {
            logger.logging(ERROR, "recvfrom error.");
            return;
        }
        logger.logging(DEBUG, "got packet from " + client_addr);
        
        /* TODO: 1. see if the packet is FIN
         *       2. if so, change state to TIME_WAIT
         **/
    }
}


void TCPServer::runningTimeWait(int nReadyFds) {
    /* Server behavior in FIN_WAIT_2 state */
    if (nReadyFds == 0) {
        // timeout
        /*
         * TODO: time to switch over state to CLOSED
         * */
    } else {
        // we have a packet to receive
        // just ignore maybe
    }
}


int main() {
    // TODO: use args to specify port and host
    
    std::string host="10.0.0.1", port="9999";
    TCPServer tcp_server(host, port);
    tcp_server.listenAndRun();
}
