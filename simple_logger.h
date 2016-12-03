#ifndef _SIMPLE_LOGGER_H_
#define _SIMPLE_LOGGER_H_

#include <iostream>
#include <string>

enum Level { DEBUG = 0, INFO, WARN, ERROR };

/* logger helper class */
class SimpleLogger {
private:
    Level level;
    std::string side;
public:
    SimpleLogger(std::string s="SERVER", Level ll=DEBUG)
        : level(ll)
        , side(s) { }
    static const std::string level_str[];
    void logging(Level logging_level, const std::string &logs) {
        if (logging_level < level)
            return;
        std::cout << "[" << level_str[logging_level] << "] " << side
                  << ": " << logs << std::endl;
    }
};

const std::string SimpleLogger::level_str[] = {"DEBUG", "INFO", 
    "WARN", "ERROR" };

#endif
