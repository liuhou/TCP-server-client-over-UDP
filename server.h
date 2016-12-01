#ifndef _SERVER_H_
#define _SERVER_H_
#include <string>
#include <iostream>

enum ServerState {
    CLOSED,
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
    
    /* Socket config */
    std::string host;
    std::string port;
    int sockfd;

    /* logger */
    SimpleLogger logger;
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

    /* Main event loop for TCPServer.
     * This is where the server receives different incoming packets, sends 
     * packets, handles timeout/retransmission, manages states transition, 
     * and etc.
     * */
    void run();

};

#endif
