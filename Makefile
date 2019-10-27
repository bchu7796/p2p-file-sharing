all:server peer

server:
	g++ -pthread -std=c++11 -o server server.cpp
peer:
	g++ -pthread -std=c++11 -o peer peers.cpp

clean:
	rm server peer