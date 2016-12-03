#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string>
#include <iostream>
#include "simple_logger.h"


class TCPClient {
private:
    enum ClientState {
        CLOSED = 0,
        SYN_SENT,
        ESTABLISHED,
        LAST_ACK,
    };
    ClientState client_state;

    /* Constant def */
    static const int ECHO_SEC = 5;
    static const int FIN_TIME_WAIT = 5; // What is segemnt life time? 
    static const int MAX_BUF_LEN = 1033;
    static const int RETRANS_TIMEOUT_USEC = 500000;
    static const int RCVD_WINDOW_SIZE = 15360;
    
    /* Socket config */
    std::string server_host;
    int server_port;
    int sockfd;
    struct sockaddr_in remaddr;
    int slen;


    /* logger */
    SimpleLogger logger;

    /* Main event loop for TCPClient.
     * This is where the client establishes connection with server, receives 
     * data packets, store the data, and etc.
     * */
    void run();

    /* Client behavior in SYN_SENT state */
    void runningSynSent(int nReadyFds);

    /* Client behavior in ESTABLISHED state */
    void runningEstablished(int nReadyFds);

    /* Client behavior in LAST_ACK state */
    void runningLastAck(int nReadyFds);

    /* Server state string */
    std::string stateStringify() {
        std::string state_strings[] = { "CLOSED"
                ,"SYN_SENT"
                ,"ESTABLISHED"
                ,"LAST_ACK" };
        return state_strings[client_state];
    } 
public:
    /* construction */
    TCPClient(std::string &h, int p)
        : client_state(CLOSED)
        , server_host(h)
        , server_port(p)
        , logger("CLIENT") { }
    
    /* This function sets up sockets, runs into event loop and transits the 
     * client state from CLOSED to SYN_SENT.
     **/
    void setupAndRun();
};

#endif
