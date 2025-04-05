CC = gcc
CFLAGS =$(shell pkg-config --cflags glib-2.0) -Wall -g -Iinclude 
LDFLAGS =
LIBS = $(shell pkg-config --libs glib-2.0)

all: folders dserver dclient

dserver: bin/dserver

dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp

bin/dserver: obj/dserver.o
	$(CC) $(LDFLAGS)  $(LIBS) $^ -o $@

bin/dclient: obj/dclient.o
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/*