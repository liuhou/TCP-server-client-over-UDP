CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=804777210-904759994

# Add all .cpp files that need to be compiled for your server
SERVER_FILES=server.cpp TCPOverUDP.cpp

# Add all .cpp files that need to be compiled for your client
CLIENT_FILES=client.cpp TCPOverUDP.cpp

all: simple-tcp-server simple-tcp-client

*.o: *.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

simple-tcp-server: $(SERVER_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVER_FILES:.cpp=.o)

simple-tcp-client: $(CLIENT_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(CLIENT_FILES:.cpp=.o)

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM simple-tcp-server simple-tcp-client *.tar.gz

test: TCPOverUDP.o test.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

tarball: clean
	tar -czvf $(USERID).tar.gz *.cpp *.h Makefile
