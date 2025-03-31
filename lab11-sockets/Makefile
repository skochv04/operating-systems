CC = gcc
CFLAGS = -Wall
BUILD=build
CFLAGS=-Wall -std=c11 -g -O0

all: client server
	
client: client.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<

server: server.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<

run_client: client
	./$(BUILD)/client $(IDENTIFIER) 127.0.0.1 5000

run_server: server
	./$(BUILD)/server 127.0.0.1 5000

clean:
	rm -rf $(BUILD)
