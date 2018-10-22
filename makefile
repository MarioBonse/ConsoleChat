CC = gcc
CFLAGS = -Wall -std=c99

.PHONY: all clean

all: msg_client

build/cl_command.o: src/cl_command.c src/client.h
	$(CC) $(CFLAGS) -c src/cl_command.c -o build/cl_command.o

build/cl_main.o: src/cl_main.c src/client.h
	$(CC) $(CFLAGS) -c src/cl_main.c -o build/cl_main.o

build/cl_network.o: src/cl_network.c src/client.h
	$(CC) $(CFLAGS) -c src/cl_network.c -o build/cl_network.o

msg_client:	build/cl_main.o	build/cl_command.o build/cl_network.o
	$(CC) -o msg_client build/cl_main.o build/cl_command.o build/cl_network.o -lpthread



build/list.o: src/list.h src/list.c
	$(CC) $(CFLAGS) -c src/list.c -o build/list.o

build/server_command.o: src/server_command.c src/server.h
	$(CC) $(CFLAGS) -c src/server_command.c -o build/server_command.o

build/server_main.o: src/server_main.c src/server.h
	$(CC) $(CFLAGS) -c src/server_main.c -o build/server_main.o

build/server_network.o: src/server_network.c src/server.h
	$(CC) $(CFLAGS) -c src/server_network.c -o build/server_network.o

msg_server: build/server_main.o	build/server_command.o build/server_network.o	build/list.o
	$(CC) -o msg_server build/server_main.o build/server_command.o build/list.o build/server_network.o -lpthread

all: msg_server

clean:
	rm -f build/*
	rm -f msg_client
	rm -f msg_server
