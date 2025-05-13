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

test/test_libs:
	@echo "[INFO] Cloning Bats test libraries..."
	@mkdir -p test/test_libs
	@git clone https://github.com/bats-core/bats-support test/test_libs/bats-support
	@git clone https://github.com/bats-core/bats-assert test/test_libs/bats-assert

test: test/test_libs
	@echo "[INFO] Running Bats tests in test/tests.bats..."
	@if command -v bats >/dev/null 2>&1; then \
		if [ -f test/tests.bats ]; then \
			bats --formatter pretty  test/tests.bats; \
		else \
			echo "[ERROR] test/tests.bats not found."; \
			exit 1; \
		fi; \
	else \
		echo "[ERROR] 'bats' command not found. Please install Bats."; \
		exit 1; \
	fi
	@echo "[INFO] Bats test run complete."


clean:
	rm -f obj/*.o tmp/*
	rm -f bin/dserver bin/dclient

.PHONY: test