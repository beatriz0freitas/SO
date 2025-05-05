CC = gcc
CFLAGS = $(shell pkg-config --cflags glib-2.0) -Wall -g -Iinclude 
LDFLAGS = $(shell pkg-config --libs glib-2.0)


all: folders dserver dclient

dserver: bin/dserver

dclient: bin/dclient

folders:
	@mkdir -p src include obj bin fifos tmp 

indexar: 
	@chmod +x bin/addGdatasetMetadata.sh
	@./bin/addGdatasetMetadata.sh Gcatalog.tsv

run: folders all
	@echo "[INFO] A iniciar servidor em background..."
	@bin/dserver data/Gdataset & echo $$! > tmp/server.pid
	@sleep 1
	@echo "[INFO] A correr script de indexação..."
	chmod +x bin/addGdatasetMetadata.sh
	@bin/addGdatasetMetadata.sh Gcatalog.tsv


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

