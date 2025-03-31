CC=gcc
CFLAGS=-Wall -std=c11 -g -O0
BUILD=build

.PHONY : all collatz_static collatz_shared clean

all: collatz_static collatz_shared

#Build only static Collatz library
collatz_static: collatz.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@.o -c $<
	@#@Select path to locate binary file
	@#@By using -c option files won`t be linked, but only compiled
	ar rcs $(BUILD)/lib$@.a $(BUILD)/$@.o
	@#@Create archive of library

#Build only shared Collatz library
collatz_shared: collatz.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -fPIC -o $(BUILD)/$@.o -c $<
	@#@Select path to locate binary file
	@#@-fPIC option means position independent code, which is necessary for creating shared library
	@#@By using -c option files won`t be linked, but only compiled
	$(CC) -shared -o $(BUILD)/lib$@.so $(BUILD)/$@.o

clean:
	rm -rf $(BUILD)