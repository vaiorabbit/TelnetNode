all: server client
clean:
	rm server.o client.o

server: server.o
	g++ server.o -O0 -o server
server.o: TelnetNode.h

client: client.o
	g++ client.o -O0 -o client
client.o: TelnetNode.h

.o.cpp: server.cpp client.cpp
