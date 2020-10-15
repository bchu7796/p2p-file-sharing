CXX = g++
CXXFLAGS = -pthread -std=c++11

all:server client

server: server_main.o server.o
	$(CXX) $(CXXFLAGS) server_main.o server.o -o server

server_main.o: server_main.cpp
	$(CXX) $(CXXFLAGS) -c server_main.cpp

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp

client: client.o peers.o
	$(CXX) $(CXXFLAGS) client.o peers.o -o client

client.o: 
	$(CXX) $(CXXFLAGS) -c client.cpp

peers.o: 
	$(CXX) $(CXXFLAGS) -c peers.cpp

clean:
	rm -f core *.o
	rm server client
