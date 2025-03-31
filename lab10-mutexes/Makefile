CC = gcc
CFLAGS = -Wall
BUILD=build
CFLAGS=-Wall -std=c11 -g -O0

all: santa
	
santa: santa.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $< -lpthread

run: santa
	./$(BUILD)/santa

clean:
	rm -rf $(BUILD)
