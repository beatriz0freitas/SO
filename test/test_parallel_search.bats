#!/usr/bin/env bats

load test/test_libs/bats-support/load.bash
load test/test_libs/bats-assert/load.bash

# --- Config ---
readonly SERVER_EXEC="./bin/dserver"
readonly CLIENT_EXEC="./bin/dclient"
readonly TEST_DOC_FOLDER="data/Gdataset"
readonly GCATALOG="Gcatalog.tsv"
readonly CACHE_SIZE="10"
readonly KEYWORD="amor"
readonly MAX_PARALLEL_PROCS=50

setup_file() {
    pkill -f "$SERVER_EXEC" || true
    rm -f information.bin
    mkdir -p fifos
}

teardown_file() {
    pkill -f "$SERVER_EXEC" || true
    sleep 1
}

start_server() {
    "$SERVER_EXEC" "$TEST_DOC_FOLDER" "$CACHE_SIZE" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 2
}

stop_server() {
    run "$CLIENT_EXEC" -f
    wait "$SERVER_PID"
}

@test "99. Performance: sequential vs parallel search (1 to $MAX_PARALLEL_PROCS)" {
    CSV_FILE="test/parallel_search_results.csv"

    echo "# Starting performance test..."
    start_server

    echo "# Indexing documents..."
    ./bin/addGdatasetMetadata.sh "$GCATALOG"

    echo "# Measuring performance..."
    mkdir -p "$(dirname "$CSV_FILE")"
    echo "mode,processes,time_ms" > "$CSV_FILE"

    # Sequential baseline
    start_time_seq=$(date +%s%N)
    run "$CLIENT_EXEC" -s "$KEYWORD"
    end_time_seq=$(date +%s%N)
    time_seq=$(( (end_time_seq - start_time_seq) / 1000000 ))
    assert_success
    echo "sequential,1,$time_seq" >> "$CSV_FILE"
    echo "# Sequential: $time_seq ms"

    # Parallel from 1 to MAX_PARALLEL_PROCS
    for ((n=1; n<=MAX_PARALLEL_PROCS; n++)); do
        start_time=$(date +%s%N)
        run "$CLIENT_EXEC" -s "$KEYWORD" "$n"
        end_time=$(date +%s%N)
        time_par=$(( (end_time - start_time) / 1000000 ))
        assert_success
        echo "parallel,$n,$time_par" >> "$CSV_FILE"
        echo "# Parallel ($n proc): $time_par ms"
    done

    echo "# Writing results to CSV: $CSV_FILE"
    run cat "$CSV_FILE"

    stop_server
}
