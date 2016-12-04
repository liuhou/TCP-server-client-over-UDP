#ifndef _SERVER_H_
#define _SERVER_H_
#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "simple_logger.h"
#include "TCPOverUDP.h"

class TCPServer {
private:

    enum ServerState {
        CLOSED = 0,
        LISTEN,
        SYN_RCVD,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        TIME_WAIT
    };
    ServerState server_state;

    /* Constant def */
    static const int ECHO_SEC = 5;
    static const int FIN_TIME_WAIT = 500000;
    static const int MAX_BUF_LEN = 1033;
    static const int RETRANS_TIMEOUT_USEC = 500000;
    
    
    /* Congestion Control */
    int Con_State = 0;
    static const int SLOW_START = 0;
    static const int CA = 1;
    static const int MSS = 1024;
    int CA_NO = 0;
    //static const int MAX_SEQ = 30720;
    /* Socket config */
    std::string host;
    std::string port;
    int sockfd;
    struct sockaddr_in their_addr;

    /* logger */
    SimpleLogger logger;
    
    /*buffer and packet*/
    std::string filename;
    SendBuffer buffer;
    FileReader reader;
    Packet packet;
    uint16_t initialSeq;

    /* Main event loop for TCPServer.
     * This is where the server receives different incoming packets, sends 
     * packets, handles timeout/retransmission, manages states transition, 
     * and etc.
     * */
    void run();


    /* Server behavior in LISTEN state */
    void runningListen(int nReadyFds);

    /* Server behavior in SYN_RCVD state */
    void runningSynRcvd(int nReadyFds);

    /* Server behavior in ESTABLISHED state */
    void runningEstablished (int nReadyFds);

    /* Server behavior in FIN_WAIT_1 state */
    void runningFinWait1 (int nReadyFds);

    /* Server behavior in FIN_WAIT_2 state */
    void runningFinWait2 (int nReadyFds);

    /* Server behavior in TIMEWAIT state */
    void runningTimeWait (int nReadyFds);
    
    /* Server behavior in CLOSED state*/
    void serverClose (int sockfd);
    
    /* Server state string */
    std::string stateStringify() {
        std::string state_strings[] = { "CLOSED"
                ,"LISTEN"
                ,"SYN_RCVD"
                ,"ESTABLISHED"
                ,"FIN_WAIT_1"
                ,"FIN_WAIT_2"
                ,"TIME_WAIT" };
        return state_strings[server_state];
    } 
public:
    /* construction */
    TCPServer(std::string &h, std::string &p, std::string &file)
        : server_state(CLOSED)
        , host(h)
        , port(p)
        , filename(file){ }

    

    /* This function sets up sockets, runs into evernt loop and transits the 
     * server state from CLOSED to LISTEN.
     **/
    void listenAndRun();
};
#endif

