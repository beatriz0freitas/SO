CC = gcc
CFLAGS =$(shell pkg-config --cflags glib-2.0) -Wall -g -Iinclude 
LDFLAGS = 
LIBS = $(shell pkg-config --libs glib-2.0)

OBJS = $(patsubst src/%.c, obj/%.o, src/*.c)

all: folders dserver dclient

dserver: bin/dserver

dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp

#TODO: Arranjar maneira de não ter de estar a adicionar os .o necessários um a um 

bin/dserver: obj/dserver.o obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o
	$(CC) $(LDFLAGS)  $(LIBS) $^ -o $@

bin/dclient:obj/dclient.o  obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/*