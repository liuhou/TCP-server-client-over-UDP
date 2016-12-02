#ifndef _SERVER_H_
#define _SERVER_H_
#include <string>
#include <iostream>

enum ServerState {
    CLOSED = 0,
    LISTEN,
    SYN_RCVD,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    TIME_WAIT
};

enum Level { DEBUG = 0, INFO, WARN, ERROR };

/* logger helper class */
class SimpleLogger {
private:
    Level level;
public:
    SimpleLogger(Level ll=DEBUG): level(ll) { }
    static const std::string level_str[];
    void logging(Level logging_level, const std::string &logs) {
        if (logging_level < level)
            return;
        std::cout << "[" << level_str[logging_level] << "] " << "SERVER: "
                  << logs << std::endl;
    }
};


class TCPServer {
private:
    ServerState server_state;

    /* Constant def */
    static const int ECHO_SEC = 5;
    static const int FIN_TIME_WAIT = 5;
    static const int MAX_BUF_LEN = 1033;
    static const int RETRANS_TIMEOUT_USEC = 500000;
    
    /* Socket config */
    std::string host;
    std::string port;
    int sockfd;

    /* logger */
    SimpleLogger logger;


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
    TCPServer(std::string &h, std::string &p)
        : server_state(CLOSED)
        , host(h)
        , port(p) { }
    

    /* This function sets up sockets, runs into evernt loop and transits the 
     * server state from CLOSED to LISTEN.
     **/
    void listenAndRun();
};

#endif
