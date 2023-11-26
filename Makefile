CC = g++
CFLAGS = -std=c++11 -Wall

all: server client

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp -lncurses

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp -lncurses

clean:
	rm -f server client
