#!/usr/bin/env bats

load test/test_libs/bats-support/load.bash
load test/test_libs/bats-assert/load.bash

# --- Parametrização Global ---
readonly CLIENT_EXEC="./bin/dclient"
readonly SERVER_EXEC="./bin/dserver"
readonly TEST_DOC_FOLDER="test/testdocs"
readonly CACHE_SIZE="10"
readonly KEYWORD="amor"
readonly NUM_READERS=10
readonly NUM_WRITERS=3
readonly CSV_FILE="test/concurrent_results.csv"

setup_file() {
    # Verifica se o servidor está a correr
    echo "# Verificando se o servidor está ativo..." >&2
    if ! pgrep -f "$SERVER_EXEC" > /dev/null; then
        echo "[ERRO] O servidor '$SERVER_EXEC' não está a correr. Inicia-o manualmente antes do teste." >&2
        exit 1
    fi

    rm -f  "$CSV_FILE"

    # Criar documentos de teste
    mkdir -p "$TEST_DOC_FOLDER"
    echo "$KEYWORD e paz em todo o lado." > "$TEST_DOC_FOLDER/doc_conc.txt"
}

teardown_file() {
    sleep 1
    # Remover documentos de teste
    rm -rf "$TEST_DOC_FOLDER"
}

run_client_consult() {
    local id=$1
    output=$("$CLIENT_EXEC" -c "$id")
    status=$?
    echo "$i,consult,$([ $status -eq 0 ] && echo success || echo failure),\"${output//$'\n'/\\n}\"" >> "$CSV_FILE"
}

run_client_search() {
    output=$("$CLIENT_EXEC" -s "$KEYWORD")
    status=$?
    echo "$i,search,$([ $status -eq 0 ] && echo success || echo failure),\"${output//$'\n'/\\n}\"" >> "$CSV_FILE"
}

run_client_write() {
    local id=$1
    local file="$TEST_DOC_FOLDER/write_doc_$id.txt"
    echo "Linha $id com $KEYWORD" > "$file"
    output=$("$CLIENT_EXEC" -a "Write $id" "Writer $id" "2024" "write_doc_$id.txt")
    status=$?
    echo "W$id,write,$([ $status -eq 0 ] && echo success || echo failure),\"${output//$'\n'/\\n}\"" >> "$CSV_FILE"
}

@test "Clientes em paralelo: ${NUM_READERS} readers, ${NUM_WRITERS} writers" {
    echo "# Indexing base document..." >&2
    run "$CLIENT_EXEC" -a "Conc Base" "Author Base" "2023" "doc_conc.txt"
    assert_success
    BASE_ID=$(echo "$output" | grep -o '[0-9]\+')

    echo "# Launching $NUM_READERS readers and $NUM_WRITERS writers..." >&2
    echo "client,type,status,logs" > "$CSV_FILE"

    for i in $(seq 1 "$NUM_READERS"); do
        (
            if (( i % 2 == 0 )); then
                echo "[CLIENT $i - CONSULT]" >&2
                run_client_consult "$BASE_ID"
            else
                echo "[CLIENT $i - SEARCH]" >&2
                run_client_search
            fi
        ) &
    done

    for i in $(seq 1 "$NUM_WRITERS"); do
        (
            echo "[CLIENT W$i - WRITE]" >&2
            run_client_write "$i"
        ) &
    done

    wait

    echo "# Final verification: keyword search..." >&2
    run "$CLIENT_EXEC" -s "$KEYWORD"
    assert_success
}
