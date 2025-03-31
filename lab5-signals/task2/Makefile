CC=gcc
CFLAGS=-Wall -std=c99 -g -O0
BUILD=build
DIR1 = art
DIR2 = art_out
VAL = 2
DIR3 = test

.PHONY : all countdown client_static client_shared client_dynamic test_static test_shared test_dynamic clean

all: countdown client_static client_shared client_dynamic flipper zad04-1 zad04-2 zad05-1

countdown: countdown.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

client_static: client.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	make -C collatz_lib collatz_static
	@#@Compile static library if it doesn't exist
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $< collatz_lib/build/libcollatz_static.a
	@#@Find path to static library file and link it with object file

client_shared: client.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	make -C collatz_lib collatz_shared
	@#@Compile shared library if it doesn't exist
	$(CC) $(CFLAGS) -L ./collatz_lib/build $< -lcollatz_shared -Wl,-rpath=./collatzlib/build -o $(BUILD)/$@
	@#@-L shows where linked library can be found
	@#@-W1,-rpath tells the linker to include the current directory in the search path for .so file at runtime

client_dynamic: client.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	make -C collatz_lib collatz_shared
	@#@Compile shared library if it doesn't exist
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $< -ldl -D DYNAMIC_DLOPEN
	@#@-ldl links libdl
	@#@-D defines macro for the preprocessor

flipper: flipper.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file
	./$(BUILD)/flipper $(DIR1) $(DIR2)
	@#@Build flipper with arguments of input directory and output directory

zad04-1: zad04-1.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

zad04-2: zad04-2.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

zad05-1: zad05-1.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

catcher: catcher.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

sender: sender.c
	mkdir -p $(BUILD)
	@#@Build files will be in the new directory
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $<
	@#@Select path to locate binary file

#tests
test_static:
	./$(BUILD)/client_static

test_shared:
	./$(BUILD)/client_shared

test_dynamic:
	./$(BUILD)/client_dynamic

test_zad04-1: zad04-1
	./$(BUILD)/zad04-1 $(VAL)

test_zad04-2: zad04-2
	mkdir -p test
	touch test/file1
	touch test/file2

	./$(BUILD)/zad04-2 $(DIR3)
	@echo "Return code: $$?"
	./$(BUILD)/zad04-2 mistake_dir
	@echo "Return code: $$?"

test_zad05-1: zad05-1
	./$(BUILD)/zad05-1 ignore
	./$(BUILD)/zad05-1 handler
	./$(BUILD)/zad05-1 mask
	./$(BUILD)/zad05-1 none

test_sender: sender
	./$(BUILD)/sender 0 1

test_catcher: catcher
	./$(BUILD)/catcher

clean:
	rm -rf $(BUILD)
	make -C collatz_lib clean
	find $(DIR2) -type f -delete
	find $(DIR3) -type f -delete
	@#@Delete all files that appeared after flipper running