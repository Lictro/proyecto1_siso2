
OSXFUSE_ROOT = /usr/local
INCLUDE_DIR = $(OSXFUSE_ROOT)/include/osxfuse/fuse
LIBRARY_DIR = $(OSXFUSE_ROOT)/lib

CFLAGS = -g -Wall -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -D_DARWIN_USE_64_BIT_INODE `pkg-config fuse --cflags` -I$(INCLUDE_DIR) -L$(LIBRARY_DIR)
LINKFLAGS = -Wall -losxfuse `pkg-config fuse --libs`

all: bin/filesystem

clean:
	rm -rf bin obj

bin:
	mkdir -p bin

bin/filesystem: bin obj/utils.o obj/filesystem.o obj/device.o obj/main.o
	g++ -g -o bin/filesystem obj/* $(LINKFLAGS)

obj:
	mkdir -p obj

obj/main.o: obj main.c filesystem.h
	gcc -g $(CFLAGS) -c main.c -o $@

obj/filesystem.o: obj filesystem.c filesystem.h 
	g++ -g $(CFLAGS) -c filesystem.c -o $@

obj/device.o: obj device.c device.h
	g++ -g $(CFLAGS) -c device.c -o $@

obj/utils.o: obj utils.c utils.h
	g++ -g $(CFLAGS) -c utils.c -o $@