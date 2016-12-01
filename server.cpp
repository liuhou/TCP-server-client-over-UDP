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

const std::string SimpleLogger::level_str[] = {"DEBUG", "INFO", 
    "WARN", "ERROR" };

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
    logger.logging(DEBUG, "running");

    while (true) {
        /*  */    
    
    
    
    }
}

int main() {
    // TODO: specifies port and host
    
    std::string host="10.0.0.1", port="9999";
    TCPServer tcp_server(host, port);
    tcp_server.listenAndRun();
}
