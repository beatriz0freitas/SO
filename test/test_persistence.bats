#!/usr/bin/env bats

load test/test_libs/bats-support/load.bash
load test/test_libs/bats-assert/load.bash

# --- Configuration ---
readonly SERVER_EXEC="./bin/dserver"
readonly CLIENT_EXEC="./bin/dclient"
readonly TEST_DOC_FOLDER="test/testdocs"
readonly SERVER_FIFO_DIR="fifos"
readonly DATA_FILE="information.bin"
readonly CACHE_SIZE="10"
readonly DOC1_ID=1
readonly DOC2_ID=2

setup_file() {
    mkdir -p "$TEST_DOC_FOLDER" "$SERVER_FIFO_DIR"
    rm -f "$DATA_FILE"

    echo "apple is here in doc 1." > "$TEST_DOC_FOLDER/doc1_persist.txt"
    echo "banana is here twice." > "$TEST_DOC_FOLDER/doc2_persist.txt"
    echo "banana again." >> "$TEST_DOC_FOLDER/doc2_persist.txt"
}

teardown_file() {
    pkill -f "$SERVER_EXEC"
    sleep 1
    rm -f "$DATA_FILE"
    rm -rf "$TEST_DOC_FOLDER" "$SERVER_FIFO_DIR"
}

start_server() {
    "$SERVER_EXEC" "$TEST_DOC_FOLDER" "$CACHE_SIZE" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
}

stop_server() {
    run "$CLIENT_EXEC" -f
    wait "$SERVER_PID"
    sleep 1
}

@test "01. Persistence: index -> shutdown -> restart -> consult + search" {
    # Start and index
    start_server

    run "$CLIENT_EXEC" -a "Persistent Doc 1" "Author P1" "2023" "doc1_persist.txt"
    assert_success
    assert_output --regexp "indexado com sucesso.*$DOC1_ID"

    run "$CLIENT_EXEC" -a "Persistent Doc 2" "Author P2" "2024" "doc2_persist.txt"
    assert_success
    assert_output --regexp "indexado com sucesso.*$DOC2_ID"

    # Verify before shutdown
    run "$CLIENT_EXEC" -c "$DOC1_ID"
    assert_success
    assert_output --partial "Persistent Doc 1"

    run "$CLIENT_EXEC" -s "banana"
    assert_success
    assert_output --partial "[$DOC2_ID]"

    # Restart
    stop_server
    start_server

    # Verify again after restart
    run "$CLIENT_EXEC" -c "$DOC1_ID"
    assert_success
    assert_output --partial "Persistent Doc 1"

    run "$CLIENT_EXEC" -s "banana"
    assert_success
    assert_output --partial "[$DOC2_ID]"
}
