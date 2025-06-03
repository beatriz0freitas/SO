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


bin/dserver: obj/dserver.o obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o obj/cache.o
	$(CC) $^ $(LDFLAGS) -o $@

bin/dclient: obj/dclient.o obj/command.o obj/utils.o obj/message.o obj/metaInformation.o obj/metaInformationDataset.o obj/executer.o obj/cache.o
	$(CC) $^ $(LDFLAGS) -o $@


# Compile the source files into object files
obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@


test/test_libs:
	@echo "[INFO] Cloning Bats test libraries..."
	@mkdir -p test/test_libs
	@git clone https://github.com/bats-core/bats-support test/test_libs/bats-support
	@git clone https://github.com/bats-core/bats-assert test/test_libs/bats-assert

test_start_stop:
	@echo "[INFO] Running Bats tests in test/test_start_stop_server.bats..."
	bats --formatter pretty  test/test_start_stop_server.bats; \
	@echo "[INFO] Bats test run complete."

test_features: test/test_libs
	@echo "[INFO] Running Bats tests in test/test_features.bats..."
	bats --formatter pretty  test/test_features.bats; \
	@echo "[INFO] Bats test run complete."

test_persistence: test/test_libs
	@echo "[INFO] Running Bats tests in test/test_persistence.bats..."
	bats --formatter pretty  test/test_persistence.bats; \
	@echo "[INFO] Bats test run complete."

test_paralell_search: test/test_libs
	@echo "[INFO] Running Bats tests in test/test_parallel_search.bats..."
	bats   --formatter pretty test/test_parallel_search.bats; \
	@echo "[INFO] Bats performance test run complete."

test_concurrency: test/test_libs
	@echo "[INFO] Running Bats tests in test/test_concurrency.bats..."
	bats --formatter pretty test/test_concurrency.bats; \
    @echo "[INFO] Bats concurrency test run complete."

test_cache_performance:
	echo "[INFO] Running Bats tests in test/test_cache_performance.bats..."
	bats --formatter pretty test/test_cache_performance.bats;
	echo "[INFO] Bats cache performance test run complete."

reset_test_env: cleanAll
	@echo "[Info] Ambiente de teste limpo"
	@rm -f information.bin || true

clean: reset_test_env
	rm -f obj/*.o tmp/*
	rm -f bin/dserver bin/dclient

cleanAll: clean
	rm -f information.bin


.PHONY: test