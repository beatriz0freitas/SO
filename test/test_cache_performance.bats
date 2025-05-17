#!/usr/bin/env bats

load test/test_libs/bats-support/load.bash
load test/test_libs/bats-assert/load.bash

# --- Configuration ---
readonly SERVER_EXEC="./bin/dserver"
readonly CLIENT_EXEC="./bin/dclient"
readonly TEST_DOC_FOLDER="data/Gdataset"
readonly GCATALOG="Gcatalog.tsv"
readonly SERVER_FIFO_DIR="fifos"
readonly DATA_FILE="information.bin"
readonly CACHE_SIZES=(10 50 100 200 500 1000 2000)
readonly RESULTS_FILE="test/cache_performance_results.csv"

setup_file() {
    pkill -f "$SERVER_EXEC" || true
    rm -f "$DATA_FILE"
    mkdir -p "$SERVER_FIFO_DIR"
}

teardown_file() {
    pkill -f "$SERVER_EXEC" || true
    sleep 1
    rm -rf "$SERVER_FIFO_DIR" "$DATA_FILE"
}

start_server() {
    local cache_size=$1
    "$SERVER_EXEC" "$TEST_DOC_FOLDER" "$cache_size" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 2
}

stop_server() {
    run "$CLIENT_EXEC" -f
    assert_success
    assert_output --partial "Servidor encerrado com sucesso."

    sleep 1

    if ps -p "$SERVER_PID" > /dev/null; then
        echo "[ERRO] Servidor ainda está ativo após shutdown" >&2
        kill -9 "$SERVER_PID"
        exit 1
    fi
}

@test "Cache Performance Test" {
    echo "cache_size,index_time,consult_time,search_time,remove_time" > "$RESULTS_FILE"

    for cache_size in "${CACHE_SIZES[@]}"; do
        setup_file
        start_server "$cache_size"

        echo "# Indexing documents..."
        ./bin/addGdatasetMetadata.sh "$GCATALOG"

        # Measure indexing time
        local start_time=$(date +%s%3N)
        for file in "$TEST_DOC_FOLDER"/*; do
            run "$CLIENT_EXEC" -a "Test Doc" "Author" "2025" "$file"
        done
        local end_time=$(date +%s%3N)
        local index_time=$((end_time - start_time))

        # Measure consult time
        start_time=$(date +%s%3N)
        for i in {1..50}; do
            run "$CLIENT_EXEC" -c "$i"
        done
        end_time=$(date +%s%3N)
        local consult_time=$((end_time - start_time))

        # Measure search time
        start_time=$(date +%s%3N)
        run "$CLIENT_EXEC" -s "amor"
        end_time=$(date +%s%3N)
        local search_time=$((end_time - start_time))

        # Measure remove time
        start_time=$(date +%s%3N)
        for i in {1..50}; do
            run "$CLIENT_EXEC" -r "$i"
        done
        end_time=$(date +%s%3N)
        local remove_time=$((end_time - start_time))

        echo "$cache_size,$index_time,$consult_time,$search_time,$remove_time" >> "$RESULTS_FILE"

        stop_server
        teardown_file
    done
}
