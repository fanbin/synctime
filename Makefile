all: server client
CPPFLAG=-g

server: server.cpp parse.cpp
	g++ $(CPPFLAG) server.cpp parse.cpp -lpthread -o sync-server

client: client.cpp parse.cpp
	g++ $(CPPFLAG) client.cpp parse.cpp -lpthread -o sync-client

clean:
	rm -rf sync-server sync-client
