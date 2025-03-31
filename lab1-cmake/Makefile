CC=gcc
CFLAGS=-Wall -std=c17 -g -O0
BUILD=build

.PHONY : all clean

all: countdown

countdown: countdown.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#Select path to locate binary file

clean:
	rm -rf $(BUILD)