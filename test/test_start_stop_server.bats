#!/usr/bin/env bats

load test/test_libs/bats-support/load.bash
load test/test_libs/bats-assert/load.bash

readonly SERVER_EXEC="./bin/dserver"
readonly CLIENT_EXEC="./bin/dclient"
readonly TEST_DOC_FOLDER="test/testdocs"
readonly CACHE_SIZE="5"

setup_file() {
    pkill -f "$SERVER_EXEC" || true
    rm -f information.bin
    mkdir -p "$TEST_DOC_FOLDER" fifos
    echo "teste de arranque" > "$TEST_DOC_FOLDER/dummy.txt"
}

teardown_file() {
    pkill -f "$SERVER_EXEC" || true
    sleep 1
    rm -rf "$TEST_DOC_FOLDER" fifos information.bin
}

@test "Start and shutdown server cleanly" {
    echo "# Starting server..."
    "$SERVER_EXEC" "$TEST_DOC_FOLDER" "$CACHE_SIZE" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 2

    run ps -p "$SERVER_PID"
    assert_success
    assert_output --partial "$SERVER_PID"

    echo "# Sending shutdown command..."
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
