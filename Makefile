CC = gcc
CFLAGS = $(shell pkg-config --cflags glib-2.0) -Wall -g -Iinclude 
LDFLAGS = $(shell pkg-config --libs glib-2.0)


all: folders dserver dclient

dserver: bin/dserver

dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp

indexar: 
	./bin/addGdatasetMetadata.sh Ccatalog.tsv
	
bin/dserver: obj/dserver.o obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o
	$(CC) $^ $(LDFLAGS) -o $@

bin/dclient: obj/dclient.o obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o
	$(CC) $^ $(LDFLAGS) -o $@


# Compile the source files into object files
obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o tmp/*
	rm -f bin/dserver bin/dclient

